#include "simplex.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

int main(int argc, char* argv[]) {
    std::string text;

    if (argc >= 2) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Error: cannot open file " << argv[1] << std::endl;
            return 1;
        }
        text.assign(std::istreambuf_iterator<char>(file),
                     std::istreambuf_iterator<char>());
    } else {
        text.assign(std::istreambuf_iterator<char>(std::cin),
                     std::istreambuf_iterator<char>());
    }

    auto constraints = parse_constraints(text);
    solve(constraints);

    return 0;
}
