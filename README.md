Simple frequency dict benchmarks
=================================

Here there be descriptions and test results. Eventually.

In the meantime, here's a cookie, and a test file: https://yadi.sk/d/xTFWy38y3GBAgS

pg.txt, 336 183 276 bytes, MD5 9ca4282866bb919e03cca2ed2b6ce429

Rust
----

```bash
$> cd rust
```

To run: `cargo run --release -- pg.txt out.txt`

To build: `cargo build --release && ./target/release/freq pg.txt out.txt`

