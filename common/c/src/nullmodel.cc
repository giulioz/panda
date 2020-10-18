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

#include <assert.h>
#include <math.h>

#include "../inc/nullmodel.hh"

NullModel::NullModel(vector<int>* rowSums, vector<int>* colSums)
{
	_logger = Logger::Instance();

	n=rowSums->size();
	m=colSums->size();

	rowSumsq = new vector<int>();
	rowIndices = new vector<vector<int>*>();
	makeUnique(rowSums, rowSumsq, rowIndices);

	colSumsq = new vector<int>();
	colIndices = new vector<vector<int>*>();
	makeUnique(colSums, colSumsq, colIndices);

	nq = rowSumsq->size();
	mq = colSumsq->size();

	muq = new double[nq]; for (int i=0; i<nq; i++) muq[i]=0;
	lambdaq = new double[mq]; for (int i=0; i<mq; i++) lambdaq[i]=0;

	int* rowMult = new int [nq];
	for (int i=0; i<nq; i++) rowMult[i]=rowIndices->at(i)->size();
	int* colMult = new int [mq];
	for (int j=0; j<mq; j++) colMult[j]=colIndices->at(j)->size();

	// Start of the iteration
	int counter = 0;
	double infeasibility=1;
	double factors[10] = {0.1, 0.2, 0.3, 0.4, 0.55, 0.7, 0.85, 1, 1.2, 1.5};
	while (counter < 100 and infeasibility>n*m*1e-12)
	{
		LOGGER( _logger, Logger::VERBOSEMODE,
				Logger::REM << " Iteration: " << counter+1 << endl )
		// Compute gradient and Hessian diagonal
		double* gMuq = new double [nq];
		double* gLambdaq = new double [mq];
		double* hMuq = new double [nq];
		double* hLambdaq = new double [mq];
		//		// The following vectors are to speed up the computation of exp(muq[i]+lambdaq[j]) below.
		//		// However, it could give numerical difficulties if expMuq is small and expLambdaq is large.
		//		double expMuq[nq];
		//		double expLambdaq[mq];
		//		for (int i=0; i<nq; i++) expMuq[i]=exp(muq[i]);
		//		for (int j=0; j<mq; j++) expLambdaq[j]=exp(lambdaq[j]);

		// Note: each gMuq[i] should actually be multiplied by rowMult[i] to really be the gradient.
		// The way it is computed now, it is the mistake made for the row sum i.
		// Accordingly, each hMuq[i] should be multiplied by rowMult[i].
		// This is not done as it would be divided away anyway when the step direction is computed.
		// A similar thing holds for the lambda's.
		for (int i=0; i<nq; i++) {gMuq[i]=-rowSumsq->at(i); hMuq[i]=0;}
		for (int j=0; j<mq; j++) {gLambdaq[j]=-colSumsq->at(j); hLambdaq[j]=0;}
		for (int i=0; i<nq; i++)
		{
			for (int j=0; j<mq; j++)
			{
				double v=exp(muq[i]+lambdaq[j]); // Alternatively: double v = expMuq[i]*expLambdaq[j];
				double g = v/(1+v);
				double h = g/(1+v);
				gMuq[i] += g*colMult[j];
				gLambdaq[j] += g*rowMult[i];
				hMuq[i] += h*colMult[j];
				hLambdaq[j] += h*rowMult[i];
			}
		}
		infeasibility = 0;
		for (int i=0; i<nq; i++) infeasibility += gMuq[i]*gMuq[i]*rowMult[i];
		for (int j=0; j<mq; j++) infeasibility += gLambdaq[j]*gLambdaq[j]*colMult[j];
		LOGGER( _logger, Logger::VERBOSEMODE,
				Logger::REM << " Infeasibility (gradient norm squared): " << infeasibility << endl )

		// Compute step direction
		double* dMuq = new double [nq];
		double* dLambdaq = new double [mq];
		for (int i=0; i<nq; i++) dMuq[i]=-gMuq[i]/hMuq[i];
		for (int j=0; j<mq; j++) dLambdaq[j]=-gLambdaq[j]/hLambdaq[j];

		// Find optimal step size
		double bestFactor = 0;

		double bestError=0;
		if (true) // Use object function as criterion
		{
			for (int i=0; i<nq; i++) bestError -= rowMult[i]*muq[i]*rowSumsq->at(i);
			for (int j=0; j<mq; j++) bestError -= colMult[j]*lambdaq[j]*colSumsq->at(j);
			for (int i=0; i<nq; i++)
			{
				for (int j=0; j<mq; j++)
				{
					bestError += log(1+exp(muq[i]+lambdaq[j]))*rowMult[i]*colMult[j];
				}
			}
		}
		else // Use gradient norm as criterion
		{
			bestError = infeasibility;
		}

		for (int k=0; k<10; k++)
		{
			double factor = factors[k];
			double* muqTry = new double[nq];
			double* lambdaqTry = new double [mq];
			for (int i=0; i<nq; i++) muqTry[i]=muq[i]+dMuq[i]*factor;
			for (int j=0; j<mq; j++) lambdaqTry[j]=lambdaq[j]+dLambdaq[j]*factor;

			double errorTry = 0;
			if (true) // Use object function as criterion
			{
				for (int i=0; i<nq; i++) errorTry -= rowMult[i]*muqTry[i]*rowSumsq->at(i);
				for (int j=0; j<mq; j++) errorTry -= colMult[j]*lambdaqTry[j]*colSumsq->at(j);
				for (int i=0; i<nq; i++)
				{
					for (int j=0; j<mq; j++)
					{
						errorTry += log(1+exp(muqTry[i]+lambdaqTry[j]))*rowMult[i]*colMult[j];
					}
				}
			}
			else // Use gradient norm as criterion
			{
				double* gMuqTry = new double [nq];
				double* gLambdaqTry = new double [mq];
				for (int i=0; i<nq; i++) gMuqTry[i]=-rowSumsq->at(i);
				for (int j=0; j<mq; j++) gLambdaqTry[j]=-colSumsq->at(j);
				for (int i=0; i<nq; i++)
				{
					for (int j=0; j<mq; j++)
					{
						double v=exp(muqTry[i])*exp(lambdaqTry[j]);
						double g = v/(1+v);
						gMuqTry[i] += g*colMult[j];
						gLambdaqTry[j] += g*rowMult[i];
					}
				}
				double infeasibilityTry = 0;
				for (int i=0; i<nq; i++) infeasibilityTry += gMuqTry[i]*gMuqTry[i]*rowMult[i];
				for (int j=0; j<mq; j++) infeasibilityTry += gLambdaqTry[j]*gLambdaqTry[j]*colMult[j];
				errorTry = infeasibilityTry;

				delete [] gMuqTry;
				delete [] gLambdaqTry;
			}

			delete [] muqTry;
			delete [] lambdaqTry;

			if (errorTry<bestError)
			{
				bestError=errorTry;
				bestFactor = factor;
			}
			else break;
		}
		// cout << "Mu: ";
		for (int i=0; i<nq; i++)
		{
			muq[i] += dMuq[i]*bestFactor;
			//cout << muq[i] << " ";
		}
		//cout << endl;
		//cout << "Lambda: ";
		for (int j=0; j<mq; j++)
		{
			lambdaq[j] += dLambdaq[j]*bestFactor;
			//cout << lambdaq[j] << " ";
		}
		//cout << endl;
		counter++;

		//Compute the entropy -- the primal objective
		double entropy = 0;
		for (int i=0; i<nq; i++)
		{
			for (int j=0; j<mq; j++)
			{
				double p = exp(muq[i]+lambdaq[j])/(1+exp(muq[i]+lambdaq[j]));
				entropy = entropy - (p*log(p) + (1-p)*log(1-p))*rowMult[i]*colMult[j];
			}
		}

		LOGGER( _logger, Logger::VERBOSEMODE,
				Logger::REM << " Best factor: " << bestFactor
							<< " - Current cost: " << bestError
							<< " - Entropy: " << entropy << endl << endl )
		// The following line is to decrease the smallest possible stepsize if the optimal one was 0.
		if (bestFactor == 0) factors[0] = factors[0]/2;

		delete [] dMuq;
		delete [] dLambdaq;

		delete [] gMuq;
		delete [] gLambdaq;
		delete [] hMuq;
		delete [] hLambdaq;
	}

	mu = new double[n];
	lambda = new double[m];

	for (int i=0; i<nq; i++)
	{
		for (unsigned int k=0; k<rowIndices->at(i)->size(); k++) mu[rowIndices->at(i)->at(k)]=muq[i];
	}
	for (int j=0; j<mq; j++)
	{
		for (unsigned int k=0; k<colIndices->at(j)->size(); k++) lambda[colIndices->at(j)->at(k)]=lambdaq[j];
	}
	//	delete rowSumsq;
	//	delete colSumsq;
	//	for (int i=0; i<rowIndices->size(); i++) delete rowIndices->at(i);
	//	delete rowIndices;
	//	for (int i=0; i<colIndices->size(); i++) delete colIndices->at(i);
	//	delete colIndices;

	// This is to speed up the probability computation afterwards. Note however that the use of these vectors may be numerically inexact if there are very sparse rows as well as very dense columns, for example (but normally this should be fine).
	expMu = new double[n];
	//cout << "Mu: ";
	for (int i=0; i<n; i++) {expMu[i] = exp(mu[i]); //cout << expMu[i] << " ";
	} //cout << endl;
	expLambda = new double[m];
	//cout << "Lambda: ";
	for (int j=0; j<m; j++) {expLambda[j] = exp(lambda[j]); //cout << expLambda[j] << " ";
	} //cout << endl;


	delete [] rowMult;
	delete [] colMult;
}


