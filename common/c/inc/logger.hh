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

#ifndef __DMT_LOGGER_H
#define __DMT_LOGGER_H

#include <iostream>
#include <pthread.h>
using namespace std;


/*
 * This is the main macro to be used for the actual logging
 *  -------------------
 * this macro was created to skip all the output
 * when the level is wrong
 */
#define LOGGER(LOGGER, LOGGER_LEVEL, LOGGER_OUTPUT) 		\
	{	if (LOGGER_LEVEL >= LOGGER->getLevel()) { 			\
			LOGGER->LOG(LOGGER_LEVEL) << LOGGER_OUTPUT;		\
			LOGGER->END_LOG(LOGGER_LEVEL);} 				\
	}


/*
 * Threadsafe Logger Class
 */
class LoggerDestroyer;
class Logger {
public:
	
	static char* REM;

	// logging levels
	enum LogLevels {
			TRACE, 		// This is the most detailed
			DEBUG, 		// Less detailed debug info
			VERBOSEMODE,    // Verbose output
			INFO,  		// Whatever output
			WARN,		// Warnings
			ERROR, 		// Relevant Errors
			FATAL		// Program exits after this
		};

	// create the logger and set/get the level
	static Logger* Instance(Logger::LogLevels level=Logger::INFO);
	void selectLevel(Logger::LogLevels level);
	Logger::LogLevels getLevel(void);

	// Get a cout-like operator
	ostream& LOG(LogLevels level);
	void END_LOG(LogLevels level);

protected:
	Logger();
  	Logger(const Logger &);
  	Logger & operator=(const Logger &);
	~Logger();
	friend class LoggerDestroyer;
	
private:
	static Logger* _instance;
	static LoggerDestroyer _destroyer; 
		// the destroyer of this static var will do the job

	pthread_mutex_t _output_lock;
	LogLevels _current_level;
};



#endif
