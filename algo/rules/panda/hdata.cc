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
using namespace std;	

#include "utils.hh"
#include "hdata.hh"



/*
 * Prints on screen the content of the binary file
 */
void HData::pprint() {
  FILE* f = fopen (HDATA_TEMP_FILE, "rb");
  if (!f) fatal_error("Could not temp output file.");

  int tid = 0;
  int len = 0;
  int* itemset = new int[HDATA_MAX_TR_SIZE];

  while (!feof(f)) {
    int read = fread(&tid, sizeof(int), 1, f);
    if (read!=1) continue;
    fread(&len, sizeof(int), 1, f);
    fread(itemset, sizeof(int), len, f);

#ifdef _V_DEBUG_
    cout << tid << " : ";
    cout << len << " :";
    for (int i=0; i<len; i++)
      cout << " " << itemset[i];
    cout << endl;
#endif
  }

  delete [] itemset;

  fclose(f);
}


/*
 * Creates a binary version of the dataset on disk
 */
HData::HData(char* input_file) {
  res_items = res_transactions = res_size = 0;
  map = unmap = NULL;

  firstScan(input_file);
  secondScan();
}

/*
 * Destructor
 */
HData::~HData() {
  if (map) delete [] map;
  if (unmap) delete [] unmap;
  unlink(HDATA_TEMP_FILE);
}
/*
 * Executes the first scan of the ascii file
 * and collects support statistics.
 */
void HData::firstScan(char* input_file) {
  supports.reserve(1024);

  FILE* fi = fopen ( input_file, "r");
  if (!fi) fatal_error("Could not open input file.");
  
  FILE* fo = fopen (HDATA_TEMP_FILE, "wb");
  if (!fo) fatal_error("Could not temp output file.");

  int* itemset = new int[HDATA_MAX_TR_SIZE];

  while (!feof(fi)) {
    int len = readItemset(fi, itemset);
    if (len>0) {
      if (len > HDATA_MAX_TR_SIZE) fatal_error("HDATA_MAX_TR_SIZE is too small");

      // update singletons supports
      for (int i=0; i<len; i++) {
	if (itemset[i] >= (int)supports.size()) {
	  supports.resize(itemset[i]+1,0);
	}
	supports[ itemset[i] ]++;
      }

      // rewrite in binary format
      fwrite(&res_transactions, sizeof(int), 1, fo);
      fwrite(&len, sizeof(int), 1, fo);
      fwrite(itemset, sizeof(int), len, fo);

      res_size += len;
      res_transactions++;
    }
  }

  res_items = supports.size();

  delete [] itemset;

  fclose(fi);
  fclose(fo);
}

/*
 * Remaps items from most frequent to least frequent
 * and removes from disk zero supports.
 */
void HData::secondScan() {
  // create map and unmapping functions
  unmap = new int[supports.size()];
  if (!unmap) fatal_error("Could not create unmapping vector.");
  map = new int[supports.size()];
  if (!map) fatal_error("Could not create mapping vector.");

  for (unsigned int i=0; i<supports.size(); i++)
    unmap[i]=i;

  indexQuickSort(0, (int)supports.size()-1, unmap, supports.begin(), false);

  for (unsigned int i=0; i<supports.size(); i++)
    map[unmap[i]] = i;

  // remove zero supports from res_items
  res_items = 0;
  while( res_items < (int)supports.size() && supports[unmap[res_items]]>0 )
    res_items++;

  // remap the binary file
  FILE* fi = fopen (HDATA_TEMP_FILE, "rb");
  if (!fi) fatal_error("Could not open input file.");
  
  FILE* fo = fopen (HDATA_TEMP_FILE, "r+b");
  if (!fo) fatal_error("Could not temp output file.");

  rewind(fi);
  rewind(fo);

  int tid = 0;
  int len = 0;
  int* itemset = new int[HDATA_MAX_TR_SIZE];

  while (!feof(fi)) {
    int read = fread(&tid, sizeof(int), 1, fi);
    if (read!=1) continue;
    fread(&len, sizeof(int), 1, fi);
    fread(itemset, sizeof(int), len, fi);

    for (int i=0; i<len; i++)
      itemset[i] = map[itemset[i]];

    fwrite(&tid, sizeof(int), 1, fo);
    fwrite(&len, sizeof(int), 1, fo);
    fwrite(itemset, sizeof(int), len, fo);
  }

  delete [] itemset;


  if ( ftruncate (fileno(fo), ftell(fo)) )
    fatal_error("Could not truncate temp output file.");

  fclose(fi);
  fclose(fo);
}


/*
 * Creates a vertical representation starting from the binary data
 * the vertical representation must already be initialized properly.
 */

/*
 * creates an fp-tree containing the residual items only
 * map is used to created a new remapped fptree
 * -- if map[i]=-1 that i is removed from the transaction
 * NB. HData should be re-writte each time ?
 */

FPtree* HData::buildFPtree(int* fp_map){
  FPtree* fptree = new FPtree(VData::MAXIMUM_LENGTH);

  FILE* f = fopen (HDATA_TEMP_FILE, "rb");
  if (!f) fatal_error("Could not temp output file.");

  int tid = 0;
  int len = 0;
  int* itemset = new int[HDATA_MAX_TR_SIZE];
  int* outset;

  while (!feof(f)) {
    int read = fread(&tid, sizeof(int), 1, f);
    if (read!=1) continue;
    fread(&len, sizeof(int), 1, f);
    fread(itemset, sizeof(int), len, f);

    // remap items
    if (fp_map!=0)
      for (int i=0; i<len; i++)
	itemset[i] = fp_map[itemset[i]];
    // reorder (remapping should be done relative to new supps)
    quickSort(0, len-1, itemset);

    // remove -1 items
    outset = itemset;
    while (outset-itemset<len && *outset==-1 ) 
      outset++;
    len -= outset-itemset;

    if (len>0) {
      fptree->processItems(outset, len);
      fptree->processTransaction(outset, len);
    }
  }

  delete [] itemset;
  fclose(f);

  return fptree;
}
