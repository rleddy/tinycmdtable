
#include "tinyCmdTable.h"


void tinyCmdTable::addChar(char c) {
	
	if ( c == '\n' ) {
		_buffer[_lineindex++] = 0;
		//
		_ready = parseCommandString();
		
	} else {
		//
		if ( _lineindex >= _linesize ) {
			_ready = parseCommandString();
		} else {
			_ready = false;
			_buffer[_lineindex++] = c;
		}
	}
	
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----



///
uint8_t tinyCmdTable::locateCommmas(void) {
	_comma_count = locate_zero_commas(_buffer, _commaPointer);
}



bool tinyCmdTable::unloadCommand(void) {
	//
	
	locateCommmas();
	
	memset(_preamble,0,MAX_PREAMBLE_SIZE);
	strcpy(_preamble,_commaPointer[0]);
	
	_cnum = atoi(_commaPointer[1]);
	
	if ( _cnum > _maxCMDs ) {
		_cnum = 0xFF;
		return(false);
	}
	
	return(true);
}


bool tinyCmdTable::unloadParameters(void) {
	
	for ( int i = 2; i < _comma_count; i++  ) {
		//
		char *val = _commaPointer[i];
		//
		if ( val != NULL ) {
			strcpy(_values[i-2],val);
		} else {
			reset();  // failed to parse
			return(false);
		}
	}
	
	return(true);
}

bool tinyCmdTable::parseCommandString(void) {
	
	if ( unloadCommand() ) {
		return(unloadParameters());
	} else {
		reset();  // failed to parse
		return(false);
	}
	return true;
}

