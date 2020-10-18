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
  along with TOPK.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <iostream>
#include <vector>
#include <float.h>
using namespace std;	

#include "utils.hh"
#include "vdata.hh"
#include "iohandler.hh"

// #define _V_DEBUG_

/*
 * Generates the vertical representation of the data
 */
VData::VData( char* input_file, 
		cost_function_type *custom_cost_function) {
	_logger = Logger::Instance();
	n_items = n_tr = size = max_tr_len = 0;
	v_unmap = v_map = NULL;
	delta_cost_function = custom_cost_function;

	_temp_binary_file = new char [30];
	struct timeval tv;
	gettimeofday(&tv,NULL);
	srand(tv.tv_usec);
	sprintf(_temp_binary_file, "%X.%s", rand(), HDATA_TEMP_FILE_SUFFIX);

	vector<int> supports;
	firstScan(input_file, supports);
	secondScan(supports);

	false_positives = 0;
	false_negatives = size;
	covered_area = 0;
}

/*
 * Executes the first scan of the ascii file
 * and collects support statistics.
 */
void VData::firstScan(char* input_file, vector<int> &supports) {
	supports.reserve(1024);

	IO_Handler* reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_READER);
	if (!reader) {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not open input file." << endl);
	}

	IO_Handler* writer = IO_Handler::get_handler(_temp_binary_file, IO_Handler::TR_BINARY_WRITER);
	if (!writer) {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not temp output file."<< endl);
	}

	int* itemset = new int[HDATA_MAX_TR_SIZE];

	while ( !reader->eof() ) {
		int len = reader->read(itemset, HDATA_MAX_TR_SIZE);
		if (len==-1)  {
			LOGGER( _logger, Logger::FATAL,
					Logger::REM <<  "HDATA_MAX_TR_SIZE is too small"<< endl);
		}

		if (len>0) {
			// update singletons supports
			for (int i=0; i<len; i++) {
				if (itemset[i] >= (int)supports.size()) {
					supports.resize(itemset[i]+1,0);
				}
				supports[ itemset[i] ]++;
			}
			// rewrite in binary format
			writer->write(itemset, len);
		}
	}

	delete [] itemset;

	delete reader;
	delete writer;
}

/*
 * Remaps items from most frequent to least frequent
 * and removes from disk zero supports.
 */
void VData::secondScan(vector<int> &supports) {
	// create map and unmapping functions
	v_unmap = new int[supports.size()];
	if (!v_unmap)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM <<  "Could not create unmapping vector."<< endl);
	}
	v_map = new int[supports.size()];
	if (!v_map)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not create mapping vector."<< endl);
	}

	for (unsigned int i=0; i<supports.size(); i++)
		v_unmap[i]=i;

	indexQuickSort(0, (int)supports.size()-1, v_unmap, supports.begin(), false);

	for (unsigned int i=0; i<supports.size(); i++)
		v_map[v_unmap[i]] = i;

	// remove zero supports from n_items
	n_items = n_tr = size = 0;
	while( n_items < (int)supports.size() && supports[v_unmap[n_items]]>0 )
		n_items++;

	// init tid-lists
	vfile = new VData::VDataEntry [n_items];
	if (!vfile)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate memory for a new VData."<< endl);
	}
	for (int i=0; i<n_items; i++)
		resize(i,supports[v_unmap[i]]);

	// open the binary file
	IO_Handler* reader = IO_Handler::get_handler(_temp_binary_file, IO_Handler::TR_BINARY_READER);
	if (!reader)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not open input file."<< endl);
	}

	reader->rew();

	n_tr = 0;
	int* itemset = new int[HDATA_MAX_TR_SIZE];

	while (!reader->eof()) {
		int len = reader->read(itemset, HDATA_MAX_TR_SIZE);
		if (len>0) {
			for (int i=0; i<len; i++)
				itemset[i] = v_map[itemset[i]];

			// AT THE MOMENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// what occurs here is mapped to something different than -1
			addTransaction(n_tr, len, itemset);

			n_tr++;
			size+=len;
		}
	}

	delete reader;

	unlink(_temp_binary_file);

	delete [] itemset;
}

VData::~VData() {
	delete [] vfile;
	delete [] v_unmap;
	delete [] v_map;
	delete [] _temp_binary_file;
}

VData::VDataEntry::VDataEntry() {
}

VData::VDataEntry::VDataEntry(int len) {
}

