all : bin/freq01cpp bin/freq02cpp bin/freq01rs bin/hack01cpp bin/freq01go

clean:
	rm -f bin/freq01cpp bin/freq02cpp bin/freq01rs bin/hack01cpp bin/freq01go

bin/freq01cpp: src/freq01.cpp
	g++ -O3 -o bin/freq01cpp src/freq01.cpp

bin/freq02cpp: src/freq02.cpp
	g++ -O3 -o bin/freq02cpp src/freq02.cpp

bin/hack01cpp: src/hack01.cpp
	g++ -O3 -o bin/hack01cpp src/hack01.cpp

bin/freq01rs: src/freq01.rs
	cd build/rust && cargo build --release
	cp junk/rust/release/freq01 bin/freq01rs

bin/freq01go: src/freq01.go
	go build -o bin/freq01go src/freq01.go
