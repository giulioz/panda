/*
  Copyright (C) 2009, Claudio Lucchese

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

#ifndef __DMT_CHRONOS_H
#define __DMT_CHRONOS_H

#include <sys/time.h>

/*
 * Timer
 */
class Chronos {
public:
  Chronos() {Time=0; State=READY_CHRONOS;}
  
  void   StartChronos();
  void   StopChronos();
  double ReadChronos();
  void   ResetChronos();
  void   RestartChronos();

private:
  struct timeval tv;
  struct timezone tz;
  long sec1,sec2,sec3;
  long usec1,usec2,usec3;
  double Time;
  int State;

  static const int RUNNING_CHRONOS = 2;
  static const int STOPPED_CHRONOS = 1;
  static const int READY_CHRONOS   = 0;
};

/*
 * shared clock that can be used at any time
 */
extern Chronos __GLOBAL_CLOCK;


#endif