double NullModel::probability(int row, int col) const
{
	assert(row<n and col<m);
	//	double e = exp(mu[row]+lambda[col]);
	double e = expMu[row]*expLambda[col]; // This is faster, but may be numerically inexact if there are very sparse rows as well as very dense columns, for example (but normally this should be fine).
	return e/(1+e);
}


double NullModel::logProbability(int row, int col, bool present) const
{
	assert(row<n and col<m);
	//	double e = exp(mu[row]+lambda[col]);
	double e = expMu[row]*expLambda[col]; // This is faster, but may be numerically inexact if there are very sparse rows as well as very dense columns, for example (but normally this should be fine).
	if (present)
		return log(e)-log(1+e);
	else
		return -log(1+e);
}


double NullModel::logProbability(set<int>* rows, set<int>* cols) const
{
	double out=0;
	set<int>::iterator rowStart;
	set<int>::iterator colStart;
	for (rowStart = rows->begin(); rowStart != rows->end(); rowStart++)
	{
		for (colStart = cols->begin(); colStart != cols->end(); colStart++)
		{
			out += logProbability(*rowStart,*colStart);
		}
	}
	return out;
}


double NullModel::logProbabilityTransaction(int row, set<int>* cols) const
{
	double out=0;
	set<int>::iterator colStart;
	for (colStart = cols->begin(); colStart != cols->end(); colStart++)
	{
		out += logProbability(row,*colStart);
	}
	return out;
}


double NullModel::minLogProbabilityTransaction(int row, set<int>* cols) const
{
	double out=0;
	set<int>::iterator colStart;
	for (colStart = cols->begin(); colStart != cols->end(); colStart++)
	{
		out = min(logProbability(row,*colStart),out);
	}
	return out;
}


void NullModel::makeUnique(vector<int>* sums, vector<int>* uniqueSums, vector<vector<int>*>* indices)
{
	uniqueSums->clear();
	for (unsigned int i=0; i<indices->size(); i++) indices->at(i)->clear();
	indices->clear();

	for (unsigned int i=0; i<sums->size(); i++)
	{
		int x=sums->at(i);
		unsigned int k;
		for (k=0 ; k<uniqueSums->size(); k++)
		{
			if (x==uniqueSums->at(k))
			{
				indices->at(k)->push_back(i);
				break;
			}
		}
		if (k==uniqueSums->size())
		{
			uniqueSums->push_back(x);
			indices->push_back(new vector<int>(1,i));
		}
	}
	/*
	for (unsigned int i=0; i<uniqueSums->size(); i++)
		cout << uniqueSums->at(i) << " ";
	cout << endl << endl;
	*/
}







