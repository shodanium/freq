all : bin/freq01cpp bin/freq02cpp bin/freq03cpp bin/freq01rs bin/hack01cpp bin/freq01go bin/freq01scala.jar

clean:
	rm -f bin/freq01cpp bin/freq02cpp bin/freq03cpp bin/freq01rs bin/hack01cpp bin/freq01go

bin/freq01cpp: src/freq01.cpp
	g++ -O3 -o bin/freq01cpp src/freq01.cpp

bin/freq02cpp: src/freq02.cpp
	g++ -O3 -o bin/freq02cpp src/freq02.cpp

bin/freq03cpp: src/freq03.cpp
	g++ -O3 -o bin/freq03cpp src/freq03.cpp

bin/hack01cpp: src/hack01.cpp
	g++ -O3 -o bin/hack01cpp src/hack01.cpp

bin/freq01rs: src/freq01.rs
	cd build/rust && cargo build --release
	cp junk/rust/release/freq01 bin/freq01rs

bin/freq01go: src/freq01.go
	go build -o bin/freq01go src/freq01.go

bin/freq01scala.jar: src/freq01.scala
	cd build/scala && sbt assembly
	cp junk/freq01scala.jar bin/
