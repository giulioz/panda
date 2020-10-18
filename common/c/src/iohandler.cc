/*
  Copyright (C) 2010, Claudio Lucchese

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

#include "../inc/iohandler.hh"

#include <iostream>
#include <cstdio>
using namespace std;

bool IO_Handler::ascii_read_rule(DMT_Rule* data) {
	int c = 0;
  	data->reset();
  	bool ok = false;

	// Read Head
	while (!eof() && c!='\n' && c!='>') {
		c = fgetc(_f);  // eat all white-spaces

		// read the item
		if ((c >= '0') && (c <= '9')) {
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			data->getHead()->append(item);
		}
	}

	// Read Tail
	while (!eof() && c!='\n' && c!='(') {
		c = fgetc(_f);  // eat all white-spaces

		// read the item
		if ((c >= '0') && (c <= '9')) {
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			data->getTail()->append(item);
		}
	}

	// Read Supp
	while (!eof() && c!='\n' && c!=',') {
		c = fgetc(_f);  // eat all white-spaces

		// read the item
		if ((c >= '0') && (c <= '9')) {
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			data->setSupport((float)item);
		}
	}
	// Read Conf
	while (!eof() && c!='\n') {
		c = fgetc(_f);  // eat all white-spaces
		float value=0.0f;
		// read integer part
		if ((c >= '0') && (c <= '9')) {
			value = (float)(c-'0');
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				value *= 10;
				value += (float)(c-'0');
				c = fgetc(_f);
			}
		}
		if (c=='.') {
			// read fractional part
			c = fgetc(_f);
			float position = 0.1;
			while((c >= '0') && (c <= '9')) {
				value += (float)(c-'0')*position;
				position*=0.1;
				c = fgetc(_f);
			}
			data->setConfidence(value);
			ok = true;
		}
	}


	return ok;

}

bool IO_Handler::ascii_read_tr(DMT_Transaction* data) {
	int c = 0;
  	data->reset();
  	bool ok = false;

	// Read Items
	while (!eof() && c!='\n') {
		c = fgetc(_f);  // eat all white-spaces

		// read the item
		if ((c >= '0') && (c <= '9')) {
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			data->append(item); // add item
			ok = true;
		}
	}

	return ok;
}


bool IO_Handler::ascii_read_ltr(DMT_Labeled_Transaction* data) {
	int c = 0;
  	data->reset();
  	bool ok = false;

	// Read Class
	while (!eof() && c!='\n') {
		c = fgetc(_f);  // eat all white-spaces

		// read the item
		if ((c >= '0') && (c <= '9')) {
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			data->setClass(item);
			ok = true;
			break;
		}
	}
	// Read Items
	while (!eof() && c!='\n') {
		c = fgetc(_f);  // eat all white-spaces

		// read the item
		if ((c >= '0') && (c <= '9')) {
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			data->append(item); // add item
		}
	}

	return ok;
}

bool IO_Handler::ascii_read_xsset(DMT_ExtSuppSet* data) {
  	int c = 0;
  	data->reset();
  	bool ok = false;


  	// Read Items
	while (!eof() && c!='\n' && c!='(') {
  		c = fgetc(_f);  // eat all white-spaces

  		// read the item
  		if ((c >= '0') && (c <= '9')) {
  			int item = c-'0';
  			c = fgetc(_f);
  			while((c >= '0') && (c <= '9')) {
  				item *=10;
  				item += c-'0';
  				c = fgetc(_f);
  			}
  			data->append(item); // add item
  		}
  	}

  	// Read Supp
	while (!eof() && c!='\n' && c!='[') {
  		c = fgetc(_f);  // eat all white-spaces

  		// read the item
  		if ((c >= '0') && (c <= '9')) {
  			int item = c-'0';
  			c = fgetc(_f);
  			while((c >= '0') && (c <= '9')) {
  				item *=10;
  				item += c-'0';
  				c = fgetc(_f);
  			}
  			data->setSupport(item); // set support
  		}
  	}

	// Read Transactions
	while (!eof() && c!='\n') {
  		c = fgetc(_f);  // eat all white-spaces

  		// read the item
  		if ((c >= '0') && (c <= '9')) {
  			int item = c-'0';
  			c = fgetc(_f);
  			while((c >= '0') && (c <= '9')) {
  				item *=10;
  				item += c-'0';
  				c = fgetc(_f);
  			}
  			data->appendTransaction(item); // transaction
			ok = true;
  		}
  	}

	return ok;
}

void IO_Handler::ascii_read_sset(DMT_SuppSet* data)
	throw (IO_Handler_Exception_EmptyLine, IO_Handler_Exception_Format) {
  	int c = 0;
  	data->reset();

  	// skip white spaces
  	while (!((c >= '0') && (c <= '9')) && c!='(' && !eof() && c!='\n')
  		c = fgetc(_f);

  	if (eof()) throw IO_Handler_Exception_EmptyLine();

  	// Read Items
  	do {
  		if ((c >= '0') && (c <= '9')) { // read the item
  			int item = c-'0';
  			c = fgetc(_f);
  			while((c >= '0') && (c <= '9')) {
  				item *=10;
  				item += c-'0';
  				c = fgetc(_f);
  			}
  			data->append(item); // add item
  				// throw IO_Handler_Exception_Format("Itemset seems to be too long.");
  		}
  		if (c=='(') break;
  		c = fgetc(_f);  // eat all white-spaces
  	} while (!eof() && c!='\n');

	if (c!='(') throw IO_Handler_Exception_Format("Missing support information.");

  	// Read Supp
  	bool ok = false;
	while (!eof() && c!='\n') {
  		c = fgetc(_f);  // eat all white-spaces

  		// read the item
  		if ((c >= '0') && (c <= '9')) {
  			int item = c-'0';
  			c = fgetc(_f);
  			while((c >= '0') && (c <= '9')) {
  				item *=10;
  				item += c-'0';
  				c = fgetc(_f);
  			}
  			data->setSupport(item); // set support
  			ok = true;
  		}
  	}

	if (!ok) throw IO_Handler_Exception_Format("Ill-formed support information.");
}



void IO_Handler::ascii_read_lwtr(DMT_Labeled_Weighted_Transaction* data)
	throw (IO_Handler_Exception_EmptyLine, IO_Handler_Exception_Format) {
	int c = 0;
  	data->reset();

  	// Read Class
  	// skip white spaces
  	while (!((c >= '0') && (c <= '9')) && !eof() && c!='\n')
  		c = fgetc(_f);

  	if (eof()) throw IO_Handler_Exception_EmptyLine();

	if ((c >= '0') && (c <= '9')) {
		int t_class = c-'0';
		c = fgetc(_f);
		while((c >= '0') && (c <= '9')) {
			t_class *= 10;
			t_class += c-'0';
			c = fgetc(_f);
		}
		data->setClass(t_class);
	} else throw IO_Handler_Exception_Format(); // class was not found

	// Read items
	while (!eof() && c!='\n') {
		// skip white spaces
		while (!((c >= '0') && (c <= '9')) && !eof() && c!='\n')
			c = fgetc(_f);

		if ((c >= '0') && (c <= '9')) {
			// read feature id
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			// there should be the ':' separator
			if (c!=':') throw IO_Handler_Exception_Format();

			// read weight
			float value=0.0f;
			bool w_found = false;
			c = fgetc(_f);
			// read integer part
			if ((c >= '0') && (c <= '9')) {
				w_found = true;
				value = (float)(c-'0');
				c = fgetc(_f);
				while((c >= '0') && (c <= '9')) {
					value *= 10;
					value += (float)(c-'0');
					c = fgetc(_f);
				}
			}
			if (c=='.') {
				// read fractional part
				c = fgetc(_f);
				float position = 0.1;
				while((c >= '0') && (c <= '9')) {
					w_found = true;
					value += (float)(c-'0')*position;
					position*=0.1;
					c = fgetc(_f);
				}
			}

			if (!w_found) throw IO_Handler_Exception_Format(); // weight not found

			data->append(item, value);
		}
	}
}


void IO_Handler::ascii_read_wtr(DMT_Weighted_Transaction* data)
	throw (IO_Handler_Exception_EmptyLine, IO_Handler_Exception_Format) {
	int c = 0;
  	data->reset();

  	// Read Class
  	// skip white spaces
  	while (!((c >= '0') && (c <= '9')) && !eof() && c!='\n')
  		c = fgetc(_f);

  	if (eof()) throw IO_Handler_Exception_EmptyLine();

	// Read items
	while (!eof() && c!='\n') {
		// skip white spaces
		while (!((c >= '0') && (c <= '9')) && !eof() && c!='\n')
			c = fgetc(_f);

		if ((c >= '0') && (c <= '9')) {
			// read feature id
			int item = c-'0';
			c = fgetc(_f);
			while((c >= '0') && (c <= '9')) {
				item *=10;
				item += c-'0';
				c = fgetc(_f);
			}
			// there should be the ':' separator
			if (c!=':') throw IO_Handler_Exception_Format();

			// read weight
			float value=0.0f;
			bool w_found = false;
			c = fgetc(_f);
			// read integer part
			if ((c >= '0') && (c <= '9')) {
				w_found = true;
				value = (float)(c-'0');
				c = fgetc(_f);
				while((c >= '0') && (c <= '9')) {
					value *= 10;
					value += (float)(c-'0');
					c = fgetc(_f);
				}
			}
			if (c=='.') {
				// read fractional part
				c = fgetc(_f);
				float position = 0.1;
				while((c >= '0') && (c <= '9')) {
					w_found = true;
					value += (float)(c-'0')*position;
					position*=0.1;
					c = fgetc(_f);
				}
			}

			if (!w_found) throw IO_Handler_Exception_Format(); // weight not found

			data->append(item, value);
		}
	}
}



class AsciiTransactionReader : public IO_Handler {
public:
	AsciiTransactionReader(FILE* fh) : IO_Handler(fh) {};
	virtual int read(int* data, int max_len) {
	  	int c = 0;
		int len = 0;
		do {
			c = fgetc(_f);
			if ((c >= '0') && (c <= '9')) {
				unsigned int item = c-'0';
				c = fgetc(_f);
				while((c >= '0') && (c <= '9')) {
					item *=10;
					item += c-'0';
					c = fgetc(_f);
				}
				if (len>=max_len) return -1;
				data[len++] = item;
			}
		} while(c != '\n' && !eof());
		return len;
	}

	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Transaction* t = dynamic_cast<DMT_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		try {
			ascii_read_tr(t);
		} catch (IO_Handler_Exception_EmptyLine& e) {
			t->reset();
		}
		t->setTid(_tid++);
		return t->getLen();
	}

};

class BinaryTransactionWriter : public IO_Handler {
public:
	BinaryTransactionWriter(FILE* fh) : IO_Handler(fh) {};
	virtual int write(int* data, int len) {
		return 	fwrite(&len, sizeof(int), 1, _f) +
				fwrite(data, sizeof(int), len, _f);
	}
};

class BinaryTransactionReader : public IO_Handler {
public:
	BinaryTransactionReader(FILE* fh) : IO_Handler(fh) {};
	virtual int read(int* data, int max_len) {
		int len;
	    int read = fread(&len, sizeof(int), 1, _f);
	    if (read<=0 || len>max_len) return -1;
	    read = fread(data, sizeof(int), len, _f);
		if (read!=len) return -1;
		return len;
	}
};

class AsciiPatternWriter : public IO_Handler {
public:
	AsciiPatternWriter(FILE* fh) : IO_Handler(fh) {};

	// TODO: cached writer ???
	virtual int write(int* data, int len, int* data2, int len2) {
		// print items
		for (int i=0; i<len; i++)
			fprintf(_f, "%d ", data[i]);
		// print support
		fprintf(_f, "(%d)", len2);
		// print transactions
		if (len2>0 && data2!=NULL) {
			fprintf(_f, " [%d", data2[0]);
			for (int i=1; i<len2; i++)
				fprintf(_f, " %d", data2[i]);
			fprintf(_f, "]");
		}
		fprintf(_f, "\n");

		// always return success
		return 1;
	}
};

/**
 *  Write transaction with label and weight in binary format
 */
