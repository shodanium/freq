Simple frequency dict benchmarks
=================================

Here there be descriptions and test results. Eventually.

In the meantime, here's a cookie, and a test file: https://yadi.sk/d/xTFWy38y3GBAgS

pg.txt, 336 183 276 bytes, MD5 9ca4282866bb919e03cca2ed2b6ce429

Rust
----

1. Go to `rust` subfolder

   ```bash
   $> cd rust
   ```

2. Run/build with `cargo`:

   - run: `cargo run --release -- pg.txt out.txt`
   - build: `cargo build --release && ./target/release/freq pg.txt out.txt`

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

