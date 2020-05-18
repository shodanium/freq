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
| 1.481..1.501s   | 227.0 | freq03.cpp | vs2019 16.1.0       |
| 1.747..1.778s   | 192.5 | freq02.cpp | vs2019 16.1.0       |
| 2.462..2.491s   | 136.6 | freq01.cpp | vs2019 16.1.0       |
| 2.655..2.675s   | 126.6 | freq01.rs  | rustc 1.43.1        |
| 4.397..4.512s   | 76.5  | freq01.go  | go 1.14.1           |
| 30.973..31.318s | 10.9  | freq01.php | php 7.0.20-zts      |
|                 |       |            |                     |
| 1.407..1.416s   | 239.0 | hack01.cpp | vs2019 16.1.0       |

**Xeon Gold 6240 @ 2.60 GHz, Debian 9**

| min..max        | MB/s  | program    | compiler or runtime |
|-----------------|-------|------------|---------------------|
| 1.443..1.457s   | 232.9 | freq03.cpp | gcc 6.3.0           |
| 1.492..1.521s   | 225.4 | freq02.cpp | gcc 6.3.0           |
| 2.197..2.245s   | 153.0 | freq01.rs  | rustc 1.43.1        |
| 2.480..2.737s   | 135.5 | freq01.cpp | gcc 6.3.0           |
| 3.880..4.187s   | 86.6  | freq01.go  | go 1.14.3           |
| 9.037..9.173s   | 37.2  | freq01.php | php 7.4.6-nts       |
|                 |       |            |                     |
| 1.349..1.442s   | 249.2 | hack01.cpp | gcc 6.3.0           |

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

1. Go to `build\scala` subfolder and run `sbt` from it (running in sbt shell mode is much faster):

   ```bash
   $ cd scala
   $ sbt
   ```

2. Run from `sbt shell`:

   ```bash
   freq(master)> run pg.txt out.txt
   ```
