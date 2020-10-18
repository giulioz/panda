/*----------------------------------------------------------------------
  File    : fptree.cpp
  Contents: fpgrowth algorithm for finding frequent sets
  Author  : Bart Goethals
  Update  : 08/04/2003 - single prefix path bug fixed (Thanks to Xiaonan Wang)
  Update  : 6/16/2009 - reduced to topk needs
  ----------------------------------------------------------------------*/

#include <iostream>
#include <stdio.h>
#include <set>
#include <float.h>
using namespace std;
#include "fptree.hh"
#include "item.hh"
#include "utils.hh"

FPtree::FPtree(int s, int maximum_len, cost_function_type *custom_cost_function)
{
	root = new set<Item>;
	nodes = 0;
	n_tr = 0;
	// opID = 0;
	max_len = maximum_len;
	strategy = s;
	cost_function = custom_cost_function;
}

FPtree::~FPtree()
{
	set<Item>::iterator it;

	for(it = root->begin(); it != root->end(); it++)
		it->removeChildren();
	delete root;
}

// adds to the header
int FPtree::processItems(int* itemset, int len, int times)
{
	set<Item>::iterator head;
	int added=0;

	for(int depth=0; depth < len; depth++) {
		head = header.find(Item(itemset[depth], 0));
		if(head == header.end()) {
			head = header.insert(Item(itemset[depth], 0)).first;
			added++;
		}
		head->Increment(times);
	}
	return added;
}

// adds to the fptree
void FPtree::processTransaction(int* itemset, int len, int times)
{
	set<Item>::iterator it, head;
	set<Item>* items = root;
	Item *current = 0;

	for(int depth=0; depth < len; depth++) {
		head = header.find(Item(itemset[depth], 0));
		if(head != header.end()) {
			it = items->find(Item(itemset[depth], 0));

			if(it == items->end()) {
				it = items->insert(Item(itemset[depth], current)).first;
				it->setNext(head->getNext());
				head->setNext( (Item*)&(*it) );
				nodes++;
			}

			it->Increment(times);
			current = (Item*)&(*it);
			items = it->makeChildren();
		}
	}
	n_tr++;
}


void FPtree::pprint() {
	cout << "header=";
	for (set<Item>::iterator i=header.begin(); i!=header.end(); i++)
		cout << " " << i->getId() << ":" << i->getSupport() << ",";
	cout << endl;
	cout << "transactions=" << endl;

	int* itemset = new int[header.size()];
	for(set<Item>::iterator i=root->begin(); i!=root->end(); i++)
		pprint(&(*i), itemset, 0);
	delete [] itemset;
}

void FPtree::pprint(const Item* node, int* itemset, int len) {
	// this node
	itemset[len++] = node->getId();

	// recurse children
	int chsupp = 0;
	set<Item>* ch = node->getChildren();
	if (ch!=NULL) {
		for(set<Item>::iterator i=ch->begin(); i!=ch->end(); i++) {
			pprint(&(*i), itemset, len);
			chsupp += i->getSupport();
		}
	}

	// print node
	int supp = node->getSupport() - chsupp;
	if (supp>0) {
		for (int i=0; i<len; i++)
			cout << itemset[i] << " ";
		cout << "(" << supp << ")" << endl;
	}
}

void FPtree::charmSupps(const Item* node, int* counts, int* itemset, int len) {
	// this node
	itemset[len++] = node->getId();

	// recurse children
	int chsupp = 0;
	set<Item>* ch = node->getChildren();
	if (ch!=NULL) {
		for(set<Item>::iterator i=ch->begin(); i!=ch->end(); i++) {
			charmSupps(&(*i), counts, itemset, len);
			chsupp += i->getSupport();
		}
	}

	int supp = node->getSupport() - chsupp;
	if (supp>0) {
		for (int i=0; i<len-1; i++) {
			for (int j=i+1; j<len; j++) {
				counts[itemset[i]]+=supp;
				counts[itemset[j]]+=supp;
			}
		}
	}
}

void FPtree::resetFlags(const Item* node) {
	node->setFlag(0);
	set<Item>* ch = node->getChildren();
	if (ch!=NULL) {
		for(set<Item>::iterator i=ch->begin(); i!=ch->end(); i++) {
			resetFlags( &(*i) );
		}
	}
}