class BinaryLabeledWeightedTransactionWriter : public IO_Handler {
public:
	BinaryLabeledWeightedTransactionWriter(FILE* fh) : IO_Handler(fh) {};

	/**
	 * write transaction to file
	 * \param data is the transaction to be stored.
	 */
	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Labeled_Weighted_Transaction* t = dynamic_cast<DMT_Labeled_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		int tclass = t->getClass();
		int tlen = t->getLen();
		fwrite(&tclass, sizeof(int), 1, _f);
		fwrite(&tlen, sizeof(int), 1, _f);
		for (int i=0; i<t->getLen(); i++) {
			int item = t->getItem(i);
			float weight = t->getWeight(i);
			fwrite(&item, sizeof(int), 1, _f);
			fwrite(&weight, sizeof(float), 1, _f);
		}
		return 1;
	}
};

/**
 *  Read transaction with label and weight in binary format
 */
class BinaryLabeledWeightedTransactionReader : public IO_Handler {
public:
	BinaryLabeledWeightedTransactionReader(FILE* fh) : IO_Handler(fh) {};

	/**
	 * read transaction from file
	 * \param data is where the transaction is stored.
	 */
	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Labeled_Weighted_Transaction* t = dynamic_cast<DMT_Labeled_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		t->reset();
		int tclass, tlen, item;
		float weight;
		fread(&tclass, sizeof(int), 1, _f);
		t->setClass(tclass);
		fread(&tlen, sizeof(int), 1, _f);
		for (int i=0; i<tlen; i++) {
			fread(&item, sizeof(int), 1, _f);
			fread(&weight, sizeof(float), 1, _f);
			t->append(item, weight);
		}
		t->setTid(_tid++);
		return 1;
	}
};



