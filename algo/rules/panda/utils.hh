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

#ifndef __UTILS_H
#define __UTILS_H

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <vector>
#include <stdlib.h>

using namespace std;

#include "chronos.hh"


/*
 * Return the position of a sorted set
 */
template <class vector1, class vector2 >
void indexQuickSort(int first, int last, vector1 v, vector2 data, bool ascending=true) {
  int middle;
  if (first < last) {  
    int x = data[v[first]];
    int i = first - 1;
    int j = last + 1;
  
    while (1) {
      do { j--;
      } while ((ascending && data[v[j]] > x) || (!ascending && data[v[j]] < x));
      do { i++;
      } while ((ascending && data[v[i]] < x) || (!ascending && data[v[i]] > x));
      if (i < j) {
  std::swap(v[i],v[j]);
      } else {
  middle=j;
  break;
      }
    }
    
    indexQuickSort(first, middle, v, data, ascending);
    indexQuickSort(middle+1, last, v, data, ascending);
  }
}

/*
 * Return the position of a sorted set (randomized)
 * assume positive values
 */
template <class vector1, class vector2 >
void indexQuickSortRandom(int first, int last, vector1 v, vector2 data, float T, bool ascending=true) {
  // draw new values
  srand ( time(NULL) );

  vector<int> counts(last+1);
  float max_val = -1;
  for(int i=first; i<=last; i++) {
    counts[i] = pow((float)data[i], T);
    if (max_val < counts[i]) max_val = counts[i];
  }
  for(int i=first; i<=last; i++)
    if (counts[i]>0)
      counts[i] = rand() % (int)( (counts[i]/max_val)*(float)RAND_MAX );

  indexQuickSort(first, last, v, counts, ascending);
}



/*
 * Sorts the given vector
 * NB: last is inclusive!
 */
template <class vector>
void quickSort(int first, int last, vector v, bool ascending=true) {
  int middle;
  if (first < last) {  
    int x;
    int i, j;
  
    x = v[first];
    i = first - 1;
    j = last + 1;
  
    while (1) {
      do { j--;
      } while ((ascending && v[j] > x) || (!ascending && v[j] < x));
      do { i++;
      } while ((ascending && v[i] < x) || (!ascending && v[i] > x));
      if (i < j) {
	int tmp;
	tmp = v[i];
	v[i] = v[j];
	v[j] = tmp;
      } else {
	middle=j;
	break;
      }
    }
    
    quickSort(first, middle, v, ascending);
    quickSort(middle+1, last, v, ascending);
  }
}



// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// this is not what i really wanted !!!!!!!
// timed lock and ulock are not thread safe !!!!!!!
class TimedMutex {
  Chronos timer;
  pthread_mutex_t sem;
  double waiting_time;

public:
  TimedMutex(){
    pthread_mutex_init(&sem, NULL);
    waiting_time = 0;
  }

  ~TimedMutex(){
    pthread_mutex_destroy(&sem);
  }
  
  inline void lock() {
    pthread_mutex_lock(&sem);
  }
  inline void timedlock() {
    timer.RestartChronos();
    lock();
  }
  inline void unlock() {
    pthread_mutex_unlock(&sem);
  }
  inline void timedunlock() {
    waiting_time += timer.ReadChronos();
    unlock();
  }

  double gettime() {
    return waiting_time;
  }
};



#endif
