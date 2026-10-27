all:
	mkdir -p build
	c++ -std=c++17 -Ithird_party/cpp-peglib \
	    -I/opt/homebrew/include -I/usr/local/include -I/opt/anaconda3/include \
	    src/*.cpp -o build/parser -pthread
	./build/parser benchmarks/sat/*.txt benchmarks/unsat/*.txt

clean:
	rm -f build/parser
