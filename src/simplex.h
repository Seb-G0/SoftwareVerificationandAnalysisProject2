#pragma once
#include "parser.h"

// Original x = solution[columns.at("x")] - solution[columns.at("x") + 1].
// All columns except RHS are nonnegative variables. Last: auxiliary x0, epsilon, RHS.
// Last row is Phase I's z + x0 = 0, maximizing -x0 (slides 41-44).
// Phase II must remove x0 and restore/canonicalize z - epsilon = 0 (slide 48).
// A feasible starting basis is not chosen here. Objective coefficients are on the LHS.
std::vector<std::vector<Rational>> make_matrix(
    std::vector<Constraint>& constraints, std::map<std::string, int>& columns,
    std::vector<int>& basis);

void pivotify(std::vector<std::vector<Rational>>& matrix, int row, int column);

void solve(std::vector<Constraint> constraints);
