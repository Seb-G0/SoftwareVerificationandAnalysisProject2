#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <map>
#include <string>
#include <vector>

using Rational = boost::multiprecision::cpp_rational;

struct Constraint {
    std::map<std::string, Rational> coefficients;
    std::string op;
    Rational rhs;
};

std::vector<Constraint> parse_constraints(const std::string& text);
