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

#include "../inc/logger.hh"

#include <cstdlib>

class LoggerDestroyer {
public:
	LoggerDestroyer() {
		_logger = NULL;
	};

	// also used to init the mutex
	void setLogger(Logger* o) { 
		_logger = o;
		pthread_mutex_init( &(_logger->_output_lock), NULL );
	}

	// also used to destroy the mutex
	~LoggerDestroyer() {
		if(_logger) {
			pthread_mutex_destroy( &(_logger->_output_lock) );
			delete _logger;
		}
	}
private:
	Logger* _logger;
};

// static shared variables
Logger* Logger::_instance = NULL;
LoggerDestroyer Logger::_destroyer;
char comment [] = {'#'};
char* Logger::REM = &comment[0];


Logger::Logger() {}
Logger::~Logger() {}

Logger* Logger::Instance(Logger::LogLevels level) {
	if (!Logger::_instance) {
		Logger::_instance = new Logger();
		Logger::_instance->selectLevel(level);
		Logger::_destroyer.setLogger(_instance);
	}
	return _instance;
}

void Logger::selectLevel(Logger::LogLevels level) {
	_current_level = level;
}

Logger::LogLevels Logger::getLevel(void){
	return _current_level;
}

ostream& Logger::LOG(LogLevels level) {
  	// lock
  	pthread_mutex_lock( &_output_lock );

	if ( level==Logger::FATAL ) {
	  	cout << endl
       		<< REM << " !! !!!!!!!!!!!!!!!!!!!! !! " << endl
       		<< REM << " !! Fatal Error          !! " << endl
       		<< REM << " !! -------------------- !! " << endl;
	}
	return cout;
}

void Logger::END_LOG(LogLevels level) {
	// unlock
  	pthread_mutex_unlock( &_output_lock );

	if ( level==Logger::FATAL )
	  	exit(1);
}