/*
 * adds an item to the current dense region
 * and updates the dense region only if the new item
 * brings some cost reduction
 */
bool FPtree::addItemIfUseful( int next_item, list<Item*>* &tlist, 
		int* itemset, int &len, int &supp, float &curr_cost) {
	bool inserted = false;
	list<Item*>* nlist = new list<Item*>(); // candidate new list
	int nsupp =  addItemToDenseRegion(tlist, next_item, nlist);
	if (nsupp>0) {
		float new_cost = cost_function(len+1, nsupp, 0, 0);
		// cout << "# ## ## Testing: " << next_item << " : " << nsupp << " @ " << new_cost << endl;

		if (new_cost <= curr_cost) {
#ifdef _V_DEBUG_
			cout << "# ## ## FP added: " << next_item << " : " << (len+1) << "x" << nsupp << " @ " << new_cost << endl;
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

/*
 * Finds a dense error free region.
 */

void FPtree::findDenseRegion( int* itemset, int &len, int &supp, 
		list<int> *residual, float Temp) {
	len = 0;
	float    curr_cost = FLT_MAX;
	list<Item*>* tlist = new list<Item*>();
	residual->clear();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// header (frequency) order
	if (strategy==FREQUENCY_ORDER) {
		int* counts = NULL;
		int* indices = NULL;
		if (Temp>0) {
			// create a randomized list of the header given a temperature T
			counts  = new int [header.size()];
			indices = new int [header.size()];
			int j=0;
			for(set<Item>::iterator i=header.begin(); len<max_len && i!=header.end(); i++) {
				counts[j] = i->getSupport();
				indices[j] = j;
				j++;
			}
			indexQuickSortRandom(0, header.size()-1, indices, counts, Temp, false);
		}

		// try addin item one by one
		for(unsigned int i=0; len<max_len && i<header.size(); i++) {
			int next_item;
			if (Temp>0) next_item = indices[i];
			else        next_item = (int) i;

			if ( !addItemIfUseful( next_item, tlist,
					itemset, len, supp, curr_cost) )
				residual->push_back(next_item);
		}

		if (Temp>0) {
			delete [] counts;
			delete [] indices;
		}
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// next is the most frequent children
	else if (strategy==CHILD_FREQUENCY_ORDER) {
		int next_item;
		bool added;
		// create counters
		int* counts  = new int [header.size()];
		int* indices = new int [header.size()];

		if (Temp>0) {
			// create a randomized list of the header given a temperature T
			int j=0;
			for(set<Item>::iterator i=header.begin(); len<max_len && i!=header.end(); i++) {
				counts[j]  = i->getSupport();
				indices[j] = j;
				j++;
			}
			indexQuickSortRandom(0, header.size()-1, indices, counts, Temp, false);
			next_item = indices[0];
		}
		else
			next_item = 0;

		// next_item = header.begin()->getId();
		added = addItemIfUseful( next_item, tlist,
				itemset, len, supp, curr_cost);


		while (added && len<max_len) { // should always get in here !
			// init
			added = false;
			for (unsigned int i = 0; i<header.size(); i++) {
				counts[i] = 0;
				indices[i] = i;
			}

			// count supps downward
			for (list<Item*>::iterator node = tlist->begin(); node!=tlist->end(); node++) {
				set<Item>* ch = (*node)->getChildren();
				if (ch!=NULL)
					for (set<Item>::iterator i=ch->begin(); i!=ch->end(); i++)
						counts[i->getId()]+= i->getSupport();
			}

			// sort by frequency
			if (Temp>0)
				indexQuickSortRandom(0, header.size()-1, indices, counts, Temp, false);
			else
				indexQuickSort(0, header.size()-1, indices, counts, false);

			// take the first that fits
			for (unsigned int i=0; !added && i<header.size() && counts[indices[i]]>0; i++) {
				next_item = indices[i];
				// cout << "# ## ## trying to add item " << next_item <<  "(" << header.size()-1<<")"<<endl;
				added = addItemIfUseful( next_item, tlist,
						itemset, len, supp, curr_cost);
				// if (added)
				//   cout << "# ## ## ADDED item " << next_item <<  "(" << header.size()-1<<")"<<endl;
			}
		}
		// ------------------------
		// create residuals
		if (curr_cost<0) {
			// count supps downward
			for (list<Item*>::iterator node = tlist->begin(); node!=tlist->end(); node++) {
				set<Item>* ch = (*node)->getChildren();
				if (ch!=NULL)
					for (set<Item>::iterator i=ch->begin(); i!=ch->end(); i++)
						counts[i->getId()]+= i->getSupport();
			}
			// sort by frequency
			indexQuickSort(0, header.size()-1, indices, counts, false);
			// parse
			for (unsigned int i=0; i<header.size() && counts[indices[i]]>0; i++)
				residual->push_back(indices[i]);
		}

		delete [] counts;
		delete [] indices;
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// itmes are sorted with a charm-like order
	else if (strategy==CHARM_ORDER) {
		// create counters
		int* counts  = new int [header.size()];
		int* indices = new int [header.size()];
		int* aux_itemset = new int [header.size()];

		for (unsigned int i = 0; i<header.size(); i++) {
			counts[i] = 0;
			indices[i] = i;
		}

		for(set<Item>::iterator i=root->begin(); i!=root->end(); i++) {
			// counts[i->getId()] += i->getSupport();
			charmSupps(&(*i), counts, aux_itemset, 0);
		}

		// sort by frequency
		if (Temp>0)
			indexQuickSortRandom(0, header.size()-1, indices, counts, Temp, false);
		else
			indexQuickSort(0, header.size()-1, indices, counts, false);

		// tary adding item one by one
		for (unsigned int i=0; i<header.size() && counts[indices[i]]>0; i++) {
			int next_item = indices[i];
			// cout << "# ## ## trying to add item " << next_item <<  "(" << counts[next_item]<<")"<<endl;
			bool added = addItemIfUseful( next_item, tlist, itemset, len, supp, curr_cost);
			if (!added) residual->push_back(next_item);

			// if (added)
			// cout << "# ## ## ADDED item " << next_item <<  "(" << header.size()-1<<")"<<endl;
		}

		delete [] aux_itemset;
		delete [] counts;
		delete [] indices;
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// next is the most correlated with
	// the current itemset
	else if (strategy==MOST_CORRELATED_ORDER) {
		// take the first randomly
		int next_item;
		int* counts  = new int [header.size()];
		int* indices = new int [header.size()];

		if (Temp>0) {
			// create a randomized list of the header given a temperature T
			int j=0;
			for(set<Item>::iterator i=header.begin(); len<max_len && i!=header.end(); i++) {
				counts[j]  = i->getSupport();
				indices[j] = j;
				j++;
			}
			indexQuickSortRandom(0, header.size()-1, indices, counts, Temp, false);
			next_item = indices[0];
		}
		else
			next_item = 0;

		delete [] counts;
		delete [] indices;

		// the first item should always be good !
		bool added = addItemIfUseful( next_item, tlist,
				itemset, len, supp, curr_cost);

		while (added && len<max_len) { // should always get in here !
			// count supports
			added = false;
			list<int> _residual;
			countSupps(itemset, len, tlist, &_residual, Temp);

			// take the first that fits
			for (list<int>::iterator it=_residual.begin(); !added && it!=_residual.end(); it++) {
				next_item = *it;
				// cout << "# ## ## trying to add item " << next_item <<  "(" << header.size()-1<<")"<<endl;
				added = addItemIfUseful( next_item, tlist,
						itemset, len, supp, curr_cost);
				// if (added)
				//   cout << "# ## ## ADDED item " << next_item <<  "(" << header.size()-1<<")"<<endl;
			}
		}
		if (curr_cost<0)
			countSupps(itemset, len, tlist, residual);
	}

	tlist->clear(); delete tlist;

	if (curr_cost>=0) // no good rectangle was found
		len = supp = 0;
}

/*
 * Count the supps of all the other items occurring in transactions tlist
 * the result is stored in the residual list
 */
void FPtree::countSupps(int* itemset, int len, list<Item*>* tlist, list<int>* residual, float Temp) {
	// reset counts
	int* counts = new int [header.size()];
	int* indices = new int[header.size()];
	for (unsigned int i = 0; i<header.size(); i++) {
		counts[i] = 0;
		indices[i] = i;
	}

	// count supps downward
	for (list<Item*>::iterator node = tlist->begin(); node!=tlist->end(); node++) {
		list<Item*> cands; // find all descendants starting from node
		cands.push_back(*node);
		while(cands.size()>0) {
			Item* curr = cands.front();
			cands.pop_front();
			counts[curr->getId()] += curr->getSupport();
			set<Item>* ch = curr->getChildren();
			if (ch!=NULL)
				for (set<Item>::iterator i=ch->begin(); i!=ch->end(); i++)
					cands.push_back( (Item*) &(*i) );
		}
	}
	// count supps upward
	/** int OP_ID = getNextOpID(); **/
	for (list<Item*>::iterator node = tlist->begin(); node!=tlist->end(); node++) {
		int tsupp = (*node)->getSupport();
		Item* curr = (*node)->getParent();
		while(curr!=0 /** && curr->getFlag()!=OP_ID **/ ) {
			counts[curr->getId()] += tsupp;
			/** curr->setFlag(OP_ID); **/
			curr = curr->getParent();
		}
	}

	// count conditional supports of other items
	residual->clear();
	for (int i=0; i<len; i++)
		counts[itemset[i]] = 0; // reset items in current itemset
	indexQuickSort(0, header.size()-1, indices, counts, false);
	for (unsigned int i=0; counts[indices[i]]!=0 && i<header.size(); i++) {
#ifdef _V_DEBUG_
		cout << "residual " << indices[i] << " @ " << counts[indices[i]] << endl;
#endif
		residual->push_back(indices[i]);
	}

	delete [] indices;
	delete [] counts;
}

int FPtree::getNextOpID() {
	//   // create a new flag
	//   opID++;
	//   if (opID==0) {
	//     for(set<Item>::iterator i=root->begin(); i!=root->end(); i++)
	//       resetFlags(&(*i));
	//     opID++;
	//   }
	//   return opID;
	return -1;
}

/*
 * Adds a new item to the current candidate region.
 * :: returns the support of the new candidate
 * :: clist is the node-pointers list of the current dense region
 * :: item is the new item to be added
 * :: nlist is the node-pointers list of the new dense region
 */
int FPtree::addItemToDenseRegion(list<Item*> *clist, int item, list<Item*> *nlist) {
	int nsupp = 0;
	nlist->clear();

	// this is the first item
	if (clist->size()==0) {
		set<Item>::iterator head;
		head = header.find(Item(item, 0));
		if (head==header.end())
			return -1; // impossible to add item
		Item* curr = head->getNext();
		while (curr!=0) {
			nlist->push_back(curr);
			nsupp += curr->getSupport();
			curr = curr->getNext();
		}
		return nsupp;
	}

	// this is the n-th item
	int last_x = clist->front()->getId();
	if (item>last_x) {
#ifdef _V_DEBUG_
		cout << "# ## ## FP: downward item add" << endl;
#endif
		// search downward
		for (list<Item*>::iterator node = clist->begin(); node!=clist->end(); node++) {
			list<Item*> cands; // find all descendants starting from node
			cands.push_back(*node);
			while(cands.size()>0) {
				Item* curr = cands.front();
				cands.pop_front();
				if (curr->getId()==item) {
					nlist->push_back(curr);
					nsupp += curr->getSupport();
				} else if (curr->getId()<item) {
					set<Item>* ch = curr->getChildren();
					for (set<Item>::iterator i=ch->begin(); i!=ch->end(); i++)
						if(i->getId()<=item) cands.push_back( (Item*) &(*i) );
				}
			}
		}
	} else {
		// search upward
#ifdef _V_DEBUG_
		cout << "# ## ## FP: upward item add" << endl;
		cout << "# ## ## FP: clist size: " <<clist->size() << endl;
#endif
		// check also count supps and similar
		for (list<Item*>::iterator node = clist->begin(); node!=clist->end(); node++) {
			int tsupp = (*node)->getSupport();
			Item* curr = (*node)->getParent();
			while(curr!=0 && curr->getId()>item)
				curr = curr->getParent();
			if ( curr!=0 && curr->getId()==item) {
				nlist->push_back(*node);
				nsupp += tsupp;
#ifdef _V_DEBUG_
				cout << "# ## ## FP: adding node " << curr->getId() << "x" << tsupp << endl;
#endif
			}
		}
	}

	return nsupp;
}


