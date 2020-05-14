package main

import (
	"fmt"
	"os"
	"log"
	"bufio"
	"sort"
	"runtime/pprof"
)

const BUF = 262144
const MAXWORD = 256

func Check(err error) {
	if err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
}

type Entry struct {
	Freq int
	Word string
}

func main() {
	if len(os.Args) >= 4 && os.Args[3] == "--prof" {
		xf, err := os.Create("cpu.prof")
		Check(err)
		defer xf.Close()

		err = pprof.StartCPUProfile(xf)
		Check(err)
		defer pprof.StopCPUProfile()
	}

	if len(os.Args) < 3 {
		log.Fatal("usage: freq <in.txt> <out.txt> [--prof]")
		os.Exit(1)
	}

	fp, err := os.Open(os.Args[1])
	Check(err)

	data := make([]byte, BUF)
	var word [256]byte
	wordlen := 0
	total := int64(0)
	freqs := make(map[string]int)

	var lctab [256]byte
	for i := 0; i < 256; i++ {
		c := rune(i)

		if (c >= 'a' && c <= 'z') {
			lctab[i] = byte(i)
		} else if (c >= 'A' && c <= 'Z') {
			lctab[i] = byte(i + 'a' - 'A')
		} else {
			lctab[i] = 0
		}
	}

	for {
		count, err := fp.Read(data)
		if count == 0 {
			if wordlen > 0 {
				freqs[string(word[0:wordlen])]++
				wordlen = 0
			}
			break
		}

		for i := 0; i < count; i++ {
			ch := lctab[data[i]]
			if wordlen < MAXWORD {
				word[wordlen] = ch
			}

			if ch == 0 {
				if wordlen > 0 {
					freqs[string(word[0:wordlen])]++
				}
				wordlen = 0
			} else {
				wordlen += 1
			}
		}

		Check(err)
		total += int64(count)
	}

	fp.Close()

	fp, err = os.OpenFile(os.Args[2], int(os.O_TRUNC|os.O_CREATE|os.O_WRONLY), 0644)
	Check(err)

	wr := bufio.NewWriter(fp)
	arr := make([]Entry, 0, len(freqs))
	for word, freq := range freqs {
		arr = append(arr, Entry{freq, word})
	}

	sort.Slice(arr[:], func(i, j int) bool {
		if arr[i].Freq != arr[j].Freq {
			return arr[i].Freq > arr[j].Freq
		}
		return arr[i].Word < arr[j].Word
	})

	for _, e := range arr {
		fmt.Fprintf(wr, "%d %s\n", e.Freq, e.Word)
	}

	wr.Flush()
	fp.Close()

//	fmt.Printf("total %d bytes\n", total)
//	fmt.Printf("total %d words\n", len(arr))
}
