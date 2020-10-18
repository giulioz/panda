/*
  Copyright (C) 2009, Claudio Lucchese

  This file is part of DMT.

  DMT is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DMT is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DMT. If not, see <http://www.gnu.org/licenses/>.

  This code is a slight modification from Tijl De Bie MaxEnt software.
 */

#ifndef __DMT_NULLMODEL_H
#define __DMT_NULLMODEL_H

#include <vector>
#include <set>

using namespace std;

#include "../inc/logger.hh"


class NullModel
{
public:
	NullModel(vector<int>* rowSums, vector<int>* colSums);
	double probability(int row, int col) const;
	double logProbability(int row, int col, bool present=true) const;
	double logProbability(set<int>* rows, set<int>* cols) const;
	double logProbabilityTransaction(int row, set<int>* cols) const;
	double minLogProbabilityTransaction(int row, set<int>* cols) const;

	double* muq;
	double* lambdaq;
	vector<int>* rowSumsq;
	vector<int>* colSumsq;
	vector<vector<int>*>* rowIndices;
	vector<vector<int>*>* colIndices;
	int nq;
	int mq;
private:
	void makeUnique(vector<int>* sums, vector<int>* uniqueSums, vector<vector<int>*>* indices);
	double* mu;
	double* lambda;
	double* expMu;
	double* expLambda;
	int n;
	int m;

	Logger* _logger;
};


#endif
