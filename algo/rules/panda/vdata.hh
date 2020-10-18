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

#ifndef __VDATA_H
#define __VDATA_H

#include <vector>
#include <list>
#include <set>
#include <algorithm>
using namespace std;

#include "logger.hh"

#include "fptree.hh"
#include "costs.hh"

#define HDATA_TEMP_FILE_SUFFIX      "HDATA.TEMP"
#define HDATA_MAX_TR_SIZE    4096

// #define VDATA_MAX_PATTERN_SIZE 255
// typedef char VDATA_COUNTER;

class VData {
public:
  
  class VDataEntry {
  public:
    VDataEntry();
    VDataEntry(int len);
    ~VDataEntry();

    void resize(int len);
    void append(int tid);

    void processTid(int n_tr, unsigned short* cov, unsigned short* uncov, unsigned short* noisy);
    void cover(vector<bool>* supporting, int &new_fp, int &new_fn);
    void simulateCover(vector<bool>* supporting, int &new_fp, int &already_cov);

    /* @param old_false_p the number of false positives generated
     * 						by previous patterns
     * @param max_noise_per_tr is the max noise being added
     * 						in the supporting transactions
     */
    void testAddItem( vector<bool>* supporting, int support,
    				int &noise, int &already_cov, int &old_false_p,
    				unsigned short* noise_vector, int &max_noise_per_tr, int n_tr);

    inline int getCovered() 	{ return covered.size();}
    inline int getUncovered() 	{ return uncovered.size();}
    inline int getNoise() 		{ return noise.size();}

    inline void rewind()  {pointer=uncovered.begin();}
    inline int  getTID()  {return *pointer;}
    inline bool hasTID()  {return pointer!=uncovered.end();}
    inline void nextTID() {pointer++;}

    inline bool isCovered(int j)	{ return covered.find(j)!=covered.end(); }
    inline bool isUncovered(int j)	{ return uncovered.find(j)!=uncovered.end(); }
    inline bool isNoise(int j)		{ return noise.find(j)!=noise.end(); }

    inline void intersect(set<int>* in, set<int>* out) { 
      set_intersection(uncovered.begin(),uncovered.end(),in->begin(),in->end(),inserter(*out, out->begin())); }
    inline set<int>* getCopy() {return new set<int>(uncovered);}

    void pprint();

  private:
    set<int> uncovered;         // uncovered TIDS
    set<int> covered;           // covered TIDS
    set<int> noise;				// noisy occurrences
    // WARNING: noise is a subset of covered

    set<int>::iterator pointer; // linear scanner
  };


  VData( char* input_file,  
	 cost_function_type *custom_cost_function = &PANDA_delta_cost);
  ~VData();

  void resize(int i, int size) {vfile[i].resize(size);}
  void addTransaction(int tid, int len, int* items);

  inline int getItems() const {return n_items;}
  inline int getTrans() const {return n_tr;}
  inline int getSize() const {return size;}
  inline int getFalsePositives(int i) { return vfile[i].getNoise();}
  inline int getFalseNegatives(int i) { return vfile[i].getUncovered();}
  inline int getCovered(int i) {return vfile[i].getCovered();}
  inline int getTotalFalsePositives() { return false_positives; }
  inline int getTotalFalseNegatives() { return false_negatives; }
  inline int getTotalCoveredArea()    { return covered_area; }

  inline void unmapItemset(int* set, int len)  // from vertical to original mapping
  { for (int i=0; i<len; i++) set[i]=v_unmap[set[i]];  }
  inline void mapItemset(int* set, int len)  // from original mapping to mapping
  { for (int i=0; i<len; i++) set[i]=v_map[set[i]];  }

  void pprint();

  // maximum itemset length
  static const int MAXIMUM_LENGTH = 254;
  float extendItemset(int* itemset, int &len, int &supp, list<int>* residual, vector<bool>* supporting);

  void updateVertical(int* pattern, int len, vector<bool>* supporting) {
	int delta_false_positives, delta_false_negatives;
    for(int i=0; i<len; i++) {
    	vfile[pattern[i]].cover(supporting, delta_false_positives, delta_false_negatives);
        false_positives += delta_false_positives;
        false_negatives += delta_false_negatives;
        covered_area    += delta_false_positives - delta_false_negatives;
    }
  }

  void simulateUpdate(int* pattern, int len, vector<bool>* supporting, int &new_false_positives, int &new_covered) {
	int delta_false_positives, delta_covered;
	new_false_positives = new_covered = 0;
    for(int i=0; i<len; i++) {
    	vfile[pattern[i]].simulateCover(supporting, delta_false_positives, delta_covered);
        new_false_positives += delta_false_positives;
        new_covered += delta_covered;
    }
  }

  void findDenseRegion( int* itemset, int &len, int &supp, 
		        list<int> *residual, int strategy, float Temp=0.0);
  bool addItemIfUseful( int next_item, set<int>* &tlist, 
			int* itemset, int &len, int &supp, float &curr_cost);
	void build_charms(int* charm_supports);

  static const int BATCH_SIZE = 50;
  FPtree* buildFPtree(int strategy, int* fp_map);

protected:
  void firstScan(char* input_file, vector<int> &supports);
  void secondScan(vector<int> &supports);

private:
  int n_items;
  int n_tr;
  int max_tr_len;
  int size;
  int *v_unmap, *v_map;  // from new (second scan=vertical) to original (horiz dataset)
  char* _temp_binary_file;
  VDataEntry* vfile;
  cost_function_type* delta_cost_function;
  Logger* _logger;

  // noise related measures
  int false_positives;
  int false_negatives;
  int covered_area;


private: //dummy
  float extendItemset_1_00(int* itemset, int &len, int &supp, list<int>* residual, vector<bool>* supporting);

};

#endif
