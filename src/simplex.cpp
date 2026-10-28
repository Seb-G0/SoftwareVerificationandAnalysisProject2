#include "rational.h"
#include <vector>
#include <ratio>
#include <algorithm>

using namespace std;

struct command;

// take vector of commands and produce LP matrix
// last col corresponds to the y variable introduced to handle strict inequalities
vector<vector<Rational>> make_matrix(vector<command> cmds) {

	// count vars
	// count eqns, equalities count for two
	// vars + # eqns = total vars 
	// (?) create dict from var name to index <- this guy needs to make it back to caller
	// iterate over cmds and append to output 2d vec:
	//     <= maps directly
	//     >= needs to be flipped
	//     < gains y on LHS
	//     > gets flipped and gains y on LHS
	//     = becomes >= and <= (flip)
	// append Z-row (just the y var)
}

// removes fractions from matrix by multiplying by LCM of denominators
int defraction(vector<vector<Rational>>& M) {
}

vector<int> solve(vector<command> cmds) {
	// vec<vec<int>> M = make_matrix(cmds);
	// defraction(M); => clean LP problem

	// TODO
	return vector<Rational>();
}

void pivotify(vector<vector<Rational>>& M, int i, int j) {
	if(M[i][j] == 0) return;

	// make pivot 1
	Rational pivot = M[i][j]; // != 0 by the above guard
	if(pivot != 1) {
		for(int k = 0; k < M[0].size(); k++) {
			M[i][k] /= pivot;
		}
	}

	// subtract from other rows
	for(int k = 0; k < M.size(); k++) {
		if(k == i || M[k][j] == 0) continue;
		Rational factor = M[k][j];
		for(int l = 0; l < M[0].size(); l++) {
			M[k][l] -= factor * M[i][l];
		}
	}
}

// in: matrix representing LP tableau, all >=, with Z-row
void simplex(vector<vector<Rational>>& M) {
	int n = M.size();
	int m = M[0].size();
	while(true) {
		// get col of max coeff in Z-row
		int j = -1;
		for(int k = 0; k < m-1; k++) {
			if(M[n-1][k] > 0) {
				j = k;
				break;
			}
		}

		if(j == -1) break;

		// get row of min ratio of RHS over new pivot coeff
		int i = -1;
		Rational min = 0;
		for(int k = 0; k < n-1; k++) {
			if (M[k][j] <= 0) continue;
			Rational ts = M[k][m-1] / M[k][j];
			if (i == -1 || ts < min) {
				min = ts;
				i = k;
			}
		}

		if(i == -1) throw std::runtime_error("LP UNBOUNDED");

		pivotify(M, i, j);
	}
}