void VData::VDataEntry::resize(int len) {
	//   max_len = len;
	//   tid_len = 0;
	//   covered = 0;
	//   noise.resize(0);
	//   if (tid_list) delete [] tid_list;
	//   tid_list = new int[max_len];
	//   if (!tid_list) fatal_error("Could not allocate memory for a resized VDataEntry.");
}

VData::VDataEntry::~VDataEntry() {
}

void VData::VDataEntry::append(int tid) {
	uncovered.insert(tid);
}

void VData::addTransaction(int tid, int len, int* itemset){
	if (max_tr_len<len) max_tr_len = len;
	for(int i=0; i<len; i++) {
		int new_item = itemset[i];
		if (new_item>=n_items)  {
			LOGGER( _logger, Logger::FATAL,
					Logger::REM << "Item is too large to be added in the vfile."<< endl);
		}
		vfile[itemset[i]].append(tid);
	}
}

void VData::pprint() {
	for (int i=0; i<n_items; i++) {
		cout << i << " :";
		vfile[i].pprint();
	}
}

void VData::VDataEntry::pprint() {
	for ( set<int>::iterator it =  uncovered.begin();
			it!=uncovered.end();
			it++)
		cout << " " << *it;
	cout << " |";
	for ( set<int>::iterator it =  covered.begin();
			it!=covered.end();
			it++)
		cout << " " << *it;
	cout << endl;
	//   for (int j=0; j<tid_len; j++)
	//     cout << " " << tid_list[j];
	//   cout << " |";
	//   for (unsigned int j=0; j<noise.size(); j++)
	//     cout << " " << noise[j];
	//   cout << endl;
}

void VData::VDataEntry::processTid(int n_tr, unsigned short* cov, unsigned short* uncov, unsigned short* noisy) {
//	for (int i=0; i<n_tr; i++)
//		noisy[i]++; // first add some noise in each tid
	for ( set<int>::iterator it =  uncovered.begin(); it!=uncovered.end(); it++) {
		uncov[*it]++;
//		noisy[*it]--;
	}
	for ( set<int>::iterator it =  covered.begin(); it!=covered.end(); it++) {
		cov[*it]++;
//		noisy[*it]--;
	}
	for ( set<int>::iterator it =  noise.begin(); it!=noise.end(); it++)
		noisy[*it]++;
}

/*
void VData::VDataEntry::processTid_OLD(int n_tr, unsigned short* cov, unsigned short* uncov, unsigned short* noisy) {
	for (int i=0; i<n_tr; i++)
		noisy[i]++; // first add some noise in each tid
	for ( set<int>::iterator it =  uncovered.begin(); it!=uncovered.end(); it++) {
		uncov[*it]++;
		noisy[*it]--;
	}
	for ( set<int>::iterator it =  covered.begin(); it!=covered.end(); it++) {
		cov[*it]++;
		noisy[*it]--;
	}
	for ( set<int>::iterator it =  noise.begin(); it!=noise.end(); it++)
		noisy[*it]++;
}
*/

void VData::VDataEntry::simulateCover(vector<bool>* supporting, int &new_fp, int &already_cov) {
	int n_tr = (int) supporting->size();
	new_fp = already_cov = 0;
	for(int i=0; i<n_tr; i++) {
		// find next useful id
		if (supporting->at(i)) {
			if (covered.count(i)==1)
				already_cov++;
			else if (uncovered.count(i)==0)
				new_fp++;
		}
	}
}

// i need both cov and uncovs
void VData::VDataEntry::cover(vector<bool>* supporting, int &new_fp, int &new_fn) {
	int n_tr = (int) supporting->size();
	new_fp = new_fn = 0;
	for(int i=0; i<n_tr; i++) {
		// find next useful id
		if (supporting->at(i)) {
			bool is_noise = covered.insert(i).second;
			if (is_noise) {
				is_noise = uncovered.erase(i)==0;
				if (is_noise) {
					noise.insert(i); // add to the list of noisy occurrences
					new_fp++;
				} else
					new_fn--;
			}
		}
	}
}


