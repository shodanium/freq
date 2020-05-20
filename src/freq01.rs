use std::{
    cmp::Ordering,
    fs::File,
    io::{BufWriter, Read, Write},
};

use clap::Clap;
use memmap::*;

/// FNV1 hash basis
const H: usize = 0x811c_9dc5;
/// FNV1 hash prime
const P: usize = 0x0100_0193;

/// Counts number of unique `[a-zA-Z]+` words in input.
#[derive(Clap, Debug)]
#[clap(version = "0.1.0")]
struct Opts {
    /// Name of input file, or `-` if STDIN should be used.
    input: String,
    /// Name of output file, or `-` if STDOUT should be used.
    output: Option<String>,
}

fn main() {
    let opts: Opts = Opts::parse();
    let input = open_mmap(&opts);

    let mut hash = H;
    let mut dict = FrequencyHashMap::new();

    let mut word_start = 0;
    let mut word_end = 0;
    for (idx, &byte) in input.iter().enumerate() {
        if (b'a' <= byte && byte <= b'z') || (b'A' <= byte && byte <= b'Z') {
            hash ^= (byte & 0x1F) as usize;
            hash *= P;
            word_end = idx + 1;
        } else {
            if word_start < word_end {
                dict.register(hash, &input[word_start..word_end]);
                hash = H;
            }
            word_start = idx + 1;
        }
    }
    if word_start < word_end {
        dict.register(hash, &input[word_start..word_end]);
    }

    let mut output = create_output(&opts);
    for (count, word) in dict.into_iter() {
        writeln!(&mut output, "{} {}", count, word).unwrap_or_else(|e| {
            let output = opts.output.as_ref().map_or("-", |s| s.as_str());
            panic!("Unable to write results in '{}': {}", output, e)
        })
    }
}

struct FrequencyHashMap {
    buckets: Vec<Option<FrequencyHashEntry>>,
    capacity: usize,
    length: usize,
    mask: usize,
    max: usize,
}

#[derive(Clone, Debug)]
struct FrequencyHashEntry {
    key: Box<[u8]>,
    value: usize,
    hash: usize,
}

struct FrequencyHashIntoIter {
    iter: std::vec::IntoIter<Option<FrequencyHashEntry>>,
}

impl FrequencyHashMap {
    const INITIAL: usize = 128;
    const LOAD_FACTOR: f32 = 0.9;

    fn new() -> FrequencyHashMap {
        FrequencyHashMap {
            buckets: vec![None; Self::INITIAL],
            capacity: Self::INITIAL,
            length: 0,
            mask: Self::INITIAL - 1,
            max: (Self::LOAD_FACTOR * Self::INITIAL as f32) as usize,
        }
    }

    fn register(&mut self, hash: usize, word: &[u8]) {
        let mut index = hash & self.mask;
        loop {
            match unsafe { self.buckets.get_unchecked_mut(index) } {
                Some(entry) => {
                    if entry.same_as(hash, word) {
                        entry.value += 1;
                        return;
                    } else {
                        index = (index + 1) & self.mask
                    }
                }
                none => {
                    none.replace(FrequencyHashEntry::new(hash, word));

                    self.length += 1;
                    if self.length > self.max {
                        self.ensure_capacity();
                    }
                    break;
                }
            }
        }
    }

    fn ensure_capacity(&mut self) {
        while self.length > self.max {
            self.capacity *= 2;
            self.mask = self.capacity - 1;
            self.max = (Self::LOAD_FACTOR * self.capacity as f32) as usize;
        }

        let new_buckets = vec![None; self.capacity];
        for bucket in std::mem::replace(&mut self.buckets, new_buckets) {
            if let Some(entry) = bucket {
                let mut index = entry.hash & self.mask;
                loop {
                    match unsafe { self.buckets.get_unchecked_mut(index) } {
                        Some(_) => index = (index + 1) & self.mask,
                        none => {
                            none.replace(entry);
                            break;
                        }
                    }
                }
            }
        }
    }
}

impl IntoIterator for FrequencyHashMap {
    type Item = (usize, String);
    type IntoIter = FrequencyHashIntoIter;

    fn into_iter(self) -> Self::IntoIter {
        let mut buckets = self.buckets;
        buckets.sort_unstable();

        FrequencyHashIntoIter {
            iter: buckets.into_iter(),
        }
    }
}

impl FrequencyHashEntry {
    #[inline]
    fn new(hash: usize, word: &[u8]) -> FrequencyHashEntry {
        FrequencyHashEntry {
            key: word.iter().map(|b| b | 0x20).collect(),
            hash,
            value: 1,
        }
    }

    #[inline]
    fn same_as(&self, hash: usize, word: &[u8]) -> bool {
        hash == self.hash
            && self.key.len() == word.len()
            && Iterator::zip(self.key.iter(), word.iter()).all(|(&l, &r)| l == (r | 0x20))
    }
}

impl Ord for FrequencyHashEntry {
    fn cmp(&self, other: &Self) -> Ordering {
        Ord::cmp(&other.value, &self.value)
            .then_with(|| Ord::cmp(self.key.as_ref(), other.key.as_ref()))
    }
}

impl PartialOrd for FrequencyHashEntry {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(Ord::cmp(&self, &other))
    }
}

impl Eq for FrequencyHashEntry {}

impl PartialEq for FrequencyHashEntry {
    fn eq(&self, other: &Self) -> bool {
        Ord::cmp(&self, &other) == Ordering::Equal
    }
}

impl Iterator for FrequencyHashIntoIter {
    type Item = (usize, String);

    fn next(&mut self) -> Option<Self::Item> {
        while let Some(opt) = self.iter.next() {
            if let Some(entry) = opt {
                let key = std::str::from_utf8(&entry.key).unwrap().to_owned();
                return Some((entry.value, key));
            }
        }
        None
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let (_, upper) = self.iter.size_hint();
        (0, upper)
    }
}

fn open_mmap(opts: &Opts) -> Mmap {
    match opts.input.as_str() {
        "-" => {
            let mut buffer = vec![];
            std::io::stdin()
                .read_to_end(&mut buffer)
                .unwrap_or_else(|e| panic!("Unable to read STDIN: {}", e));
            let mut mmap = MmapOptions::new()
                .len(buffer.len())
                .map_anon()
                .unwrap_or_else(|e| panic!("Unable to read STDIN: {}", e));
            mmap.copy_from_slice(&buffer);
            mmap.make_read_only()
                .unwrap_or_else(|e| panic!("Unable to read STDIN: {}", e))
        }
        fnm => {
            let file = File::open(fnm)
                .unwrap_or_else(|e| panic!("Unable to open '{}' for reading: {}", fnm, e));
            unsafe {
                MmapOptions::new()
                    .map(&file)
                    .unwrap_or_else(|e| panic!("Unable to read '{}' in memory: {}", fnm, e))
            }
        }
    }
}

fn create_output(opts: &Opts) -> Box<dyn Write> {
    match opts.output.as_deref() {
        Some("-") | None => Box::new(BufWriter::new(std::io::stdout())),
        Some(fnm) => {
            let file = File::create(fnm)
                .unwrap_or_else(|e| panic!("Unable to open '{}' for writing: {}", fnm, e));
            Box::new(BufWriter::new(file))
        }
    }
}
