#include "simplex.h"
#include <algorithm>
#include <set>
#include <stdexcept>
#include <iostream>

using namespace std;


vector<vector<Rational>> make_matrix(vector<Constraint>& cons, map<string, int>& cols, vector<int>& basis) {
	cols.clear();
	basis.clear();
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

		basis.push_back(firstSlack + matrix.size());
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
		basis.push_back(firstSlack + matrix.size());
		matrix.push_back(row);
	}

	vector<Rational> objective(cols.size() + 2, 0);
	objective[auxCol] = 1; // z + x0 = 0: maximize -x0
	matrix.push_back(objective);

	vector<Rational> epsRow(cols.size() + 2, 0);
	epsRow[epsCol] = -1;
	matrix.push_back(epsRow);

	return matrix;
}

vector<Rational> extract_soln(vector<vector<Rational>>& M, vector<int>& B) {
	int m = M[0].size();
	vector<Rational> out(m-1, 0);

	for(int j = 0; j < B.size(); j++) {
		out[B[j]] = M[j][m-1];
	}

	return out;
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

// Objective coefficients are on the LHS: a negative coefficient improves z.
void simplex(vector<vector<Rational>>& M, vector<int>& B, int ignoreRow, int ignoreCol) {
    int n = M.size();
    int m = M[0].size();
    int zind = (ignoreRow == n-1 ? n-2 : n-1);
    
    while(true) {
        // Bland's rule: choose the lowest-index improving column.
        int j = -1;
        for(int k = 0; k < m-1; k++) {
            if(k == ignoreCol) continue;
            if(M[zind][k] < 0) {
                j = k;
                break;
            }
        }

        if(j == -1) break;

        // get row of min ratio of RHS over new pivot coeff
        int i = -1;
        Rational min = 0;
        for(int k = 0; k < n; k++) {
            if (k == zind || k == ignoreRow || M[k][j] <= 0) continue;
            Rational ts = M[k][m-1] / M[k][j];
            if (i == -1 || ts < min || (ts == min && B[k] < B[i])) {
                min = ts;
                i = k;
            }
        }

        if(i == -1) throw std::runtime_error("LP UNBOUNDED");

        B[i] = j; 
        pivotify(M, i, j);
    }
}

void solve(vector<Constraint> cmds) {
	map<string, int> cols;
	vector<int> B;
	vector<vector<Rational>> M = make_matrix(cmds, cols, B);
	int n = M.size();
	int m = M[0].size();
	int auxCol = m - 3;

	// find most negative RHS
	int i = -1;
	Rational min_rhs = 0;
	for(int j = 0; j < n-2; j++) {
		if(M[j][m-1] < min_rhs) {
			min_rhs = M[j][m-1];
			i = j;
		}
	}

	// if negative RHS, pivotify most negative one
	if(min_rhs < 0) {
		pivotify(M, i, auxCol);
		B[i] = auxCol;
	}

	// simplex phase 1: optimize objective row (n-2), ignore epsilon row (n-1)
	simplex(M, B, n-1, -1);

	if(M[n-2][m-1] != 0) {
		cout << "UNSAT" << endl;
		return;
	}

	// Phase II must keep x0 at zero, even if it is still basic after Phase I.
	for (int row = 0; row < B.size(); ++row) {
		if (B[row] != auxCol) continue;
		int column = -1;
		for (int j = 0; j < m-1; ++j) {
			if (j != auxCol && M[row][j] != 0) {
				column = j;
				break;
			}
		}
		if (column != -1) {
			pivotify(M, row, column);
			B[row] = column;
		} else {
			M.erase(M.begin() + row);
			B.erase(B.begin() + row);
		}
		break;
	}
	n = M.size();
	
	// simplex phase 2: optimize epsilon row (n-1), ignore objective row (n-2) and aux column
	simplex(M, B, n-2, auxCol);

	if(M[n-1][m-1] <= 0) {
		cout << "UNSAT" << endl;
		return;
	}

	vector<Rational> raw_soln = extract_soln(M, B);

	// Collect original variable names from the constraints
	set<string> names;
	for (const auto& con : cmds)
		for (const auto& [name, coeff] : con.coefficients)
			names.insert(name);

	// Print each original variable's value: x = x_pos - x_neg
	for (const auto& name : names) {
		int col = cols.at(name);
		Rational val = raw_soln[col] - raw_soln[col + 1];
		cout << name << "=" << val << endl;
	}
}
