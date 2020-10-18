/*
 Copyright (C) 2009, Claudio Lucchese

 This file is part of TOPK.

 TOPK is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TOPK is distributed in the hope that it will be useful, but
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

//#define _USE_FPTREE_

int main (int argc, char* argv[]) {
	Logger* logger = Logger::Instance();

	// presentation message
	LOGGER( logger, Logger::INFO, 
			Logger::REM << " ** PaNDa Copyright (C) 2009 Claudio Lucchese"   	<< endl <<
			Logger::REM << " ** Error-tolerant frequent itemsets discovery."	<< endl << endl
	)

	// default settings
	int   k           = -1;
	int   strategy    = FPtree::FREQUENCY_ORDER;
	int   rand_iter   = 0;
	char* input_file  = NULL;
	char* output_file = NULL;
	cost_function_type *cost_function = &PANDA_delta_cost;
	initial_cost_function_type *initial_cost = &PANDA_initial_cost;
	float cost_alpha  = 0.5;
	bool use_fp_tree  = false;
	bool verbose_mode = false;
	float item_tolerance_ratio = 1.0; 	// 1 means full tolerance
	float tran_tolerance_ratio = 1.0;

	// parse command line arguments
	bool args_ok = true;
	char option_char;
	while ( args_ok && (option_char= getopt(argc,argv, "d:k:s:r:o:c:w:a:y:t:v")) != -1 ){
		if (option_char=='d')
			input_file = optarg;
		else if (option_char=='k') {
			int aux = atoi(optarg);
			if (aux!=0) k=aux;
			else args_ok = false;
		} else if (option_char=='s') {
			switch (*optarg) {
			case 'f': strategy = FPtree::FREQUENCY_ORDER; break;
			case 'c': strategy = FPtree::CHILD_FREQUENCY_ORDER; break;
			case 'o': strategy = FPtree::MOST_CORRELATED_ORDER; break;
			case 'h': strategy = FPtree::CHARM_ORDER; break;
			}
		} else if (option_char=='r') {
			int aux = atoi(optarg);
			if (aux >=0) rand_iter=aux;
			else args_ok = false;
		} else if (option_char=='o') {
			output_file = optarg;
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
			}
		} else if (option_char=='w') {
			float aux = atof(optarg);
			if (aux>=0 && aux<=1.0) cost_alpha=aux;
			else args_ok = false;
		} else if (option_char=='a') {
			switch (*optarg) {
			case 'f': use_fp_tree = true; break;
			case 'v': use_fp_tree = false; break;
			}
		} else if (option_char=='v') {
			verbose_mode = true;
		} else if (option_char=='t') {
			float aux = atof(optarg);
			if (aux>=0.0 && aux<=1.0)
				tran_tolerance_ratio = aux;
		} else if (option_char=='y') {
			float aux = atof(optarg);
			if (aux>=0.0 && aux<=1.0)
				item_tolerance_ratio = aux;
		} else
			args_ok = false;
	}

	if (k==-1) k = 0x7fffffff;

	// check params and usage message
	if (!args_ok || input_file==NULL) {
		LOGGER( logger, Logger::WARN,
				"usage: " << argv[0] << endl <<
				"       -d <dataset>       input data file (MANDATORY)" << endl <<
				"       -k <#patterns>     -1:infinity, def:-1" << endl <<
				"       -s <strategy>      f - for frequency, c - for child frequency," << endl <<
				"                          o - for correlated, h - for charm" << endl <<
				"                          def:f" << endl <<
				"       -r <# rnd iter>    0: no randomness, def:0" << endl <<
				"       -c <cost f>        1 - norm 1, w - norm s with weight," << endl <<
				"                          2 - norm 2," << endl <<
				"                          x - typed xor, n - naive xor" << endl <<
				"                          def:1" << endl <<
				"       -w <weight>        weight to be used for the \"-c w\" option" << endl <<
				"       -o <output>        def: non output file" << endl <<
				"       -a <data struct>   f - for fptree, v - for full vertical" << endl <<
				"                          def:f" << endl <<
				"       -y                 row tolerance ratio" << endl <<
				"       -t                 column tolerance ratio" << endl <<
				"       -v                 verbose mode. Outputs cost per iteration." << endl
		)

		return 1;
	}


	if (verbose_mode)
		logger->selectLevel(Logger::VERBOSEMODE);

	// timers
	__GLOBAL_CLOCK.StartChronos();

	// stats and info
	int PATTERNS         = 0;
	int PATTERNS_LENGTH  = 0;
	int PATTERNS_SUPP    = 0;
	float INPUT_COST         = 0.0;
	float PATTERN_MODEL_COST = 0.0;

	// file output
	IO_Handler* output = NULL;
	if (output_file) output = IO_Handler::get_handler(output_file, IO_Handler::P_ASCII_WRITER);

	// Console output
	IO_Handler* screen = IO_Handler::get_handler("/dev/stdout", IO_Handler::P_ASCII_WRITER);

	// vertical data
	VData vertData(input_file, cost_function);
#ifdef _V_DEBUG_
	vertData.pprint();
#endif

	// set up auxiliary settings for cost computations
	costAuxSetup(cost_alpha, &vertData, item_tolerance_ratio, tran_tolerance_ratio);
	INPUT_COST = initial_cost();
	PATTERN_MODEL_COST = INPUT_COST;

	LOGGER( logger, Logger::VERBOSEMODE,
			Logger::REM << " Cost at " << 0 << "  " << PATTERN_MODEL_COST << endl
	)


#ifdef _VERBOSE_
	LOGGER( logger, Logger::INFO, 
			"# Verical build completed." << endl <<
			"# ## Elapsed time: " << __GLOBAL_CLOCK.ReadChronos() << endl <<
			"# ## No. Items:        " << vertData.getItems() << endl <<
			"# ## No. Transactions: " << vertData.getTrans() << endl <<
			"# ## Size:             " << vertData.getSize() << endl
	)
#endif


	int* fp_map    = NULL;
	int* fp_unmap  = NULL;
	int* supports  = NULL;
	FPtree* fptree = NULL;
	if (use_fp_tree) {
		// map and unmap data
		fp_map   = new int [vertData.getItems()];
		fp_unmap = new int [vertData.getItems()];
		supports = new int [vertData.getItems()];
	}

	// top-K Loop
	for (int kiteration = 0;  kiteration<k; kiteration++) {
		if (use_fp_tree) {
			// create the mapping function
			for (int i=0; i<vertData.getItems(); i++) {
				supports[i] = vertData.getFalseNegatives(i);
				fp_unmap[i] = i;
			}
			indexQuickSort(0, vertData.getItems()-1, fp_unmap, supports, false);
			// create unmap function
			for (int i=0; i<vertData.getItems(); i++) 
				fp_map[fp_unmap[i]] = i;

			// remove zero support items
			for (int i=0; i<vertData.getItems(); i++) 
				if( supports[i] == 0)
					fp_map[i] = -1;

#ifdef _V_DEBUG_
			cout << "mapping: "<< endl;
			for (int i=0; i<vertData.getItems(); i++) 
				cout << i << " : " << fp_unmap[i] << " @ " << supports[fp_unmap[i]] << endl;
#endif

			// fp-tree
			fptree = vertData.buildFPtree(strategy, fp_map);
#ifdef _V_DEBUG_
			fptree->pprint();
#endif

#ifdef _VERBOSE_
			LOGGER( logger, Logger::INFO, 
					"# FP-Tree build completed." << endl <<
					"# ## Res. Items:        " << fptree->getItems() << endl <<
					"# ## Res. Transactions: " << fptree->getTrans() << endl <<
					"# ## Elapsed time: " << __GLOBAL_CLOCK.ReadChronos() << endl
			)
#endif

			if ( fptree->getTrans() == 0) {
				delete fptree;
				break;
			}
		}

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Randomized Loop
		int num_distinct_items;
		if (use_fp_tree) 
			num_distinct_items = fptree->getItems();
		else
			num_distinct_items = VData::MAXIMUM_LENGTH;  // can be reduced to frequent items only

		float      best_delta_cost      = FLT_MAX;
		int*       best_itemset         = new int [num_distinct_items];
		int        best_len             = -1;
		int        best_supp            = -1;
		list<int>* best_residual        = new list<int>;
		vector<bool>* best_transactions = new vector<bool>;

		float      curr_delta_cost;
		int*       curr_itemset          = new int [num_distinct_items];
		int        curr_len;
		int        curr_supp;
		list<int>* curr_residual         = new list<int>;
		vector<bool>* curr_transactions  = new vector<bool>;

		float Temp = 2.0*.9;
		if (rand_iter==0) Temp = 0.0;
		for (int rand_iter_i = 0; rand_iter_i<=rand_iter; rand_iter_i++) {
			Temp /=.9;
			curr_len = 0;

			if (use_fp_tree) {
				// find a dense region
				fptree->findDenseRegion(curr_itemset, curr_len, curr_supp, curr_residual, Temp);
				if (curr_len==0) continue;

				// unmap from fp-tree to vertical dataset
				for (int i=0; i<curr_len; i++)
					curr_itemset[i] = fp_unmap[curr_itemset[i]];
				for (list<int>::iterator it=curr_residual->begin(); it!=curr_residual->end(); it++)
					*it = fp_unmap[*it];
			} else {
				// find a dense region
				vertData.findDenseRegion(curr_itemset, curr_len, curr_supp, curr_residual, strategy, Temp);
				if (curr_len==0) continue;
			}

#ifdef _V_DEBUG_
			cout << "dense core is: ";
			for (int i=0; i<curr_len; i++)
				cout << curr_itemset[i] << " ";
			cout << "("<<curr_supp<<")"<<endl;

			for (list<int>::iterator it=curr_residual->begin(); it!=curr_residual->end(); it++)
				cout << "residual  " << *it << endl;
#endif

			// extend using also data already covered
			curr_delta_cost = vertData.extendItemset( curr_itemset, curr_len, curr_supp,
					curr_residual, curr_transactions);

			if (curr_delta_cost < best_delta_cost) {
				std::swap(best_delta_cost, curr_delta_cost);
				std::swap(best_itemset, curr_itemset);
				std::swap(best_len, curr_len);
				std::swap(best_supp, curr_supp);
				std::swap(best_residual, curr_residual);
				std::swap(best_transactions, curr_transactions);
			}
		}

		if (best_delta_cost <= 0.0) {
			// remove the pattern from the vertical data
			vertData.updateVertical(best_itemset, best_len, best_transactions);

			// console output
			vertData.unmapItemset(best_itemset, best_len);
			screen->write(best_itemset, best_len, NULL, best_supp);

			// file output
			if (output) {
				int* transactions = new int [best_supp];
				for (int i=0, j=0; i<vertData.getTrans(); i++)
					if (best_transactions->at(i))
						transactions[j++] = i;
				output->write(best_itemset, best_len, transactions, best_supp);
				delete transactions;
			}

			// some stats
			PATTERNS++;
			PATTERNS_LENGTH += best_len;
			PATTERNS_SUPP   += best_supp;
			PATTERN_MODEL_COST += best_delta_cost;

			LOGGER( logger, Logger::VERBOSEMODE,
					Logger::REM << " Cost at " << kiteration+1 << "  " << PATTERN_MODEL_COST << endl
			)
		}


		// free some memory
		if (use_fp_tree)
			delete fptree;
		delete [] best_itemset;
		delete best_residual;
		delete best_transactions;
		delete [] curr_itemset;
		delete curr_residual;
		delete curr_transactions;

		if (best_delta_cost > 0.0)
			break; // No pattern found
	}



	// representation stats:
	int FALSE_POSITIVES = 0;
	int FALSE_NEGATIVES = 0;
	int PATTERNS_COVER  = 0;
	for (int i=0; i<vertData.getItems(); i++) {
		FALSE_POSITIVES += vertData.getFalsePositives(i);
		FALSE_NEGATIVES += vertData.getFalseNegatives(i);
		PATTERNS_COVER  += vertData.getCovered(i);
	}

	LOGGER( logger, Logger::INFO, 
			endl <<
			Logger::REM << endl <<
			Logger::REM << " Input data cost : " << INPUT_COST << endl <<
			Logger::REM << " No. Patterns    : " << PATTERNS << endl <<
			Logger::REM << " Avg. length     : " << (float)PATTERNS_LENGTH/(float)PATTERNS << endl <<
			Logger::REM << " Avg. support    : " << (float)PATTERNS_SUPP/(float)PATTERNS << endl <<
			Logger::REM << " Patterns Cost   : " << PATTERNS_LENGTH+PATTERNS_SUPP << endl <<
			Logger::REM << " Patterns Cover  : " << PATTERNS_COVER << endl <<
			Logger::REM << " False Pos Cost  : " << FALSE_POSITIVES << endl <<
			Logger::REM << " False Neg Cost  : " << FALSE_NEGATIVES << endl <<
			Logger::REM << " Total Cost      : " << PATTERN_MODEL_COST << endl <<
			Logger::REM << " Cost Ratio      : " << PATTERN_MODEL_COST/INPUT_COST << endl <<
			Logger::REM << endl <<
			Logger::REM << " Total elapsed time : " << __GLOBAL_CLOCK.ReadChronos() << endl
	)

	if (use_fp_tree) {
		delete [] supports;
		delete [] fp_map;
		delete [] fp_unmap;
	}
	if (output)
		delete output;

	return 0;
}

/*
 * possible improvements:
 *
 * + remove bitvector for list<int>
 * + compare with LPCA
 * + salva strategy ...
 *
 * * cost is ok with stats, is there any redundant stat calc ?
 *
 * + bring back the horizontal data format for very large datasets
 *
 * + discuss in the paper the generation of residual items ...
 *   the actual solution is better than expected !!! not sure
 *
 * + plots rather table only at the end!
 *
 * + output delle statistiche opzionale ( do i want this ? )
 */