void VData::VDataEntry::testAddItem( vector<bool>* supporting, int support,
		int &new_noise, int &already_cov, int &old_false_p,
		unsigned short* noise_vector, int &max_noise_per_tr, int n_tr) {
	/*
	new_noise += support;
	old_false_p = 0;
	for ( set<int>::iterator it = uncovered.begin(); it!=uncovered.end(); it++) {
		if (supporting->at(*it)) { // take only transactions that support the current itemset
			new_noise--;                 // this will not add any new_noise
		}
	}
	for ( set<int>::iterator it =  covered.begin(); it!=covered.end(); it++) {
		if (supporting->at(*it)) { // take only transactions that support the current itemset
			new_noise--;                 // this will not add any new_noise
			already_cov++;           // no gain here !!!!!
		}
	}
	*/
	// for each supported transaction
	//  if it is not supported and not covered count noisy+old_false_p
	old_false_p = 0;
	max_noise_per_tr = 0;
	for (int i=0; i<n_tr; i++) {
		if (supporting->at(i)){
			if ( covered.find(i) == covered.end() ) {
				if ( uncovered.find(i) == uncovered.end() ) {
					new_noise++; 		// this is new noise !
					if (max_noise_per_tr < noise_vector[i]+1)
						max_noise_per_tr = noise_vector[i]+1;
				}
			} else {
				already_cov++; 			// this is already covered by a previous pattern
				if ( noise.find(i)!=noise.end() ) {
					old_false_p++;		// this was covered by adding noise previously
					if (max_noise_per_tr < noise_vector[i]+1)
						max_noise_per_tr = noise_vector[i]+1;
				}
			}
		}
	}
}


// Warning: we have a maximum noise of 255 !!
float VData::extendItemset( int* itemset, int &len, int &supp, 
		list<int>* residual, vector<bool>* supporting) {
#ifdef _V_DEBUG_
	cout << "transactions are : " << n_tr << endl;
#endif
	// covered occurrences by a previous pattern
	unsigned short* cov_tids = new unsigned short[n_tr];
	if (!cov_tids)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate covered tids vector."<< endl);
	}
	// not yet covered occurrences by any pattern
	unsigned short* uncov_tids = new unsigned short[n_tr];
	if (!uncov_tids)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate uncovered tids vector."<< endl);
	}
	// noise already present in current and overlapped patterns (ie. false positives)
	unsigned short* noisy_tids = new unsigned short[n_tr];
	if (!noisy_tids)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate transaction-wise noise vector."<< endl);
	}
	// noise already present in current and overlapped patterns (ie. false positives)
	unsigned short* noisy_items = new unsigned short[n_items];
	if (!noisy_items)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate item-wise noisy vector."<< endl);
	}
	for (int i=0; i<n_tr; i++)
		cov_tids[i] = uncov_tids[i] = noisy_tids[i] = 0;
	for (int i=0;i<n_items; i++)
		noisy_items[i]=0;

	// actual supporting transactions
	// true if the transaction was already included in the support
	supporting->clear();
	supporting->resize(n_tr, false); // this should be a bitvector

	// fill covered and uncovered
	for (int i=0; i<len; i++)
		vfile[itemset[i]].processTid(n_tr, cov_tids, uncov_tids, noisy_tids);

	//for (int i=0; i<n_tr; i++) {
	//	cout << "noise in tr " << i << " " << noisy_tids[i]<< endl;
	//}

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// find actual supporting transactions
	float cost;
	int noise   = 0;
	int covs    = 0;
	int support = 0;
	for (int i=0; i<n_tr; i++) {
		if ( uncov_tids[i]>0 &&                     // there is something to cover !
				cov_tids[i] + uncov_tids[i] == len ) { // no holes in here
			support++;
			covs += cov_tids[i];
			supporting->at(i) = true;

			// update noisy items
			for (int j=0; j<n_items; j++)
				if ( !vfile[j].isUncovered(i) )
					if ( !vfile[j].isCovered(i) or vfile[j].isNoise(i) )
						noisy_items[j]++;
		}
	}
	cost = delta_cost_function(len, support, noise, covs);
#ifdef _V_DEBUG_
	cout << "# ## ## VD clean core: " << len << "x" << support << " @ " << cost << endl;
