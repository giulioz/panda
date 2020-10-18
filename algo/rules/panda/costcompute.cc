/*
 Copyright (C) 2009, Claudio Lucchese

 This file is part of DMT.

 DMP is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 DMT is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TOPK. If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cfloat>

using namespace std;

#include "utils.hh"
#include "vdata.hh"
#include "fptree.hh"
#include "chronos.hh"
#include "iohandler.hh"
#include "basicstruct.hh"
#include "nullmodel.hh"

//#define _USE_FPTREE_

int main (int argc, char* argv[]) {
	Logger* logger = Logger::Instance();

	// presentation message
	LOGGER( logger, Logger::INFO, 
			Logger::REM << " ** CostCompute Copyright (C) 2011 Claudio Lucchese"   	<< endl <<
			Logger::REM << " ** Computes the cost of a given pattern set."	<< endl << endl
	)

	logger->selectLevel(Logger::INFO);

	// default settings
	char* dataset_file  = NULL;
	char* patterns_file  = NULL;
	cost_function_type *cost_function = &PANDA_delta_cost;
	initial_cost_function_type *initial_cost = &PANDA_initial_cost;
	float cost_alpha  = 0.5;

	bool useInformationRatio = false;

	// parse command line arguments
	bool args_ok = true;
	char option_char;
	while ( args_ok && (option_char= getopt(argc,argv, "d:p:c:w:")) != -1 ){
		if (option_char=='d') {
			dataset_file = optarg;
		} else if (option_char=='p') {
			patterns_file = optarg;
		} else if (option_char=='c') {
			switch (*optarg) {
			case '1': cost_function=PANDA_delta_cost;
					  initial_cost =PANDA_initial_cost; break;
			case '2': cost_function=PANDA_weighted_L2_delta_cost; break;
			case 'w': cost_function=PANDA_weighted_delta_cost;
					  initial_cost =PANDA_weighted_initial_cost; break;
			case 'x': cost_function=MDL_TypedXOR_delta_cost;
					  initial_cost =MDL_TypedXOR_initial_cost; break;
			case 'n': cost_function=MDL_NaiveXOR_delta_cost;
					  initial_cost =MDL_NaiveXOR_initial_cost; break;
			case 'r': useInformationRatio = true; break;
			}
		} else if (option_char=='w') {
			float aux = atof(optarg);
			if (aux>=0 && aux<=1.0) cost_alpha=aux;
			else args_ok = false;
		} else
			args_ok = false;
	}

	// check params and usage message
	if (!args_ok || dataset_file==NULL || patterns_file==NULL) {
		LOGGER( logger, Logger::WARN,
				"usage: " << argv[0] << endl <<
				"       -d <dataset>       input data file (MANDATORY)" << endl <<
				"       -p <patterns>      patterns data file in extended format (MANDATORY)" << endl <<
				"       -c <cost f>        1 - norm 1, w - norm s with weight," << endl <<
				"                          2 - norm 2," << endl <<
				"                          x - typed xor, n - naive xor" << endl <<
				"                          r - information ratio" << endl <<
				"                          def:1" << endl <<
				"       -w <weight>        weight to be used for the \"-c w\" option" << endl
		)

		return 1;
	}

	// output cost computation method
	if (useInformationRatio) {
		LOGGER( logger, Logger::INFO,
				Logger::REM << " Cost model: information ratio" << endl << endl )
	} else if (cost_function==PANDA_delta_cost) {
		LOGGER( logger, Logger::INFO,
				Logger::REM << " Cost model: norm1" << endl << endl )
	} else if (cost_function==PANDA_weighted_delta_cost) {
		LOGGER( logger, Logger::INFO,
				Logger::REM << " Cost model: weighted ("<< cost_alpha << ")" << endl << endl )
	} else if (cost_function==MDL_TypedXOR_delta_cost) {
		LOGGER( logger, Logger::INFO,
				Logger::REM << " Cost model: mdl-typed" << endl << endl )
	} else if (cost_function==MDL_NaiveXOR_delta_cost) {
		LOGGER( logger, Logger::INFO,
				Logger::REM << " Cost model: mdl-naive" << endl << endl )
	}

	if (!useInformationRatio) {
		// pattern input
		IO_Handler* patterns = NULL;
		if (patterns_file) patterns = IO_Handler::get_handler(patterns_file, IO_Handler::XSET_ASCII_READER);

		// vertical data
		VData vertData(dataset_file, cost_function);

		// set up auxiliary settings for cost computations
		costAuxSetup(cost_alpha, &vertData);

		LOGGER( logger, Logger::DEBUG,
				Logger::REM << Logger::REM << " Vertical build completed." << endl <<
				Logger::REM << Logger::REM << " No. Items:        " << vertData.getItems() << endl <<
				Logger::REM << Logger::REM << " No. Transactions: " << vertData.getTrans() << endl <<
				Logger::REM << Logger::REM << " Size:             " << vertData.getSize() << endl
		)


		int k = 0;
		float cost = initial_cost();
		LOGGER( logger, Logger::INFO,
				k << "\t" << cost << endl )

		int*       best_itemset         = new int [vertData.getItems()];
		int        best_len             = -1;
		vector<bool>* best_transactions = new vector<bool>(vertData.getTrans());

		DMT_ExtSuppSet p(vertData.getItems(), vertData.getTrans());

		while (! patterns->eof()) {
			k++;
			patterns->read(&p);

			// map itemset
			best_len = 0;
			for (int i=0; i<p.getLen(); i++)
				best_itemset[best_len++] = p.getItem(i);
			vertData.mapItemset(best_itemset, best_len);
			for (unsigned int i=0; i<best_transactions->size(); i++)
				(*best_transactions)[i] = false;
			for (int i=0; i<p.getTransactionsLen(); i++)
				(*best_transactions)[p.getTransaction(i)] = true;

			// compute noise
			int new_false_positives, new_covered;
			vertData.simulateUpdate(best_itemset, best_len, best_transactions, new_false_positives, new_covered);
			float delta_cost = cost_function(p.getLen(), p.getTransactionsLen(), new_false_positives, new_covered);

			LOGGER( logger, Logger::DEBUG,
					Logger::REM << " " << delta_cost << endl )

			// remove the pattern from the vertical data
			vertData.updateVertical(best_itemset, best_len, best_transactions);

			cost += delta_cost;
			LOGGER( logger, Logger::INFO,
					k << "\t" << cost << endl )

		}


		// free some memory
		delete [] best_itemset;
		delete best_transactions;
		if (patterns)
			delete patterns;


	} else {
		// -------------------------------------------
		// Information ratio
		// -------------------------------------------
		// logger->selectLevel(Logger::DEBUG);

		// read the dataset
		IO_Handler* reader = IO_Handler::get_handler(dataset_file, IO_Handler::TR_ASCII_READER);
		if (!reader) {
			LOGGER( logger, Logger::FATAL,
					Logger::REM << "Could not open input file." << endl);
		}

		int* itemset = new int[HDATA_MAX_TR_SIZE];
		vector<int> rowSums;
		vector<int> colSums;
		vector< vector<int> > dataset;

		while ( !reader->eof() ) {
			int len = reader->read(itemset, HDATA_MAX_TR_SIZE);
			if (len==-1)  {
				LOGGER( logger, Logger::FATAL,
						Logger::REM <<  "HDATA_MAX_TR_SIZE is too small"<< endl);
			}

			// update marginals
			bool items_have_changed = false;
			rowSums.push_back(len);
			if (len>0) {
				// update singletons supports
				for (int i=0; i<len; i++) {
					if (itemset[i] >= (int)colSums.size()) {
						colSums.resize(itemset[i]+1,0);
						items_have_changed = true;
					}
					colSums[ itemset[i] ]++;
				}
			}

			// update the dataset
			dataset.resize(dataset.size()+1);
			dataset.back().resize(colSums.size(),0);

			if (items_have_changed) // extend every row
				for ( vector< vector<int> >::iterator row=dataset.begin();
						row!=dataset.end(); row++)
					row->resize(colSums.size(),0);

			if (len>0) // update last transaction
				for (int i=0; i<len; i++)
					dataset.back()[itemset[i]] = 1;
		}

		delete [] itemset;
		delete reader;

		// build the null model
		NullModel maxEntModel(&rowSums, &colSums);

		// pattern input
		IO_Handler* patterns = NULL;
		if (patterns_file) patterns = IO_Handler::get_handler(patterns_file, IO_Handler::XSET_ASCII_READER);

		int k = 0;
		float informativeness = 0.0;
		LOGGER( logger, Logger::INFO,
				k << "\t" << informativeness << endl )

		DMT_ExtSuppSet p(colSums.size(), rowSums.size());
		while (! patterns->eof()) {
			k++;
			patterns->read(&p);

			// compute delta Information Content
			double noise = 0.0;
			double ic = 0.0;
			for (int row=0; row<p.getTransactionsLen(); row++) {
				for (int col=0; col<p.getLen(); col++) {
					int r = p.getTransaction(row);
					int c = p.getItem(col);
					if (dataset[r][c] == 1) { // present
						ic -= maxEntModel.logProbability(r,c,true);
					} else if (dataset[r][c] == 0) { // absent
						ic -= maxEntModel.logProbability(r,c,false);
						noise += 1;
					}
					dataset[r][c] = -1; // covered
				}
			}
			LOGGER( logger, Logger::DEBUG,
					Logger::REM << " Information Content "<< ic << endl);

			// compute Description length
			double dl = 0.0;
			set<int> items;
			for (int col=0; col<p.getLen(); col++)
				items.insert(p.getItem(col));
			for (unsigned int col=0; col<colSums.size(); col++){
				double p = ((float)colSums[col])/((float)rowSums.size());
				if (items.find(col)!=items.end()) // present
					dl -= log( p );
				else
					dl -= log( 1.0 - p );
			}
			set<int> transactions;
			for (int row=0; row<p.getTransactionsLen(); row++)
				transactions.insert(p.getTransaction(row));
			for (unsigned int row=0; row<rowSums.size(); row++){
				double p = ((float)rowSums[row])/((float)colSums.size());
				if (transactions.find(row)!=transactions.end()) // present
					dl -= log( p );
				else
					dl -= log( 1.0 - p );
			}
			if (noise!=0.0) {
				double p_n = noise/(float)(p.getLen()*p.getTransactionsLen());
				dl += (float)p.getLen() * (float)p.getTransactionsLen() *
						( -p_n*log(p_n) - (1.0-p_n)*log(1.0-p_n) );
			}
			LOGGER( logger, Logger::DEBUG,
					Logger::REM << " Description length " << dl << endl )

			// compute delta information ratio
			double delta_ir = ic/dl;


			LOGGER( logger, Logger::DEBUG,
					Logger::REM << " Delta Information Ratio " << delta_ir << endl )

			informativeness += delta_ir;

			LOGGER( logger, Logger::INFO,
					k << "\t" << informativeness << endl )
		}

		if (patterns)
			delete patterns;

	}
	return 0;
}
