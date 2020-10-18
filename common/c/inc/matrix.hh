/*
  Copyright (C) 2009, Claudio Lucchese

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

#ifndef __DMT_MATRIX_H
#define __DMT_MATRIX_H


#include "../inc/logger.hh"


/**
 * Upper Triangular matrix.
 * Handles an upper triangular matrix without the diagonal.
 */
template <class T1>
class Tri_Upper {
public:
	/**
	 * Creates the triangular matrix NxN given N.
	 * \param side the side of the triangular matrix
	 */
	Tri_Upper(long side) {
		_n = side;
		// size of the memory buffer
		_sz = (_n*(_n-1))/2;
		_buf   = new T1[_sz];
		if (!_buf) LOGGER(Logger::Instance(), Logger::FATAL, "Could not allocate memory of upper triangular matrix.");
		memset(_buf, 0x0, _sz * sizeof(T1));
	}

	/** Destructor */
	~Tri_Upper() { delete [] _buf; }

	/** Returns/sets an element of the matrix
	 * \param i row
	 * \param j column
	 * \pre i<j<=N, i.e. not test on indices is done.
	 */
	inline T1& operator()(long i, long j) {
		return _buf[direct_position(i, j)];
	}

	/** Returns the side of the matrix
	 * \return the side of the matrix */
	inline long getSize() { return _n; }

	/** Returns a new matrix, where a row and a column have been removed
	 * \param row the row/column to be removed
	 * \return a new smaller matrix
	 * \pre row is valid
	 * \note new matrix must be de-allocated by the caller.
	 */
	inline Tri_Upper* shrink(long row) {
		Tri_Upper* m = new Tri_Upper(_n-1);
		long src=0, dest=0;
		for (long r=0; r<_n-1; r++) {
			if (r==row) {// skip row
				src += _n-r-1;
				continue;
			}
			for (long c=r+1; c<_n; c++) {
				if (c!=row)
					m->_buf[dest++] = _buf[src];
				src++;
			}
		}
		return m;
	}

	/** finds the maximum element in the matrix
	 * \param &row will store the row of the maximum element
	 * \param &col will store the col of the maximum element
	 * \return the maximum value
	 * \note Takes linear time.
	 */
	inline T1 findMax(long &row, long &col) {
		long r = 0, c = 1;			//  initial values
		T1 max = _buf[0];			//
		long max_row=r, max_col=c; 	//
		for (long i=1; i<_sz; i++) {
			c++;
			if (c==_n) c = ++r +1;
			if (_buf[i]>max) {
				max = _buf[i];
				max_row = r;
				max_col = c;
			}
		}
		row = max_row;
		col = max_col;
		return max;
	}

private:
	/** Converts coordinates in 2D space into a flat address space */
	inline int direct_position(long x, long y) {
		return x*_n + y - ((x+1)*(x+2))/2; }

	T1 *_buf;
	long _n;
	long _sz;
};

#endif