#endif

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// extension loop
	list<int>::iterator it=residual->begin();
	bool improved = true;
	while(improved) {
		improved = false;

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// try extending transactions
		for (int i=0; i<n_tr; i++) {

			// should transactions be sorted by decreasing noise ?
			if (!supporting->at(i) && uncov_tids[i]>0) {  // there is something NEW to cover !
				// new noise to be added
				int new_holes = len - cov_tids[i] - uncov_tids[i];
				// new noise plus old noise
				int total_holes = new_holes + noisy_tids[i];

				// check false positives item-wise
				int max_noise_per_item = 0;
				for (int j=0; j<len; j++) {
					if ( !vfile[itemset[j]].isUncovered(i) ) {
						if ( !vfile[itemset[j]].isCovered(i) or vfile[itemset[j]].isNoise(i) )
							if (max_noise_per_item < noisy_items[itemset[j]] +1)
								max_noise_per_item = noisy_items[itemset[j]] +1;
					}
				}

#ifdef _V_DEBUG_
				cout << "## ## new noise per tr: " << new_holes << endl;
				cout << "## ## tot noise per tr: " << total_holes << endl;
				cout << "## ## max noise per it: " << max_noise_per_item << endl;
				cout << "## ## noise ratio r: " << float(total_holes)/float(len) << endl;
				cout << "## ## noise ratio c: " << float(max_noise_per_item)/float(support+1) << endl;
#endif

				if (!isTolerableTrans(len,total_holes)) continue;
				if (!isTolerableItem(support+1, max_noise_per_item)) continue;

				int   new_noise = noise + new_holes;
				int   new_covs  = covs  + cov_tids[i]; // coverage gain
				float new_cost  = delta_cost_function(len, support+1, new_noise, new_covs);
				if (new_cost<=cost) {
					support++;
					supporting->at(i) = true;
					// update noise per item
					for (int j=0; j<n_items; j++)
						if ( !vfile[j].isUncovered(i) )
							if ( !vfile[j].isCovered(i) or vfile[j].isNoise(i) )
								noisy_items[j]++;
					// update noise per transaction
					noisy_tids[i] += new_holes;

					cost  = new_cost;
					covs  = new_covs;
					noise = new_noise;
#ifdef _V_DEBUG_
					cout << "# ## ## VD noisy add tr: " << i << " : "
							<< len << "x" << support << " @ " << cost << endl;
#endif
				}
			}
		}

#ifdef _V_DEBUG_
		cout << "# ## ## VD noisy core: " << len << "x" << support << " @ " << cost << endl;
#endif

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// try extending items
		for (;len<MAXIMUM_LENGTH && it!=residual->end(); it++) {
#ifdef _V_DEBUG_
			cout << "# ## ## VD: trying to add item " << *it << endl;
#endif

			int new_noise = noise, new_covs = covs;
			int old_fp=0, max_noise_per_tr=0;
			vfile[*it].testAddItem( supporting, support, new_noise, new_covs, old_fp,
									noisy_tids, max_noise_per_tr, n_tr);

#ifdef _V_DEBUG_
			cout << "## ## new noise per it: " << (new_noise-noise) << endl;
			cout << "## ## tot noise per it: " << (new_noise-noise)+old_fp << endl;
			cout << "## ## max noise per tr: " << max_noise_per_tr << endl;
			cout << "## ## noise ratio r: " << float(max_noise_per_tr)/float(len+1) << endl;
			cout << "## ## noise ratio c: " << float((new_noise-noise)+old_fp)/float(support) << endl;
#endif

			if (!isTolerableItem(support, (new_noise-noise)+old_fp )) continue;
			if (!isTolerableTrans(len+1,max_noise_per_tr)) continue;

			float new_cost = delta_cost_function(len+1, support, new_noise, new_covs);
			if (new_cost<=cost) {
				// update noise per item
				noisy_items[*it] += (new_noise-noise);
				// update transaction and noise per transaction
				vfile[*it].processTid(n_tr, cov_tids, uncov_tids, noisy_tids);

				itemset[len++] = *it;
				cost  = new_cost;
				covs  = new_covs;
				noise = new_noise;
				improved = true;

#ifdef _V_DEBUG_
				cout << "# ## ## VD noisy add item: " << *it << ", new cost "<< cost << endl;
#endif
				it++; // go next item !
				break;
			}
		}
	}

	supp = support;


	delete [] cov_tids;
	delete [] uncov_tids;
	delete [] noisy_tids;
	delete [] noisy_items;

	return cost;
}

