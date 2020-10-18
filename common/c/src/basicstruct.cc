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

#include <stdio.h>
#include <iostream>
#include "../inc/basicstruct.hh"
using namespace std;

DMT_Error::DMT_Error() {}
DMT_Error::~DMT_Error() {}


DMT_Set::DMT_Set(int bufsize) {
	_bufsize = bufsize;
	_items   = new int [_bufsize];
	_len     = 0;
}

DMT_Set::~DMT_Set(){
	if(_items)	delete [] _items;
}


DMT_SuppSet::DMT_SuppSet(int bufsize) : DMT_Set(bufsize) {
	_support 	= -1;
}

DMT_SuppSet::~DMT_SuppSet() {
}


DMT_ExtSuppSet::DMT_ExtSuppSet(int bufsize, int trbufsize) : DMT_SuppSet(bufsize) {
	_trbufsize = trbufsize;
	_transactions = new int [_trbufsize];
	_trlen = 0;
}

DMT_ExtSuppSet::~DMT_ExtSuppSet(){
	if(_transactions)	delete [] _transactions;
}

void DMT_ExtSuppSet::setSupport(int val) {
	DMT_SuppSet::setSupport(val);
	if (this->getSupport()>_trlen) {
		if(_transactions)	delete [] _transactions;
		_trbufsize = this->getSupport();
		_transactions = new int [_trbufsize];
	}
}

DMT_Rule::DMT_Rule(DMT_Set* head, DMT_Set* tail) {
	_head = head;
	_tail = tail;
	_supp = 0.0f;
	_conf = 0.0f;
}

DMT_Rule::~DMT_Rule() {
}

// ~~~~~~~~~~~~~~~~~~~ Transactions ~~~~~~~~~~~~~~~~~~~
DMT_Transaction::DMT_Transaction(int bufsize)
	: DMT_Set(bufsize) {
}

DMT_Transaction::~DMT_Transaction() {
}

DMT_Labeled_Transaction::DMT_Labeled_Transaction(int bufsize)
	: DMT_Transaction(bufsize) {
	_class   = -1;
}

DMT_Labeled_Transaction::~DMT_Labeled_Transaction() {
}

DMT_Weighted_Transaction::DMT_Weighted_Transaction(int bufsize)
	: DMT_Transaction(bufsize) {
	_weights = new float [bufsize];
}

DMT_Weighted_Transaction::~DMT_Weighted_Transaction() {
	if (_weights) delete [] _weights;
}

DMT_Labeled_Weighted_Transaction::DMT_Labeled_Weighted_Transaction(int bufsize)
	: DMT_Transaction(bufsize), DMT_Labeled_Transaction(bufsize), DMT_Weighted_Transaction(bufsize)  {
}

DMT_Labeled_Weighted_Transaction::~DMT_Labeled_Weighted_Transaction() {
}
