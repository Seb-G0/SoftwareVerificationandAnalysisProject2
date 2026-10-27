#pragma once
#include "rational.h"
#include <map>
#include <string>
#include <vector>

struct Constraint {
    std::map<std::string, Rational> coefficients;
    std::string op;
    Rational rhs;
};

std::vector<Constraint> parse_constraints(const std::string& text);