// TODO: set maximum length
float VData::extendItemset_1_00( int* itemset, int &len, int &supp,
		list<int>* residual, vector<bool>* supporting) {
#ifdef _V_DEBUG_
	cout << "transactions are : " << n_tr << endl;
#endif
	// should allocate this once and for all ?!?
	// support vector
	unsigned short* cov_tids = new unsigned short[n_tr];
	if (!cov_tids)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate covered tids vector."<< endl);
	}
	unsigned short* uncov_tids = new unsigned short[n_tr];
	if (!uncov_tids)  {
		LOGGER( _logger, Logger::FATAL,
				Logger::REM << "Could not allocate uncovered tids vector."<< endl);
	}
	for (int i=0; i<n_tr; i++)
		cov_tids[i] = uncov_tids[i] = 0;

	// actual supporting transactions
	supporting->clear();
	supporting->resize(n_tr, false); // this should be a bitvector

	// compute initial support vector
	for (int i=0; i<len; i++)
		vfile[itemset[i]].processTid(n_tr, cov_tids, uncov_tids, NULL);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// find actual supporting transactions
	float cost;
	int noise   = 0;
	int covs    = 0;
	int support = 0;
	for (int i=0; i<n_tr; i++) {
		if ( uncov_tids[i]>0 &&                     // there is something to cover !
				cov_tids[i] + uncov_tids[i] == len ) { // no holes in here
			support++;
			covs += cov_tids[i];
			supporting->at(i) = true;
		}
	}
	cost = delta_cost_function(len, support, noise, covs);
#ifdef _V_DEBUG_
	cout << "# ## ## VD clean core: " << len << "x" << support << " @ " << cost << endl;
#endif

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// extension loop
	list<int>::iterator it=residual->begin();
	bool improved = true;
	while(improved) {
		improved = false;

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// try extending transactions
		for (int i=0; i<n_tr; i++) {

			// should transactions be sorted by decreasing noise ?

			if (!supporting->at(i) && uncov_tids[i]>0) {  // there is something NEW to cover !
				int   holes     = len - cov_tids[i] - uncov_tids[i]; // noise to be added

				if (!isTolerableTrans(len,holes+cov_tids[i])) continue;

				int   new_noise = noise + holes;
				int   new_covs  = covs  + cov_tids[i]; // coverage gain
				float new_cost  = delta_cost_function(len, support+1, new_noise, new_covs);
				if (new_cost<=cost) {
					support++;
					supporting->at(i) = true;
					cost  = new_cost;
					covs  = new_covs;
					noise = new_noise;
#ifdef _V_DEBUG_
					cout << "# ## ## VD noisy add tr: " << i << " : "
							<< len << "x" << support << " @ " << cost << endl;
#endif
				}
			}
		}

#ifdef _V_DEBUG_
		cout << "# ## ## VD noisy core: " << len << "x" << support << " @ " << cost << endl;
#endif

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// try extending items
		for (;len<MAXIMUM_LENGTH && it!=residual->end(); it++) {
#ifdef _V_DEBUG_
			cout << "# ## ## VD: trying to add item " << *it << endl;
#endif

			int new_noise = noise, new_covs = covs;
			// vfile[*it].testAddItem( cov_tids, uncov_tids, len, support, noise,  gain );
			int dummy;
			vfile[*it].testAddItem( supporting, support, new_noise, new_covs, dummy, NULL, dummy, n_tr);

			if (!isTolerableItem(support,(new_noise-noise)+(new_covs-covs) ) ) continue;

			float new_cost = delta_cost_function(len+1, support, new_noise, new_covs);
			if (new_cost<=cost) {
				vfile[*it].processTid(n_tr, cov_tids, uncov_tids, NULL);
				itemset[len++] = *it;
				cost  = new_cost;
				covs  = new_covs;
				noise = new_noise;
				improved = true;

#ifdef _V_DEBUG_
				cout << "# ## ## VD noisy add item: " << *it << ", new cost "<< cost << endl;
#endif
				it++; // go next item !
				break;
			}
		}
	}

	supp = support;


	delete [] cov_tids;
	delete [] uncov_tids;

	return cost;
}

