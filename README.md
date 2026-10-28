# Project 2

Andy Ma and Sebastian Gonzalez

## Build Instructions

Run `make`. `make test` also runs benchmarks in benchmarks/sat and benchmarks/unsat. `make clean` deletes the binary.

## Libraries Used

We used peglib for parsing and boost for rational arithmetic.

## Project Description

The program takes either standard input or a single file name as the only command line argument.

src/parser.cpp uses peglib to parse the input file.
src/simplex.cpp implements the two-stage simplex algorithm from class to determine sat or unsat on rational inequalities.
