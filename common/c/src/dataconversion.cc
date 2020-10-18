/*
  Copyright (C) 2011, Claudio Lucchese

  This file is part of DMT.

  TOPK is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  TOPK is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DMT. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../inc/dataconversion.hh"
#include "../inc/iohandler.hh"
#include "../inc/basicstruct.hh"

#include <iostream>
#include <cmath>
#include <set>


void DMT_wt2t(char* input_file, char* output_file) {
	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_WEIGHTED_READER);
	IO_Handler* data_writer = IO_Handler::get_handler(output_file, IO_Handler::TR_ASCII_WRITER);

	DMT_Weighted_Transaction t(4024);

	while (!data_reader->eof()) {
		data_reader->read(&t);
		data_writer->write(&t);
	}

	delete data_reader;
	delete data_writer;
}

void DMT_lwt2wt(char* input_file, char* output_file, char* classes_file) {
	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_LABELED_WEIGHTED_READER);
	IO_Handler* data_writer = IO_Handler::get_handler(output_file, IO_Handler::TR_ASCII_WEIGHTED_WRITER);
	DMT_Labeled_Weighted_Transaction t(4024);

	IO_Handler* class_writer = NULL;
	if (classes_file)
		class_writer = IO_Handler::get_handler(classes_file, IO_Handler::TR_ASCII_LABELED_WRITER);
	DMT_Labeled_Transaction c(10);
	c.reset();

	while (!data_reader->eof()) {
		data_reader->read(&t);
		data_writer->write(&t);
		if (classes_file) {
			c.setClass(t.getClass());
			class_writer->write(&c);
		}
	}

	delete data_reader;
	delete data_writer;

	if (classes_file)
		delete class_writer;
}

void DMT_lwt2t(char* input_file, char* output_file, char* classes_file) {
	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_LABELED_WEIGHTED_READER);
	IO_Handler* data_writer = IO_Handler::get_handler(output_file, IO_Handler::TR_ASCII_WRITER);
	DMT_Labeled_Weighted_Transaction t(4024);

	IO_Handler* class_writer = NULL;
	if (classes_file)
		class_writer = IO_Handler::get_handler(classes_file, IO_Handler::TR_ASCII_LABELED_WRITER);
	DMT_Labeled_Transaction c(10);
	c.reset();

	while (!data_reader->eof()) {
		data_reader->read(&t);
		data_writer->write(&t);
		if (classes_file) {
			c.setClass(t.getClass());
			class_writer->write(&c);
		}
	}

	delete data_reader;
	delete data_writer;

	if (classes_file)
		delete class_writer;
}

int DMT_lwt_countLabels(char* input_file) {
	std::set<int> labels;

	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_LABELED_WEIGHTED_READER);
	DMT_Labeled_Weighted_Transaction t(4024);

	while (!data_reader->eof()) {
		data_reader->read(&t);
		labels.insert(t.getClass());
	}
	delete data_reader;

	return labels.size();
}

void DMT_t2lt(char* input_file, char* output_file) {
	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_READER);
	IO_Handler* data_writer = IO_Handler::get_handler(output_file, IO_Handler::TR_ASCII_LABELED_WRITER);
	DMT_Labeled_Transaction t(4024);

	while (!data_reader->eof()) {
		data_reader->read(&t);
		t.setClass(0);
		data_writer->write(&t);
	}

	delete data_reader;
	delete data_writer;
}

void DMT_t2sparsedbp(char* input_file, char* output_file) {
	// first count transactions, max_item and min_item
	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_READER);
	DMT_Labeled_Transaction t(4024);

	int num_tr = 0;
	int max_item = 0;
	while (!data_reader->eof()) {
		data_reader->read(&t);
		num_tr++;
		for (int i=0; i<t.getLen(); i++) {
			int item = t.getItem(i);
			if (item>max_item) max_item = item;
		}
	}

	// now start the remapping
	data_reader->rew();
	t.reset();
	IO_Handler* data_writer = IO_Handler::get_handler(output_file, IO_Handler::TR_ASCII_LABELED_WRITER);

	// write num_tr
	t.setClass(num_tr);
	data_writer->write(&t);
	// write max_item
	t.setClass(max_item+1); // make sure 0 is mapped to 1, and make easy the unmapping
	data_writer->write(&t);

	while (!data_reader->eof()) {
		data_reader->read(&t);
		for (int i=0; i<t.getLen(); i++) {
			int item = t.getItem(i)+1;
			t.setItem(i, item);
		}
		t.setClass(t.getLen()); // use the class to output the length of the pattern
		data_writer->write(&t);
	}

	delete data_reader;
	delete data_writer;
}

int DMT_dbp2extsuppset(char* item_file, char* tran_file, char* output_file) throw (DMT_Error*) {
	IO_Handler* item_reader = IO_Handler::get_handler(item_file, IO_Handler::TR_ASCII_READER);
	IO_Handler* tran_reader = IO_Handler::get_handler(tran_file, IO_Handler::TR_ASCII_READER);
	IO_Handler* pattern_writer = IO_Handler::get_handler(output_file, IO_Handler::XSET_ASCII_WRITER);

	int num_patterns=0, num_items=0, num_tr=0;
	DMT_Transaction t(255);
	// check num patterns
	item_reader->read(&t);
	num_patterns = t.getItem(0);
	tran_reader->read(&t);
	if (t.getItem(0) != num_patterns) throw new DMT_Error();
	// get num items
	item_reader->read(&t);
	num_items = t.getItem(0);
	// get num transactions
	tran_reader->read(&t);
	num_tr = t.getItem(0);

	DMT_Transaction* items       = new DMT_Transaction(num_items);
	DMT_Transaction* occurrences = new DMT_Transaction(num_tr);
	DMT_ExtSuppSet* pattern = new DMT_ExtSuppSet(num_items, num_tr);

	int asso_patterns = 0;

	while (!item_reader->eof() && !tran_reader->eof()) {
		item_reader->read(items);
		tran_reader->read(occurrences);
		pattern->reset();
		int supp = 0;
		for (int i=0; i<items->getLen(); i++)
			if (items->getItem(i)==1)
				pattern->append(i);
		for (int i=0; i<occurrences->getLen(); i++)
			if (occurrences->getItem(i)==1) {
				pattern->appendTransaction(i);
				supp++;
			}
		pattern->setSupport(supp);
		if (pattern->getLen()>0) {
			pattern_writer->write(pattern);
			asso_patterns++;
		}
	}


	delete items;
	delete occurrences;
	delete pattern;

	delete item_reader;
	delete tran_reader;
	delete pattern_writer;

	return asso_patterns;
}

void DMT_checklenghts(char* a, char* b) {
	IO_Handler* a_reader = IO_Handler::get_handler(a, IO_Handler::TR_ASCII_LABELED_WEIGHTED_READER);
	IO_Handler* b_reader = IO_Handler::get_handler(b, IO_Handler::TR_ASCII_WEIGHTED_READER);
	DMT_Labeled_Weighted_Transaction ta(4024);
	DMT_Weighted_Transaction tb(4024);

	std::cout << "CHECKING" << std::endl;

	while (!a_reader->eof()) {
		a_reader->read(&ta);
		b_reader->read(&tb);
		if (ta.getLen()!=tb.getLen())
			std::cout << "ERROR at line" << ta.getTid() << std::endl;
		for (int i=0; i<ta.getLen(); i++) {
			if (ta.getItem(i)!=tb.getItem(i))
				std::cout << "ITEM ERROR at line" << ta.getTid() << std::endl;
			if (fabs(ta.getWeight(i)-tb.getWeight(i))>0.001)
				std::cout << "WEIGHT ERROR at line" << ta.getTid() << std::endl;
		}
	}

	delete a_reader;
	delete b_reader;
}

void DMT_lt2lwt(char* input_file, char* output_file) {
	IO_Handler* data_reader = IO_Handler::get_handler(input_file, IO_Handler::TR_ASCII_LABELED_READER);
	IO_Handler* data_writer = IO_Handler::get_handler(output_file, IO_Handler::TR_ASCII_LABELED_WEIGHTED_WRITER);
	DMT_Labeled_Transaction in(4024);
	DMT_Labeled_Weighted_Transaction out(4024);

	while (!data_reader->eof()) {
		data_reader->read(&in);
		out.reset();
		out.setClass(in.getClass());
		for (int i=0; i<in.getLen(); i++) {
			out.append(in.getItem(i),1.0);
		}
		data_writer->write(&out);
	}

	delete data_reader;
	delete data_writer;
}