/**
 *  Write transaction with weights in binary format
 */
class BinaryWeightedTransactionWriter : public IO_Handler {
public:
	BinaryWeightedTransactionWriter(FILE* fh) : IO_Handler(fh) {};

	/**
	 * write transaction to file
	 * \param data is the transaction to be stored.
	 */
	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Weighted_Transaction* t = dynamic_cast<DMT_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		int tlen = t->getLen();
		fwrite(&tlen, sizeof(int), 1, _f);
		for (int i=0; i<t->getLen(); i++) {
			int item = t->getItem(i);
			float weight = t->getWeight(i);
			fwrite(&item, sizeof(int), 1, _f);
			fwrite(&weight, sizeof(float), 1, _f);
		}
		return 1;
	}
};

/**
 *  Read transaction with weights in binary format
 */
class BinaryWeightedTransactionReader : public IO_Handler {
public:
	BinaryWeightedTransactionReader(FILE* fh) : IO_Handler(fh) {};

	/**
	 * read transaction from file
	 * \param data is where the transaction is stored.
	 */
	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Weighted_Transaction* t = dynamic_cast<DMT_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		t->reset();
		int tlen, item;
		float weight;
		fread(&tlen, sizeof(int), 1, _f);
		for (int i=0; i<tlen; i++) {
			fread(&item, sizeof(int), 1, _f);
			fread(&weight, sizeof(float), 1, _f);
			t->append(item, weight);
		}
		t->setTid(_tid++);
		return 1;
	}
};


