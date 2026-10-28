# Requires a C++17 compiler, curl, and tar. Libraries download once, locally.
.PHONY: all clean test

all: third_party/boost_1_88_0/.ready third_party/cpp-peglib/peglib.h third_party/cpp-peglib/LICENSE
	mkdir -p build
	c++ -std=c++17 -Ithird_party/cpp-peglib -Ithird_party/boost_1_88_0 \
	    src/main.cpp src/parser.cpp src/simplex.cpp -o build/simplex -pthread

test: all
	@failed=0; \
	for f in benchmarks/sat/*.txt benchmarks/unsat/*.txt; do \
		expected=SAT; case "$$f" in benchmarks/unsat/*) expected=UNSAT;; esac; \
		if output=$$(./build/simplex "$$f"); then \
			if [ "$$output" = "UNSAT" ]; then actual=UNSAT; else actual=SAT; fi; \
			if [ "$$actual" = "$$expected" ]; then echo "PASS: $$actual $$f"; \
			else echo "FAIL: $$f (expected $$expected, got $$actual)"; failed=1; fi; \
		else echo "ERROR: $$f"; failed=1; fi; \
	done; exit $$failed

third_party/boost_1_88_0/.ready:
	mkdir -p third_party
	@if [ ! -d third_party/boost_1_88_0/boost ]; then \
		curl -fL --retry 3 https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.bz2 -o third_party/boost_1_88_0.tar.bz2 && \
		tar -xjf third_party/boost_1_88_0.tar.bz2 -C third_party boost_1_88_0/boost boost_1_88_0/LICENSE_1_0.txt && \
		rm -f third_party/boost_1_88_0.tar.bz2; \
	fi
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
	rm -f build/simplex
