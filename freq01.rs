use clap::Clap;
use fnv::FnvHashMap;
use std::{
    fs::File,
    io::{BufRead, BufReader, BufWriter, Write},
};

/// Counts number of unique `[a-zA-Z]+` words in input.
#[derive(Clap, Debug)]
#[clap(version = "0.1.0")]
struct Opts {
    /// Name of input file, or `-` if STDIN should be used.
    input: String,
    /// Name of output file, or `-` if STDOUT should be used.
    output: Option<String>,
}

struct FreqDict {
    dict: FnvHashMap<String, u32>,
}

fn main() {
    let opts: Opts = Opts::parse();
    let mut input = open_input(&opts);
    let mut output = create_output(&opts);

    let mut word = Vec::with_capacity(16);
    let mut dict = FreqDict::new();

    let mut buffer = [0u8; 16 * 1024];
    loop {
        let read_count = input
            .read(&mut buffer)
            .unwrap_or_else(|e| panic!("Unable to read bytes from '{}': {}", opts.input, e));

        if read_count == 0 {
            break;
        }

        for &byte in buffer.iter().take(read_count) {
            if b'a' <= byte && byte <= b'z' {
                word.push(byte);
                continue;
            } else if b'A' <= byte && byte <= b'Z' {
                word.push(byte ^ 0x20);
                continue;
            }

            dict.add_word(&word);
            word.clear();
        }
    }
    dict.add_word(&word);

    for (count, word) in dict.get_freq() {
        writeln!(&mut output, "{} {}", count, word).unwrap_or_else(|e| {
            let output = opts.output.as_ref().map_or("-", |s| s.as_str());
            panic!("Unable to write results in '{}': {}", output, e)
        })
    }
}

impl FreqDict {
    fn new() -> Self {
        FreqDict {
            dict: FnvHashMap::default(),
        }
    }

    fn add_word(&mut self, word: &[u8]) {
        if !word.is_empty() {
            let key = std::str::from_utf8(&word).unwrap();
            if let Some(counter) = self.dict.get_mut(key) {
                *counter += 1;
            } else {
                self.dict.insert(key.to_owned(), 1);
            }
        }
    }

    fn get_freq(&self) -> Vec<(u32, &str)> {
        let mut freq = self.dict.iter()
            .map(|(w, c)| (*c, w.as_str()))
            .collect::<Vec<_>>();
        freq.sort_unstable_by(|(c1, w1), (c2, w2)| {
            Ord::cmp(c1, c2).reverse().then_with(|| Ord::cmp(w1, w2))
        });
        freq
    }
}

fn open_input(opts: &Opts) -> Box<dyn BufRead> {
    match opts.input.as_str() {
        "-" => Box::new(BufReader::new(std::io::stdin())),
        fnm => {
            let file = File::open(fnm)
                .unwrap_or_else(|e| panic!("Unable to open '{}' for reading: {}", fnm, e));
            Box::new(BufReader::new(file))
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
