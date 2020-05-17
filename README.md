Simple frequency dict benchmarks
=================================

Here there be descriptions and test results. Eventually.

In the meantime, here's a cookie, and a test file: https://yadi.sk/d/xTFWy38y3GBAgS

pg.txt, 336 183 276 bytes, MD5 9ca4282866bb919e03cca2ed2b6ce429

Benchmark results
------------------

**i5-4690 @ 3.7 GHz, Windows 7 x64**

| min..max        | MB/s  | program    | compiler or runtime |
|-----------------|-------|------------|---------------------|
| 1.747..1.778s   | 192.5 | freq02.cpp | vs2019 16.1.0       |
| 2.506..2.524s   | 134.1 | freq01.cpp | vs2019 16.1.0       |
| 2.655..2.675s   | 126.6 | freq01.rs  | rustc 1.43.1        |
| 4.397..4.512s   | 76.5  | freq01.go  | go 1.14.1           |
| 30.973..31.318s | 10.9  | freq01.php | php 7.0.20-zts      |
|                 |       |            |                     |
| 1.331..1.339s   | 252.6 | hack01.cpp | vs2019 16.1.0       |

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

Rust
-----

```bash
cd rust
cargo build --release

cd ..
./rust/target/release/freq01 pg.txt out.txt
```

Scala
-----

1. Go to `scala` subfolder and run `sbt` from it (running in sbt shell mode is much faster):

   ```bash
   $> cd scala
   $> sbt
   ```

2. Run from `sbt shell`:

   ```bash
   freq(master)> run pg.txt out.txt
   ```