FPtree* VData::buildFPtree(int strategy, int* fp_map){
	FPtree* fptree    = new FPtree(strategy, VData::MAXIMUM_LENGTH, delta_cost_function);
	int* transactions = new int[BATCH_SIZE*max_tr_len];
	int* t_lengths    = new int[BATCH_SIZE];

	for (int i=0; i<n_items; i++) vfile[i].rewind();

	// read transactions in bunches
	for (int base = 0; base < n_tr; base+=BATCH_SIZE) {
		int limit = base + BATCH_SIZE;
		for (int i=0; i<BATCH_SIZE; i++) t_lengths[i]=0; // reset itemsets
		for (int i=0; i<n_items; i++) {
			// map the item
			int item, tid;
			if (fp_map!=0) item = fp_map[i];
			else           item = i;
			if (item==-1) continue;  // this item will be removed from the fp-tree
			while (vfile[i].hasTID() && (tid = vfile[i].getTID())<limit) {    // add to the transaction
				int offset = tid - base;
				transactions[offset*max_tr_len + t_lengths[offset]] = item;
				t_lengths[offset]++;
				vfile[i].nextTID();
				tid = vfile[i].getTID(); // ERRORE !!! too many skips ?!??!
			}
		}
		// cout << " reorder " << endl;
		// reorder and add to the fptree
		for (int i=0; i<BATCH_SIZE; i++)
			if (t_lengths[i]>0) {
				int* outset = transactions + i*max_tr_len;
				quickSort(0, t_lengths[i]-1, outset);
#ifdef _V_DEBUG_
				cout << "# ## adding to the fptree: ";
				for (int j=0; j<t_lengths[i]; j++)
					cout << outset[j] << " ";
				cout << endl;
#endif
				fptree->processItems(outset, t_lengths[i]);
				fptree->processTransaction(outset, t_lengths[i]);
			}
	}

	delete [] transactions;
	delete [] t_lengths;

	return fptree;
}