/** "Feature Format" Reader.
 *
 * Reads from ASCII files in "feature format".
 *
 * The "feature format" is as follows:
 * <pre>
 *  0 2:10.0 3:15.0 4:3
 *  1 3:7.0 4:4.0 5:10.0
 *  ...
 *  </pre>
 *  where the first number of each line is a class id,
 *  and the colon splits a feature id from the corresponding
 *  feature weight. Feature weights are floating point numbers.
 *  Feature ids are sorted increasingly.
 *  Plus and minus before the weight value are not accepted.
 *
 *  Class ids and feature ids should not overlap.
 *
 */
class AsciiLabeledWeightedTransactionReader : public IO_Handler {
public:
	AsciiLabeledWeightedTransactionReader(FILE* fh) : IO_Handler(fh) {};

	/**
	 * Reads a transaction from the input file.
	 *
	 * \param data is a pointer to an instance of a DMT_Labeled_Weighted_Transaction class.
	 * \returns the len of the read transaction
	 * \throws IO_Handler_Exception is raised when the file format is not as expected,
	 * or when argument dynamic data type is not correct.
	 */
	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Labeled_Weighted_Transaction* t = dynamic_cast<DMT_Labeled_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		try {
			ascii_read_lwtr(t);
		} catch (IO_Handler_Exception_EmptyLine& e) {
			t->reset();
		}
		t->setTid(_tid++);
		return t->getLen();
	}
};


