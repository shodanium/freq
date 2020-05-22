Simple frequency dict benchmarks
=================================

Or, "a million ways to skin a corpus".

Here there be descriptions and test results. Eventually.

In the meantime, here's a cookie, and a test file: https://yadi.sk/d/xTFWy38y3GBAgS

pg.txt, 336 183 276 bytes, MD5 9ca4282866bb919e03cca2ed2b6ce429

What
-----

Briefly, the task is as follows. Write a program that

 * takes exactly 2 command line arguments, `myprog in.txt out.txt`
 * counts all unique input words, and print them in `freq desc, word asc` order
 * words are `[a-zA-Z]{1,256}`, ie. latin letters only, and should be lowercased
 * input can contain arbitrary bytes, assume **nothing** about encoding, zeroes, etc
 * input might not fit in RAM, output however must fit in RAM
 * single-threaded only, for simplicity etc

And here's more crazy.

 * assume (for now) that 32 bits is enough for frequencies, ie. that all freqs
   are under 4e9
 * assume that any 64 bit hash function will **easily** collide, and that any
   128 bit hash function **might** actually collide, either "by random luck"
   or by malicious attack

Last but not least, have a tiny example.

`in.txt`:
```
The time has come, the Walrus said,
to talk of many things
```

`out.txt`:
```
2 the
1 come
1 has
1 many
1 of
1 said
1 talk
1 things
1 time
1 to
1 walrus
```

Why
----

This is a community project with the only ultimate goal: **to explore**.

 * to explore different programming languages and syntaxes
 * to explore "typical" and idiomatic solutions
 * to explore elegant and concise solutions
 * to explore algorithms and programming techniques
 * to explore performance and optimization tricks

That, and to have some fun fiddling with a simple quick task in a myriad
different ways.

Benchmark results
------------------

**i5-4690 @ 3.7 GHz, Windows 7 x64**

| min..max         | MB/s  | program      | compiler or runtime             |
|------------------|-------|--------------|---------------------------------|
| 1.310..1.325s    | 256.6 | freq03.cpp   | vs2019 16.1.0                   |
| 1.747..1.778s    | 192.5 | freq02.cpp   | vs2019 16.1.0                   |
| 2.462..2.491s    | 136.6 | freq01.cpp   | vs2019 16.1.0                   |
| 2.655..2.675s    | 126.6 | freq01.rs    | rustc 1.43.1                    |
| 4.397..4.512s    | 76.5  | freq01.go    | go 1.14.1                       |
| 14.089..14.901s  | 23.9  | freq01.scala | scala 2.13.1, jdk 1.8.0_144     |
| 30.973..31.318s  | 10.9  | freq01.php   | php 7.0.20-zts                  |
| 99.880..102.353s | 3.4   | freq01.py    | python 3.7.0                    |
|                  |       |              |                                 |
| 1.407..1.416s    | 239.0 | hack01.cpp   | vs2019 16.1.0                   |

**Xeon Gold 6240 @ 2.6 GHz, Debian 9 x64**

| min..max         | MB/s  | program      | compiler or runtime             |
|------------------|-------|--------------|---------------------------------|
| 1.260..1.278s    | 266.8 | freq03.cpp   | gcc 6.3.0                       |
| 1.492..1.521s    | 225.4 | freq02.cpp   | gcc 6.3.0                       |
| 2.197..2.245s    | 153.0 | freq01.rs    | rustc 1.43.1                    |
| 2.480..2.737s    | 135.5 | freq01.cpp   | gcc 6.3.0                       |
| 3.880..4.187s    | 86.6  | freq01.go    | go 1.14.3                       |
| 9.037..9.173s    | 37.2  | freq01.php   | php 7.4.6-nts                   |
| 20.152..22.151s  | 16.7  | freq01.scala | scala 2.13.1, openjdk 1.8.0_242 |
| 83.251..85.987s  | 4.0   | freq01.py    | python 3.5.3                    |
|                  |       |              |                                 |
| 1.349..1.442s    | 249.2 | hack01.cpp   | gcc 6.3.0                       |

C++
----

```bash
g++ -O3 -o freq01 freq01.cpp
./freq01 pg.txt out.txt
```

Go
---

```bash
go build freq01.go
./freq01 pg.txt out.txt
```

Java
----

```bash
javac -d junk src/freq01.java
java -cp junk freq01 pg.txt out.txt
```

Rust
-----

```bash
cd build/rust
cargo build --release

cd ../..
./junk/freq01rust pg.txt out.txt
```

Scala
-----

```bash
cd build/scala
sbt assembly
cd ../..
java -jar ./junk/freq01scala.jar pg.txt out.txt
```

