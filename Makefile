# Requires a C++17 compiler, curl, and tar. Libraries download once, locally.
.PHONY: all clean

all: third_party/boost_1_88_0/.ready third_party/cpp-peglib/peglib.h third_party/cpp-peglib/LICENSE
	mkdir -p build
	c++ -std=c++17 -Ithird_party/cpp-peglib -Ithird_party/boost_1_88_0 \
	    src/main.cpp src/parser.cpp -o build/parser -pthread
	./build/parser benchmarks/sat/*.txt benchmarks/unsat/*.txt

third_party/boost_1_88_0/.ready:
	mkdir -p third_party
	curl -fL --retry 3 https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.bz2 -o third_party/boost_1_88_0.tar.bz2
	tar -xjf third_party/boost_1_88_0.tar.bz2 -C third_party boost_1_88_0/boost boost_1_88_0/LICENSE_1_0.txt
	rm -f third_party/boost_1_88_0.tar.bz2
	touch $@

third_party/cpp-peglib/peglib.h:
	mkdir -p third_party/cpp-peglib
	curl -fL --retry 3 https://raw.githubusercontent.com/yhirose/cpp-peglib/v1.29.0/peglib.h -o $@.tmp
	mv $@.tmp $@

third_party/cpp-peglib/LICENSE:
	mkdir -p third_party/cpp-peglib
	curl -fL --retry 3 https://raw.githubusercontent.com/yhirose/cpp-peglib/v1.29.0/LICENSE -o $@.tmp
	mv $@.tmp $@

clean:
	rm -f build/parser