/** "Feature Format" Reader.
 *
 * Reads from ASCII files in "feature format".
 *
 * The "feature format" is as follows:
 * <pre>
 *  2:10.0 3:15.0 4:3
 *  3:7.0 4:4.0 5:10.0
 *  ...
 *  </pre>
 *  where the colon splits a feature id from the corresponding
 *  feature weight. Feature weights are floating point numbers.
 *  Feature ids are sorted increasingly.
 *  Plus and minus before the weight value are not accepted.
 */
class AsciiWeightedTransactionReader : public IO_Handler {
public:
	AsciiWeightedTransactionReader(FILE* fh) : IO_Handler(fh) {};

	/**
	 * Reads a transaction from the input file.
	 *
	 * \param data is a pointer to an instance of a DMT_Weighted_Transaction class.
	 * \returns the len of the read transaction
	 * \throws IO_Handler_Exception is raised when the file format is not as expected,
	 * or when argument dynamic data type is not correct.
	 */
	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Weighted_Transaction* t = dynamic_cast<DMT_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		try {
			ascii_read_wtr(t);
		} catch (IO_Handler_Exception_EmptyLine& e) {
			t->reset();
		}
		t->setTid(_tid++);
		return t->getLen();
	}
};

// format is: c a b c d   (first item is the class id )
class AsciiLabeledTransactionReader : public IO_Handler {
public:
	AsciiLabeledTransactionReader(FILE* fh) : IO_Handler(fh) {};

	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Labeled_Transaction* t = dynamic_cast<DMT_Labeled_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		ascii_read_ltr(t);
		t->setTid(_tid++);
		return t->getLen();
	}
};

/** Transaction Writer
 * Format is described in \ref AsciiLabeledTransactionReader.
 */
class AsciiLabeledTransactionWriter : public IO_Handler {
public:
	AsciiLabeledTransactionWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Labeled_Transaction* t = dynamic_cast<DMT_Labeled_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		fprintf(_f, "%d", t->getClass());

		if (t->getLen()==0)
			fprintf(_f, "\n");
		else {
			int i;
			for (i=0; i<t->getLen()-1; i++)
				fprintf(_f, " %d", t->getItem(i));
			fprintf(_f, " %d\n", t->getItem(i));
		}
		return 1;
	}
};

/** Weighted transaction writer
 */
class AsciiLabeledWeightedTransactionWriter : public IO_Handler {
public:
	AsciiLabeledWeightedTransactionWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Labeled_Weighted_Transaction* t = dynamic_cast<DMT_Labeled_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		fprintf(_f, "%d", t->getClass());

		if (t->getLen()==0)
			fprintf(_f, "\n");
		else {
			int i;
			for (i=0; i<t->getLen(); i++)
				fprintf(_f, " %d:%.5f", t->getItem(i), t->getWeight(i));
			fprintf(_f, "\n");
		}
		return 1;
	}
};

/** Weighted transaction writer
 */
class AsciiWeightedTransactionWriter : public IO_Handler {
public:
	AsciiWeightedTransactionWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Weighted_Transaction* t = dynamic_cast<DMT_Weighted_Transaction*>(data);
		if (!t) throw new IO_Handler_Exception("Wrong data type.");

		if (t->getLen()==0)
			fprintf(_f, "\n");
		else {
			int i;
			for (i=0; i<t->getLen()-1; i++)
				fprintf(_f, "%d:%.5f ", t->getItem(i), t->getWeight(i));
			fprintf(_f, "%d:%.5f\n", t->getItem(i), t->getWeight(i));
		}
		return 1;
	}
};

class AsciiTransactionWriter : public IO_Handler {
public:
	AsciiTransactionWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Transaction* t = dynamic_cast<DMT_Transaction*>(data);
		if (!t) return -1;

		if (t->getLen()==0) {
			fprintf(_f, "\n");
		} else {
			int i;
			for (i=0; i<t->getLen()-1; i++)
				fprintf(_f, "%d ", t->getItem(i));
			fprintf(_f, "%d\n", t->getItem(i));
		}
		return 1;
	}
};


