

#ifndef tinyCmdTable_h
#define tinyCmdTable_h

#include <inttypes.h>

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif

#define MAX_CMD_LINE 80
#define MAX_CMD_PARAMETERS  4
#define MAX_CMDS 12
#define MAX_VALUE_LEN 12
//
#define CMD_LENGTH_STRICT 6		// 4 + room for any syntax and zero
#define MAX_PREAMBLE_SIZE 16
#define MAX_COMMAS (MAX_CMD_PARAMETERS+4)



static inline uint8_t locate_zero_commas(char *src, char *wordStart[MAX_COMMAS]) {
	//
	
	uint8_t comma_count = 0;
	
	wordStart[comma_count++] = src;
	
	while ( *src ) {
		char c = *src;
		if ( c == ',' ) {
			*src = 0;
			wordStart[comma_count++] = (src + 1);
			if ( comma_count >= MAX_COMMAS ) break;
		}
		src++;
	}
	
	return(comma_count);
}



static inline void copy_upTo(char *src, char *end, char *dst) {
	memcpy(dst,src,(unsigned int)(end - src));
}


class tinyCmdTable {
public:
	tinyCmdTable(uint8_t linesize,uint8_t appMaxCmd) {
		//
		// manage commands
		_ready = false;
		_errorSet = false;
		//
		_comma_count = 0;
		//
		memset(_values[0],0,MAX_VALUE_LEN);
		memset(_values[1],0,MAX_VALUE_LEN);
		memset(_values[2],0,MAX_VALUE_LEN);
		memset(_values[3],0,MAX_VALUE_LEN);
		//
		_maxCMDs = min(appMaxCmd,MAX_CMDS);
		_cnum = 0xFF;
		
		// read in cmd string
		_linesize = min(MAX_CMD_LINE,linesize);
		reset();
	}
	virtual ~tinyCmdTable() {}
	
	
	uint8_t cmdNumber(void) {
		return(_cnum);
	}
	
	// ---- ---- ---- ---- ---- ---- ---- ---- ----
	uint8_t unload_uint8(uint8_t paramNum) {
		uint8_t ival;
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			//
			uint8_t ival = (uint8_t)atoi(value);
			return(ival);
		}
		return(0);
	}
	
	uint16_t unload_uint16(uint8_t paramNum) {
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			uint16_t ival = (uint16_t)atoi(value);
			return(ival);
		}
		return(0);
	}
	
	uint32_t unload_uint32(uint8_t paramNum) {
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			uint32_t ival = atol(value);
			return(ival);
		}
		return(0);
	}
	
	int8_t unload_int8(uint8_t paramNum) {
		uint8_t ival;
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			//
			int8_t ival = atoi(value);
			return(ival);
		}
		return(0);
	}
	
	int16_t unload_int16(uint8_t paramNum) {
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			int16_t ival = atoi(value);
			return(ival);
		}
		return(0);
	}
	
	int32_t unload_int32(uint8_t paramNum) {
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			int32_t ival = atol(value);
			return(ival);
		}
		return(0);
	}
	
	
	double unload_float(uint8_t paramNum) {
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			//
			double fval = atof(value);
			return(fval);
		}
		return(0.0);
	}
	
	
	bool unload_bool(uint8_t paramNum) {
		//
		if ( paramNum < MAX_CMD_PARAMETERS ) {
			char * value = &(_values[paramNum][0]);
			if ( strlen(value) > 0 ) {
				if ( strcmp(value,"true") == 0 ) return(true);
				else if ( value[0] == '1' ) return(true);
				else if ( strcmp(value,"on") == 0 ) return(true);
				else if ( strcmp(value,"HIGH") == 0 ) return(true);
				else if ( strcmp(value,"yes") == 0 ) return(true);
			}
		}
		
		return(false);
	}
	
	
	///
	void addChar(char c);
	
	bool parseCommandString(void);
	bool ready(void) { return _ready; }
	
	char 	* preamble(void) { return _preamble; }
	bool 	unloadParameters(void);
	
	//
	bool reset(bool state = true) {
		//
		_ready = false;
		_errorSet = !state;
		//
		_lineindex = 0;
		for ( uint8_t i = 0; i < MAX_CMD_LINE; i++ ) { _buffer[i] = 0; }
		for ( uint8_t i = 0; i < MAX_CMD_PARAMETERS; i++ ) { memset(_values[i],0,MAX_VALUE_LEN); }
		//
	}
	
	bool error(void) {
		bool inError = _errorSet;
		_errorSet = false;
		return(inError);
	}
	
	void reportReadState(const char *tag) {
		Serial.print("DBG=->");
		Serial.print(tag);
	}
	
	// ---- ---- ---- ---- ---- ---- ---- ----
	uint8_t locateCommmas(void);
	bool unloadCommand(void);
	
	
protected:
	
	bool 			_ready;
	bool			_errorSet;
	//
	uint8_t			_maxCMDs;
	uint8_t			_cnum;
	uint8_t			_comma_count;
	//
	uint8_t			_readState;
	
	//
	char 			_buffer[MAX_CMD_LINE];
	uint8_t			_linesize;
	uint8_t			_lineindex;
	//
	char 			_preamble[MAX_PREAMBLE_SIZE];
	//
	char 			_values[MAX_CMD_PARAMETERS][MAX_VALUE_LEN];
	char 			*_commaPointer[MAX_COMMAS];  // just locate the commas in the input string...
};



#endif
