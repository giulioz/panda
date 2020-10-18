#ifndef __FP_TREE_HH
#define __FP_TREE_HH

/*----------------------------------------------------------------------
  File    : fptree.hh
  Contents: fpgrowth algorithm for finding frequent sets
  Author  : Bart Goethals
  Update  : 8/4/2003 - single prefix path bug fixed (Thanks to Xiaonan Wang)
  Update  : 6/16/2009 - reduced to topk needs (claudio lucchese)
  ----------------------------------------------------------------------*/

#include <set>
#include <list>
using namespace std;

#include "item.hh"
#include "costs.hh"

class FPtree {
public:
  FPtree(int s, int maximum_len=0x7fffff, 
	 cost_function_type *custom_cost_function = &PANDA_delta_cost);
  ~FPtree();
  
  void processTransaction(int* itemset, int len, int times=1);
  int processItems(int* itemset, int len, int times=1);
  
  void pprint();

  inline int getItems() const {return header.size();}
  inline int getTrans() const {return n_tr;}

  // itemset should be sufficiently large
  // max len could be enforced
  void findDenseRegion( int* itemset, int &len, int &supp, 
		        list<int> *residual, float Temp=0.0);

  static const int FREQUENCY_ORDER        = 0;
  static const int CHILD_FREQUENCY_ORDER  = 1;
  static const int MOST_CORRELATED_ORDER  = 2;
  static const int CHARM_ORDER            = 3;

private:
  set<Item> header;
  set<Item> *root;
  unsigned nodes;
  int n_tr;
  int max_len;
  int strategy;

  cost_function_type* cost_function;
  // float (*cost_function)(int, int, int, int);

  //int opID;

protected:
  int getNextOpID();
  void pprint(const Item* node, int* itemset, int len);

  // adds an item to the current dense region, return the support
  bool addItemIfUseful( int next_item, list<Item*>* &tlist, 
			int* itemset, int &len, int &supp, float &curr_cost);
  int addItemToDenseRegion(list<Item*> *clist, int item, list<Item*> *nlist);
  int getNextItem(list<Item*>* tlist, int strategy);
  void resetFlags(const Item* node);
  void countSupps(int* itemset, int len, list<Item*>* tlist, list<int>* residual, float Temp=0.0);
  void charmSupps(const Item* node, int* counts, int* itemset, int len);
};

#endif