/** Itemsets Reader.
 *
 * Reads itemsets and their support from an ASCII file.
 *
 * The format is as follows:
 * <pre>
 *  1 2 3 4 (999)
 *  4 5 6 (777)
 *  ...
 *  </pre>
 *  where each number is an item, and support is between parentheses.
 *  Ordering of items is not assumed.
 *
 */
class AsciiSuppSetReader : public IO_Handler {
public:
	AsciiSuppSetReader(FILE* fh) : IO_Handler(fh) {};

	/**
	 * Reads an itemset from the input file.
	 *
	 * \param data is a pointer to an instance of a DMT_SuppSet class.
	 * \returns the len of the read transaction
	 * \throws IO_Handler_Exception is raised when the file format is not as expected,
	 * or when argument dynamic data type is not correct.
	 * \note If there is an empty line at the end of the file, or the last byte is a newline '\\n',
	 * the function will call the reset method of data and return its length anyway.
	 * In case of a zero length data is created, the caller may use eof() to check
	 * whether the file was completely consumed, or there is a true empty line in the middle of the file.
	 */
	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_SuppSet* sset = dynamic_cast<DMT_SuppSet*>(data);
		if (!sset) throw new IO_Handler_Exception("Wrong data type.");

		try {
			ascii_read_sset(sset);
		} catch (IO_Handler_Exception_EmptyLine& e) {
			sset->reset();
		}
		return sset->getLen();
	}
};

class AsciiSuppSetWriter : public IO_Handler {
public:
	AsciiSuppSetWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_SuppSet* sset = dynamic_cast<DMT_SuppSet*>(data);
		if (!sset) return -1;

		for (int i=0; i<sset->getLen(); i++)
			fprintf(_f, "%d ", sset->getItem(i));
		fprintf(_f, "(%d)\n", sset->getSupport());

		return 1;
	}
};

class AsciiExtSuppSetReader : public IO_Handler {
public:
	AsciiExtSuppSetReader(FILE* fh) : IO_Handler(fh) {};

	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_ExtSuppSet* set = dynamic_cast<DMT_ExtSuppSet*>(data);
		if (!set) return -1;

		ascii_read_xsset(set);
		return set->getLen();
	}
};
class AsciiExtSuppSetWriter : public IO_Handler {
public:
	AsciiExtSuppSetWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_ExtSuppSet* set = dynamic_cast<DMT_ExtSuppSet*>(data);
		if (!set) return -1;

		// Print pattern
		for(int i=0; i<set->getLen(); i++)
			fprintf(_f, "%d ", set->getItem(i));
		fprintf(_f, "(%d) [", set->getSupport());
		if (set->getTransactionsLen()>0) {
			for(int i=0; i<set->getTransactionsLen()-1; i++)
				fprintf(_f, "%d ", set->getTransaction(i));
			fprintf(_f, "%d", set->getTransaction( set->getTransactionsLen()-1 ) );
		}
		fprintf(_f, "]\n");

		return 1;
	}
};

class AsciiRuleWriter : public IO_Handler {
public:
	AsciiRuleWriter(FILE* fh) : IO_Handler(fh) {};

	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Rule* t = dynamic_cast<DMT_Rule*>(data);
		if (!t) return -1;

		// Print rule
		for(int i=0; i<t->getHead()->getLen(); i++)
			fprintf(_f, "%d ", t->getHead()->getItem(i));
		fprintf(_f, "> ");
		for(int i=0; i<t->getTail()->getLen(); i++)
			fprintf(_f, "%d ", t->getTail()->getItem(i));
		fprintf(_f, "(%.0f, %.2f)\n", t->getSupport(), t->getConfidence());

		return 1;
	}

};

class AsciiRuleReader : public IO_Handler {
public:
	AsciiRuleReader(FILE* fh) : IO_Handler(fh) {};

	virtual int read(DMT_DataStructure* data) throw (IO_Handler_Exception) {
		DMT_Rule* r = dynamic_cast<DMT_Rule*>(data);
		if (!r) return -1;

		ascii_read_rule(r);
		return r->getHead()->getLen() + r->getTail()->getLen();
	}

};


