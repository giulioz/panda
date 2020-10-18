/*
  Copyright (C) 2010, Claudio Lucchese

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
*/

#ifndef __DMT_BASICSTRUCT_H
#define __DMT_BASICSTRUCT_H

#include <algorithm>

class DMT_Error {
public:
	DMT_Error();
	virtual ~DMT_Error();
};

class DMT_DataStructure {
public:
	DMT_DataStructure() {};
	virtual ~DMT_DataStructure() {};
};

class DMT_Set : public virtual DMT_DataStructure {
public:
	DMT_Set(int bufsize=255);
	virtual ~DMT_Set();

	inline virtual void reset()		{_len=0;};

	inline virtual int getLen()	{return _len;};

	inline virtual int  getItem(int i) 			{return _items[i];};
	inline virtual void setItem(int i, int val) {_items[i]=val;};

	inline virtual void append(int i) throw (DMT_Error*) {
		if (_len<_bufsize) {
			_items[_len++]=i;
		} else throw new DMT_Error();
	}

	inline virtual void sort() { std::sort(_items, _items+_len); }

private:
	int 	_len;
	int* 	_items;
	int     _bufsize;
};

/*
 * Association Rule
 *   made out of two sets
 */

class DMT_Rule : public virtual DMT_DataStructure {
public:
	DMT_Rule(DMT_Set* head, DMT_Set* tail);
	virtual ~DMT_Rule();

	inline virtual void reset()	{ _supp=0.0f; _conf=0.0f; _head->reset(); _tail->reset();}

	inline virtual DMT_Set* getHead() {return _head;}
	inline virtual DMT_Set* getTail() {return _tail;}

	inline virtual void  setSupport(float s) {_supp = s;}
	inline virtual float getSupport()        {return _supp;}

	inline virtual void  setConfidence(float c) {_conf = c;}
	inline virtual float getConfidence()        {return _conf;}

	inline virtual void sort() { _head->sort(); _tail->sort(); }

private:
	DMT_Set* _head;
	DMT_Set* _tail;
	float _supp;
	float _conf;
};


/*
 * Format:
 * 1 4 6 8 (3234)
 * First sequence is the itemset
 * between () the support (absolute)
 */
class DMT_SuppSet : public virtual DMT_Set {
public:
	DMT_SuppSet(int bufsize=255);
	virtual ~DMT_SuppSet();

	inline virtual int  getSupport() 		{return _support;};
	inline virtual void setSupport(int val) {_support=val;};

	inline virtual void reset()			{DMT_Set::reset(); _support=-1;};

private:
	int		_support;
};

/*
 * Format:
 * 1 4 6 8 (3234) [12 23 445 65]
 * First sequence is the itemset
 * between () the support (absolute)
 * between [] the list of transactions supporting the itemset
 *            this is optional
 */


class DMT_ExtSuppSet : public virtual DMT_SuppSet{
public:
	DMT_ExtSuppSet(int bufsize=255, int trbufsize=10240);
	virtual ~DMT_ExtSuppSet();

	inline virtual void reset()			{DMT_SuppSet::reset(); _trlen=0;};

	virtual void setSupport(int val);


	inline virtual int  getTransaction(int i) 			{return _transactions[i];};
	inline virtual void setTransaction(int i, int val) {_transactions[i]=val;};
	inline virtual int getTransactionsLen()	{return _trlen;};
	inline virtual void appendTransaction(int i) throw (DMT_Error*) {
		if (_trlen<_trbufsize) {
			_transactions[_trlen++]=i;
		} else throw new DMT_Error();
	}

private:
	int* 	_transactions;
	int		_trbufsize;
	int		_trlen;
};


/**
 * This is a DMT_Set plus a transaction ID,
 * which must be assigned by the data reader.
 */
class DMT_Transaction : public DMT_Set {
public:
	DMT_Transaction(int bufsize=255);
	virtual ~DMT_Transaction();

	inline virtual void reset()			{DMT_Set::reset(); _tid=-1;};

	inline virtual int  getTid()		{return _tid;};
	inline virtual void setTid(int tid)	{_tid = tid;};

private:
	int		_tid;
};

/*
 * Format:
 * 1 4 6 8
 * First number is a positive integer identifying the class ID
 * Second to n-th are items, non overlapping with class IDs
 */
class DMT_Labeled_Transaction : public virtual DMT_Transaction {
public:
	DMT_Labeled_Transaction(int bufsize=255);
	virtual ~DMT_Labeled_Transaction();

	inline virtual void reset()			{DMT_Transaction::reset(); _class=-1;};

	inline virtual int  getClass()		{return _class;};
	inline virtual void setClass(int c)	{_class = c;};

private:
	int		_class;
};

/** Transaction with feature weights.
 */
class DMT_Weighted_Transaction : public virtual DMT_Transaction {
public:
	DMT_Weighted_Transaction(int bufsize=255);
	virtual ~DMT_Weighted_Transaction();

	inline virtual void reset()			{DMT_Transaction::reset();};

	inline virtual float getWeight(int i) 			 {return _weights[i];};
	inline virtual void  setWeight(int i, float w)   {_weights[i]=w;};

	inline virtual void append(int i, float w) throw (DMT_Error*) {
		DMT_Transaction::append(i);
		_weights[getLen()-1] = w;
	}

private:
	float* 	_weights;
};

/** Transaction with class label and feature weights.
 */
class DMT_Labeled_Weighted_Transaction : public DMT_Labeled_Transaction, public DMT_Weighted_Transaction {
public:
	DMT_Labeled_Weighted_Transaction(int bufsize=255);
	virtual ~DMT_Labeled_Weighted_Transaction();

	inline virtual void reset()	{ DMT_Labeled_Transaction::reset(); DMT_Weighted_Transaction::reset();};
};

#endif /* __DMT_BASICSTRUCT_HH_ */
