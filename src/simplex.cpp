#include "simplex.h"
#include <algorithm>
#include <set>
#include <stdexcept>
#include <iostream>

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

vector<Rational> extract_soln(vector<vector<Rational>>& M, vector<int>& B) {
	int m = M[0].size();
	vector<Rational> out(m-1, 0);

	for(int j = 0; j < B.size(); j++) {
		out[B[j]] = M[j][m-1];
	}

	return out;
}

void solve(vector<command> cmds) {
	vector<vector<Rational>> M = make_matrix(cmds);
	int n = M.size();
	int m = M[0].size();

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
	if(max < 0) {
		pivotify(M, i, m-2);
		B[i] = m-2;
	}

	// simplex phase 1
	simplex(M, B, n-2, -1);

	if(M[n-1][m-1] != 0) {
		cout << "UNSAT" << endl;
		return;
	}
	
	simplex(M, B, n-1, m-2);

	vector<Rational> raw_soln = extract_soln(M, B);
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
void simplex(vector<vector<Rational>>& M, vector<int>& B, int ignoreRow, int ignoreCol) {
    int n = M.size();
    int m = M[0].size();
    int zind = (ignoreRow == n-1 ? n-2 : n-1);
    
    while(true) {
        // get col of max coeff in Z-row
        int j = -1;
        for(int k = 0; k < m-1; k++) {
            if(k == ignoreCol) continue;
            if(M[zind][k] > 0) {
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
            if (i == -1 || ts < min) {
                min = ts;
                i = k;
            }
        }

        if(i == -1) throw std::runtime_error("LP UNBOUNDED");

        B[i] = j; 
        pivotify(M, i, j);
    }
}
