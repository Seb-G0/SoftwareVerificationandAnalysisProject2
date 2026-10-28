#include "simplex.h"
#include <algorithm>
#include <set>
#include <stdexcept>

using namespace std;


vector<vector<Rational>> make_matrix(vector<Constraint>& cons, map<string, int>& cols) {
	cols.clear();
	set<string> names;
	for (const auto& con : cons)
		for (const auto& [name, coeff] : con.coefficients)
			names.insert(name);

	// Avoid collisions with actual input names such as xprime or slack1.
	auto extra_column = [&](string name) {
		while (names.count(name) || cols.count(name)) name += "prime";
		int index = cols.size();
		cols[name] = index;
	};
	for (const auto& name : names) {
		int index = cols.size();
		cols[name] = index;
		extra_column(name + "prime");
	}

	int firstSlack = cols.size();
	int rows = 2; // epsilon >= 0 and epsilon <= 1
	for (const auto& con : cons) rows += con.op == "=" ? 2 : 1;
	for (int i = 0; i < rows; ++i) extra_column("slack" + to_string(i + 1));

	int auxCol = cols.size();
	extra_column("x0");
	int epsCol = cols.size();
	int rhsCol = epsCol + 1;

	vector<vector<Rational>> matrix;

	auto add_row = [&](const Constraint& con, int sign, bool strict){
		vector<Rational> row(cols.size() + 2, 0);

		for (const auto& [name, coeff] : con.coefficients){
			int column = cols.at(name);
			row[column] = sign * coeff;
			row[column + 1] = -sign * coeff;
		}

		row[firstSlack + matrix.size()] = 1;
		row[auxCol] = -1;
		row[epsCol] = strict ? 1 : 0;
		row[rhsCol] = sign * con.rhs;

		matrix.push_back(row);
	};

	for (const auto& con : cons){
		if (con.op == "<=" || con.op == "<"){
			add_row(con, 1, con.op == "<");
		}
		else if (con.op == ">=" || con.op == ">"){
			add_row(con, -1, con.op == ">");
		}
		else if (con.op == "=") {
			add_row(con, 1, false);
			add_row(con, -1, false);
		}
	}

	for (int sign : {-1, 1}) {
		vector<Rational> row(cols.size() + 2, 0);
		row[firstSlack + matrix.size()] = 1;
		row[auxCol] = -1;
		row[epsCol] = sign;
		row[rhsCol] = sign == 1 ? 1 : 0;
		matrix.push_back(row);
	}

	vector<Rational> objective(cols.size() + 2, 0);
	objective[auxCol] = 1;
	matrix.push_back(objective);

	return matrix;
}

/* Unfinished solver methods; enable them when their implementations are ready.

// removes fractions from matrix by multiplying by LCM of denominators
int defraction(vector<vector<Rational>>& M) {
}

vector<int> solve(vector<command> cmds) {
	// vec<vec<int>> M = make_matrix(cmds);
	// defraction(M); => clean LP problem

	// TODO
	return vector<Rational>();
}
*/

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
