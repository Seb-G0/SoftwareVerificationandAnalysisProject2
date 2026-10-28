#pragma once
#include "parser.h"

// Original x = solution[columns.at("x")] - solution[columns.at("x") + 1].
// All columns except RHS are nonnegative variables. Last: auxiliary x0, epsilon, RHS.
// Last two rows: Phase I's z + x0 = 0 and Phase II's z - epsilon = 0.
// Both objective rows are updated by pivots. Phase II fixes the auxiliary x0 at zero.
// The initial basis contains slacks and may be infeasible. Coefficients are on the LHS.
std::vector<std::vector<Rational>> make_matrix(
    std::vector<Constraint>& constraints, std::map<std::string, int>& columns,
    std::vector<int>& basis);

void pivotify(std::vector<std::vector<Rational>>& matrix, int row, int column);

void solve(std::vector<Constraint> constraints);