void VData::findDenseRegion( int* itemset, int &len, int &supp, 
		list<int> *residual, int strategy, float Temp) {
	len = 0;
	float curr_cost = FLT_MAX;
	set<int>* tlist = new set<int>();
	residual->clear();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// header (frequency) order
	if (strategy==FPtree::FREQUENCY_ORDER) {
		int* counts  = new int [getItems()];
		int* indices = new int [getItems()];
		for(int i=0; i<getItems(); i++) {
			counts[i]  = getFalseNegatives(i);
			indices[i] = i;
		}

		if (Temp>0) // randomize
			indexQuickSortRandom(0, getItems()-1, indices, counts, Temp, false);
		else // do not randomize
			indexQuickSort(0, getItems()-1, indices, counts, false);

		// try adding items one by one
		for(int i=0; len<MAXIMUM_LENGTH && i<getItems(); i++) {
			int next_item = indices[i];
			if ( getFalseNegatives(next_item) == 0) // no good items are left
				break;
			// cout << "testing item " << i << endl;
			if ( !addItemIfUseful( next_item, tlist,
					itemset, len, supp, curr_cost) )
				residual->push_back(next_item);
		}

		delete [] counts;
		delete [] indices;
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// next is the most correlated with 
	// the current itemset
	else if (strategy==FPtree::MOST_CORRELATED_ORDER) {
		// take the first randomly
		int* counts  = new int [getItems()];
		int* indices = new int [getItems()];
		for(int i=0; i<getItems(); i++) {
			counts[i]  = getFalseNegatives(i);
			indices[i] = i;
		}

		if (Temp>0) // randomize
			indexQuickSortRandom(0, getItems()-1, indices, counts, Temp, false);
		else // do not randomize
			indexQuickSort(0, getItems()-1, indices, counts, false);

		// the first item should always be good !
		int next_item = indices[0];
		bool added = addItemIfUseful( next_item, tlist, 
				itemset, len, supp, curr_cost);
		counts[next_item] = 0;

		while (added && len<MAXIMUM_LENGTH) { // should always get in here !
			// count supports
			added = false;

			// Count conditional supports
			for (int i=0; i<getItems(); i++) {
				if (counts[i]>0) {
					set<int> nlist;
					vfile[i].intersect(tlist, &nlist);
					counts[i] = nlist.size();
				}
			}

			if (Temp>0) // randomize
				indexQuickSortRandom(0, getItems()-1, indices, counts, Temp, false);
			else // do not randomize
				indexQuickSort(0, getItems()-1, indices, counts, false);

			// take the first that fits
			for (int i=0; !added && i<getItems() && counts[indices[i]]>0; i++) {
				next_item = indices[i];
				// cout << "# ## ## trying to add item " << next_item <<  "(" << header.size()-1<<")"<<endl;
				added = addItemIfUseful( next_item, tlist,
						itemset, len, supp, curr_cost);
				if (added)
					counts[next_item] = 0; // so that this is not considered any more
				// if (added)
				//   cout << "# ## ## ADDED item " << next_item <<  "(" << header.size()-1<<")"<<endl;
			}
		}
		// set residuals
		for (int i=0; i<getItems(); i++) 
			if (counts[i]>0) 
				residual->push_back(i);

		delete [] counts;
		delete [] indices;
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// itmes are sorted with a charm-like order
	else if (strategy==FPtree::CHARM_ORDER) {
		// build charm supps
		int* charm_supports = new int [getItems()];
		build_charms(charm_supports);

		// create counters
		int* indices = new int [getItems()];
		for (int i = 0; i<getItems(); i++)
			indices[i] = i;

		// sort by frequency
		if (Temp>0)
			indexQuickSortRandom(0, getItems()-1, indices, charm_supports, Temp, false);
		else
			indexQuickSort(0, getItems()-1, indices, charm_supports, false);

		// try adding item one by one
		for (int i=0; i<getItems() && charm_supports[indices[i]]>0; i++) {
			int next_item = indices[i];
			// cout << "# ## ## trying to add item " << next_item <<  "(" << charm_supports[next_item]<<")"<<endl;
			bool added = addItemIfUseful( next_item, tlist, itemset, len, supp, curr_cost);
			if (!added) residual->push_back(next_item);

			// if (added)
			// cout << "# ## ## ADDED item " << next_item <<  "(" << header.size()-1<<")"<<endl;
		}

		delete [] indices;
		delete [] charm_supports;
	}



	// final clean-up
	tlist->clear(); delete tlist;
	if (curr_cost>=0) // no good rectangle was found
		len = supp = 0;
}

bool VData::addItemIfUseful( int next_item, set<int>* &tlist, 
		int* itemset, int &len, int &supp, float &curr_cost) {
	bool inserted = false;
	set<int>* nlist = NULL; // candidate new tid-list

	if (len==0) {
		nlist = vfile[next_item].getCopy();
	} else {
		nlist = new set<int>();
		vfile[next_item].intersect(tlist,nlist);
	}
	int nsupp = nlist->size();
	// cout << "support is "<< nsupp << endl;
	if (nsupp>0) {
		float new_cost = delta_cost_function(len+1, nsupp, 0, 0);
		// cout << "# ## ## Testing: " << next_item << " : " << nsupp << " @ " << new_cost << endl;

		if (new_cost <= curr_cost) {
#ifdef _V_DEBUG_
			cout << "# ## ## VD added: " << next_item << " : " << (len+1) << "x" << nsupp << " @ " << new_cost << endl;
#endif
			inserted = true;
			itemset[len++] = next_item;
			supp = nsupp;
			curr_cost = new_cost;
			// swap lists
			std::swap(tlist, nlist);
		}
	}
	nlist->clear(); delete nlist;
	return inserted;
}

// build charms by partial de-inversion
void VData::build_charms(int* charm_supports) {
	for (int i=0; i<getItems(); i++)  
		charm_supports[i] = getFalseNegatives(i);

	int* transactions = new int[BATCH_SIZE*max_tr_len];
	int* t_lengths    = new int[BATCH_SIZE];

	for (int i=0; i<n_items; i++) vfile[i].rewind();

	// read transactions in bunches
	for (int base = 0; base < n_tr; base+=BATCH_SIZE) {
		int limit = base + BATCH_SIZE;
		for (int i=0; i<BATCH_SIZE; i++) t_lengths[i]=0; // reset itemsets
		for (int i=0; i<n_items; i++) {
			// map the item
			int tid;
			while (vfile[i].hasTID() && (tid = vfile[i].getTID())<limit) {    // add to the transaction
				int offset = tid - base;
				transactions[offset*max_tr_len + t_lengths[offset]] = i;
				t_lengths[offset]++;
				vfile[i].nextTID();
				tid = vfile[i].getTID();
			}
		}
		// cout << " reorder " << endl;
		// reorder and add to the fptree
		for (int i=0; i<BATCH_SIZE; i++)
			if (t_lengths[i]>0) {
				int* outset = transactions + i*max_tr_len;
				// quickSort(0, t_lengths[i]-1, outset); // don't need to sort anymore
#ifdef _V_DEBUG_
				cout << "# ## adding to the fptree: ";
				for (int j=0; j<t_lengths[i]; j++) 
					cout << outset[j] << " ";
				cout << endl;
#endif
				for (int a=0; a<t_lengths[i]-1; a++) {
					for (int b=a+1; b<t_lengths[i]; b++) {
						charm_supports[outset[a]]++;
						charm_supports[outset[b]]++;
					}
				}
			}
	}

	delete [] transactions;
	delete [] t_lengths;

}