IO_Handler* IO_Handler::get_handler(const char* filename, IO_Format format, bool append)
	throw (IO_Handler_Exception_FileNotFound,IO_Handler_Exception_WrongHandlerCode)
{
	// open file
	FILE* f = NULL;
	switch(format) {
		// readers
		case TR_ASCII_READER :
		case TR_ASCII_LABELED_READER:
		case TR_ASCII_WEIGHTED_READER:
		case SSET_ASCII_READER:
		case XSET_ASCII_READER:
		case TR_ASCII_LABELED_WEIGHTED_READER:
		case RULE_ASCII_READER:
			f = fopen ( filename, "r");
			break;

		// writers
		case P_ASCII_WRITER:
		case SSET_ASCII_WRITER:
		case XSET_ASCII_WRITER:
		case RULE_ASCII_WRITER:
		case TR_ASCII_WRITER:
		case TR_ASCII_LABELED_WRITER:
		case TR_ASCII_LABELED_WEIGHTED_WRITER:
		case TR_ASCII_WEIGHTED_WRITER:
			f = fopen ( filename, append?"a":"w");
			break;

		// binary readers
		case TR_BINARY_READER:
		case TR_BINARY_LABELED_WEIGHTED_READER:
		case TR_BINARY_WEIGHTED_READER:
			f =  fopen ( filename, "rb");
			break;


		// binary writers
		case TR_BINARY_WRITER:
		case TR_BINARY_LABELED_WEIGHTED_WRITER:
		case TR_BINARY_WEIGHTED_WRITER:
			f =  fopen ( filename, append?"ab":"wb");
			break;
		default: ;
	}

  	if (!f) throw IO_Handler_Exception_FileNotFound();
		
	// create handler
	switch(format) {
		case TR_ASCII_READER  : return new AsciiTransactionReader(f);
		case TR_ASCII_WRITER  : return new AsciiTransactionWriter(f);
		case TR_BINARY_READER : return new BinaryTransactionReader(f);
		case TR_BINARY_WRITER : return new BinaryTransactionWriter(f);
		case P_ASCII_WRITER   : return new AsciiPatternWriter(f);
		case TR_ASCII_LABELED_READER           : return new AsciiLabeledTransactionReader(f);
		case TR_ASCII_LABELED_WRITER           : return new AsciiLabeledTransactionWriter(f);
		case TR_ASCII_WEIGHTED_READER          : return new AsciiWeightedTransactionReader(f);
		case TR_ASCII_WEIGHTED_WRITER          : return new AsciiWeightedTransactionWriter(f);
		case TR_ASCII_LABELED_WEIGHTED_READER  : return new AsciiLabeledWeightedTransactionReader(f);
		case TR_ASCII_LABELED_WEIGHTED_WRITER  : return new AsciiLabeledWeightedTransactionWriter(f);
		case TR_BINARY_LABELED_WEIGHTED_WRITER : return new BinaryLabeledWeightedTransactionWriter(f);
		case TR_BINARY_LABELED_WEIGHTED_READER : return new BinaryLabeledWeightedTransactionReader(f);
		case TR_BINARY_WEIGHTED_WRITER : return new BinaryWeightedTransactionWriter(f);
		case TR_BINARY_WEIGHTED_READER : return new BinaryWeightedTransactionReader(f);
		case SSET_ASCII_READER: return new AsciiSuppSetReader(f);
		case SSET_ASCII_WRITER: return new AsciiSuppSetWriter(f);
		case XSET_ASCII_READER: return new AsciiExtSuppSetReader(f);
		case XSET_ASCII_WRITER: return new AsciiExtSuppSetWriter(f);
		case RULE_ASCII_WRITER: return new AsciiRuleWriter(f);
		case RULE_ASCII_READER: return new AsciiRuleReader(f);
		default: throw IO_Handler_Exception_WrongHandlerCode();
	}
}

/** \pre hf is an handler to an already opened file */
IO_Handler::IO_Handler(FILE* fh) {
	_f = fh;
	fseek(_f, 0, SEEK_END);
	_file_size = ftell(_f);
	fseek(_f, 0, SEEK_SET);
	_tid = 0;
}

IO_Handler::~IO_Handler() {
	if (_f) fclose(_f);
}



