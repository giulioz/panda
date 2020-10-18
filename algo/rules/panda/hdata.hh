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

#ifndef __HDATA_H
#define __HDATA_H



#include "vdata.hh"
#include "fptree.hh"

#include <vector>

using namespace std;

class HData {
public:

  HData(char* input_file);           // starting from an ascii file
  ~HData();

  VData* buildVertical();              // create vertical representation
  FPtree* buildFPtree(int* fp_map=0);  // create residual fp-tree

  void pprint(); // prints on screen the binary data

  inline int getResItems() const { return res_items;}
  inline int getResTrans() const { return res_transactions;}
  inline int getResSize()  const { return res_size;}
  inline void unmapItemset(int* set, int len)  // from vertical to original mapping
  { for (int i=0; i<len; i++) set[i]=unmap[set[i]];  }

protected:
  void firstScan(char* input_file);
  void secondScan();

private:
  int res_items;         // items after second scan, i.e. items in the vertical DB
  int res_transactions;
  int res_size;

  vector<int> supports;
  int* map;    // from original (horiz dataset) to new (second scan vertical)
  int* unmap;  // from new (second scan=vertical) to original (horiz dataset)
};


#endif
