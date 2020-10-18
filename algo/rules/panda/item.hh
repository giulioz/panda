#ifndef __ITEM_HH_
#define __ITEM_HH_

/*----------------------------------------------------------------------
  File    : item.h
  Contents: itemset management
  Author  : Bart Goethals
  Update  : 4/4/2003
  ----------------------------------------------------------------------*/

#include <set>

using namespace std;

class Item;

class Item_
{
 public:
	
  Item_();
  ~Item_();
	
  int id;
  int supp;
	
  set<Item> *children;
	
  Item *parent;
  Item *nodelink;
  int flag; // used to avoid visiting twice the same node
};

class Item
{
 public:
	
  Item(int s, Item *p);
  Item(const Item& i);
  ~Item();
	
  int getId() const {return item->id;}  
  int getSupport() const {return item->supp;}
	
  set<Item> *getChildren() const {return item->children;}
  set<Item> *makeChildren() const;

  Item* getParent() const {return item->parent;}
  Item* getNext() const {return item->nodelink;}
  void setNext(Item *i) const {item->nodelink = i;}
  void Increment(int i=1) const {item->supp += i;}
	
  void removeChildren() const;
	
  void setFlag(int i) const {item->flag = i;}
  int  getFlag() const {return item->flag;}

  bool operator< (const Item &i) const {return getId() < i.getId();}

 private:
	
  Item_ *item;
};

#endif
