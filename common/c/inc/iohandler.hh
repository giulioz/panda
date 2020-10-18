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

#ifndef __DMT_IOHANDLER_H
#define __DMT_IOHANDLER_H

#include <stdio.h>

#include "basicstruct.hh"


/**
 * This is used to raise exceptions when something wrong occurs during file reading/writing.
 */
class IO_Handler_Exception {
public:
	/** \param msg Is the message stored within the exeption
	 * and returned by the \ref getMessage function.
	 */
	IO_Handler_Exception(const char* msg) {_msg = msg;}

	/** \returns the message describing the exception.
	 */
	const char* getMessage() {return _msg;}
private:
	const char* _msg;
};

/**
 * Handles empty and zero-length lines.
 */
class IO_Handler_Exception_EmptyLine : public IO_Handler_Exception {
public:
	IO_Handler_Exception_EmptyLine(const char* msg="Empty line.") : IO_Handler_Exception(msg) {};
};
/**
 * Handles empty and zero-length lines.
 */
class IO_Handler_Exception_Format : public IO_Handler_Exception {
public:
	IO_Handler_Exception_Format(const char* msg="Wrong Format.") : IO_Handler_Exception(msg) {};
};
/**
 * File not found execption.
 */
class IO_Handler_Exception_FileNotFound : public IO_Handler_Exception {
public:
	IO_Handler_Exception_FileNotFound(const char* msg="File not found.") : IO_Handler_Exception(msg) {};
};
/**
 * Wrong Handler Code.
 */
class IO_Handler_Exception_WrongHandlerCode : public IO_Handler_Exception {
public:
	IO_Handler_Exception_WrongHandlerCode(const char* msg="Wrong Handler Code.") : IO_Handler_Exception(msg) {};
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Transaction Handler abstract class
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class IO_Handler {
public:
	// accepted file formats
	enum IO_Format {TR_ASCII_READER, TR_ASCII_WRITER, TR_BINARY_READER, TR_BINARY_WRITER,
					TR_ASCII_LABELED_READER, TR_ASCII_LABELED_WRITER,
					// transactions with weights
					TR_ASCII_WEIGHTED_READER, TR_ASCII_WEIGHTED_WRITER,
					TR_BINARY_WEIGHTED_READER, TR_BINARY_WEIGHTED_WRITER,
					 // transactions with labels and weights
					TR_ASCII_LABELED_WEIGHTED_READER, TR_ASCII_LABELED_WEIGHTED_WRITER,
					TR_BINARY_LABELED_WEIGHTED_READER, TR_BINARY_LABELED_WEIGHTED_WRITER,
					P_ASCII_WRITER,
					SSET_ASCII_READER, SSET_ASCII_WRITER, // itemset with support
					XSET_ASCII_READER, XSET_ASCII_WRITER, // itemset with support and transactions
					RULE_ASCII_WRITER, RULE_ASCII_READER
				};

	static IO_Handler* get_handler(const char* filename, IO_Format format, bool append=false)
			throw (IO_Handler_Exception_FileNotFound,IO_Handler_Exception_WrongHandlerCode);

	IO_Handler(FILE* f);
		
	// --------------- DMT Data Structures  ----------------------
	virtual int read(DMT_DataStructure* data)  throw (IO_Handler_Exception) {return -1;};
	virtual int write(DMT_DataStructure* data) throw (IO_Handler_Exception) {return -1;};


	// --------------- Back-Compatibility ---------------------------
	// read an item from the stream
	//         returns -1 if buffer is not sufficient to store the itemset
	//                       or if something else goes wrong
	//         otherwise return the number of read/written items
	virtual int read(int* data, int max_len) {return -1;};

	// write and itemset to the stream
	virtual int write(int* data, int len) {return -1;};

	// write a pattern to the stream
	virtual int write(int* data, int len, int* data2, int len2) {return -1;};

	// --------------- File Operations ---------------------------
	// end of file
	inline int eof() {return ftell(_f)>=_file_size;}

	// rewind file
	inline void rew() {rewind(_f); _tid=0;}
	
	// close file
	inline void close() {fclose(_f); _f=NULL;}

	inline int getNoTransactionsRead() { return _tid; }

	virtual ~IO_Handler();
	
protected:
	FILE* _f;
	int _file_size;
	int _tid;

	void ascii_read_sset(DMT_SuppSet* data) throw (IO_Handler_Exception_EmptyLine, IO_Handler_Exception_Format);
	bool ascii_read_xsset(DMT_ExtSuppSet* data);
	bool ascii_read_ltr(DMT_Labeled_Transaction* data);
	bool ascii_read_rule(DMT_Rule* data);
	bool ascii_read_tr(DMT_Transaction* data);
	void ascii_read_wtr(DMT_Weighted_Transaction* data) throw (IO_Handler_Exception_EmptyLine, IO_Handler_Exception_Format);
	void ascii_read_lwtr(DMT_Labeled_Weighted_Transaction* data) throw (IO_Handler_Exception_EmptyLine, IO_Handler_Exception_Format);

};

#endif
