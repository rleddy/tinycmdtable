#include <tinyCmdTable.h>


#define TASKER_MAX_TASKS 4
#include <Tasker.h>
#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>


//
// ---- ---- ---- ---- ---- ----
//

//
//

#define QUERY_STATE_ 0
#define LIMIT_ 1
#define SET_TIME_ 2
#define TOGGLE_READ_SENSORS_ 3
#define LAMP_ 4
#define PUMP_ 5
#define HEATER_ 6
#define STEPPER_ 7

//
//
#define NUMCMDS 8
#define BUFSIZE 128

#define SENSOR_ALL_ 0
#define SENSOR_OD_ 1
#define SENSOR_Temperature_ 2



tinyCmdTable cmdTable(BUFSIZE,NUMCMDS);


////////////////////

Tasker gTasker;
//
bool gReporting = false;
static unsigned int gSampleInterval = 1000; // default 1 sec


typedef struct {
  uint8_t sensor;
  double low;
  double high;
  bool checking;
  int currentState;
} sensorLimit;


////////////////////


int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2




////////////////////


////////////////////

bool gCheckLimits = false;
uint8_t gSensorCount = 2;
static sensorLimit appSensors[2] = {
  { 0, -100.0,100.0, false, 0 },       // set beyond possible measurments
  { 1, -10000.0, 10000.0, false, 0 }
};




void setLimitOnSensor(char *sensor,float upperLimit,float lowerLimit) {
  for ( uint8_t i = 0; i < gSensorCount; i++ ) {
    if ( strcmp(appSensors[i].sensor,sensor) == 0 ) {
      appSensors[i].checking = !(appSensors[i].checking);
      appSensors[i].low = lowerLimit;
      appSensors[i].high = upperLimit;
      return;
    }
  }
}

#define LIMITS_LOWER_BOUND_REPORT_COUNT -4
#define LIMITS_UPPER_BOUND_REPORT_COUNT 4
#define LIMITS_LOWER_BOUND_REPORT_RESTART 10
#define LIMITS_UPPER_BOUND_REPORT_RESTART -10

int checkLimitOnSensor(uint8_t sensor,float value) {
      //
  if ( sensor <  gSensorCount ) {
      if ( value < appSensors[sensor].low ) {
        appSensors[sensor].currentState--;
        if ( appSensors[sensor].currentState < LIMITS_LOWER_BOUND_REPORT_COUNT ) {
          appSensors[sensor].currentState = LIMITS_LOWER_BOUND_REPORT_RESTART;
          return(0);
        }
        return -1;
      } else if (  value > appSensors[sensor].high ) {
        appSensors[sensor].currentState++;
        if ( appSensors[sensor].currentState > LIMITS_UPPER_BOUND_REPORT_COUNT ) {
          appSensors[sensor].currentState = LIMITS_UPPER_BOUND_REPORT_RESTART;
          return(0);
        }
        appSensors[sensor].currentState++;
        return 1;
      }
      //  Where it is supposed to be so make it responsive
      appSensors[sensor].currentState = 0;
  }
 
  return(0);
}

// ---- ----


  
float getTemp(){

  // random

  float temp = 35.0;
    
  return temp;
}


static bool gCMD = 0;
void sensorReport(void) {

  if ( gReporting ) {
    //
    int sensorValue = analogRead(A0);// read the input on analog pin 0:
    float voltage = sensorValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float temperature = getTemp();
    printData(voltage,temperature);
    //
  }

}



//   -------- ------------   -------- ------------

void itWorksLEDIndicator(bool cmd) {
  if ( itWorksLED ) {
    if ( cmd ) {
      //digitalWrite(LEDTestPin, HIGH);       // sets the digital pin 7 on
    } else {
      //digitalWrite(LEDTestPin, LOW);       // sets the digital pin 7 off
    }
  } else {
    //digitalWrite(LEDTestPin, LOW);       // sets the digital pin 7 off
  }
}

//  writePump
static bool gPumpDirection = false;  // state info
static uint8_t gPumpRate = 0;
// 
void writePump(bool pDirection, uint8_t rate) {
    gPumpDirection = pDirection;
    gPumpRate = rate;
    Serial.print("BDG >= gPumpDirection >> ");
    Serial.print(gPumpDirection);
    Serial.print(" >> ");
    Serial.print(rate);
}

// should be called in sequence
void writePumpInvert() {
  writePump(gPumpDirection,gPumpRate);
}

void writePumpRevert() {
  writePump(gPumpDirection,0);  
}


//  writeLight
static bool gLightState = false;  // state info
static uint8_t gLightIntensity = 0;
// 
void writeLight(bool state, uint8_t intensity) {
    gLightState = state;
    gLightIntensity = intensity;
//
    Serial.print("BDG >= writeLight >> ");
    Serial.print(gLightState);
    Serial.print(" >> ");
    Serial.print(gLightIntensity);
}

// should be called in sequence
void writeLightInvert() {
  writeLight(!gLightState,gLightIntensity);
}

void writeLightRevert() {
  writeLight(!gLightState,gLightIntensity);  
}


uint8_t gLightOnLevel = 0;
AlarmID_t gStartAlarmID;
AlarmID_t gStopAlarmID;
//
void lightON(void) {
  uint8_t intensity = (uint8_t)(gLightOnLevel);
  writeLight(false,intensity);
}

void lightOFF(void) {
  uint8_t intensity = (uint8_t)(0);
  writeLight(true,intensity);
}




void initLightAlarms(uint8_t level, uint8_t start_hour, uint8_t start_minute, uint8_t stop_hour, uint8_t stop_minute) {
  //
  gLightOnLevel = level;
  
  gStartAlarmID = Alarm.alarmRepeat(start_hour,start_minute,0, lightON);  //
  gStopAlarmID = Alarm.alarmRepeat(stop_hour,stop_minute,0,lightOFF);  //
  //
}

void cancelLightAlarms(void) {
  gLightOnLevel = 0;
  Alarm.disable(gStartAlarmID);
  Alarm.disable(gStopAlarmID);
  Alarm.free(gStartAlarmID);
  Alarm.free(gStopAlarmID);
  lightOFF();
}



//  writeHeat
static bool gHeatState = false;  // state info
static uint8_t gHeatIntensity = 0;
//
void writeHeat(bool state, uint8_t intensity) {
    gHeatState = state;
    gHeatIntensity = intensity;
    //
    Serial.print("BDG >= writeHeat >> ");
    Serial.print(gHeatState);
    Serial.print(" >> ");
    Serial.print(gHeatIntensity);
}

// should be called in sequence
void writeHeatInvert() {
  writeHeat(!gHeatState,gHeatIntensity);
}

void writeHeatRevert() {
  writeHeat(!gHeatState,gHeatIntensity);  
}



//   -------- ------------   -------- ------------


void printLimit(uint8_t sensor,float value) {
  int cc = checkLimitOnSensor(sensor,value);
  if ( cc != 0 ) {
    Serial.print(F("CRX"));
    if ( cc > 0 ) {
      Serial.print(cmdTable.preamble());
      Serial.print(F(",LIMIT: "));
      Serial.print(sensor);
      Serial.println(F(",LEVEL: HIGH "));          
    } else {
      Serial.print(cmdTable.preamble());
      Serial.print(F(",LIMIT: "));
      Serial.print(sensor);
      Serial.println(F(",LEVEL: LOW "));          
    }
  }
}



void printData(float OD,float temperature) {
    Serial.print(cmdTable.preamble());
    //
    Serial.print(F(",OD: "));
    Serial.print(OD,3); // print out the value you read:
    Serial.print(F(",Temperature: "));
    //
    Serial.println(temperature,3);

    if ( gCheckLimits ) {
      printLimit(SENSOR_OD_,OD);
      printLimit(SENSOR_TEMP_,temperature);
    }
}



void sendAllStateInfo(void) {
  //
  Serial.print("QRY");
  Serial.print(cmdTable.preamble());
  Serial.print(F(",SOO: "));
  Serial.print(gReporting); // print out the value you read
  //
  Serial.print(F(",SOO-inteval: "));
  Serial.print(gSampleInterval/1000);
  //
  Serial.print(F(",PumpDir: "));
  Serial.print(gPumpDirection);
  Serial.print(F(",PumpRate: "));
  Serial.print(gPumpRate);

  Serial.print(F(",Lamp: "));
  Serial.print(!(gLightState));
  Serial.print(F(",Lamp-level: "));
  Serial.print(gLightIntensity);

  Serial.print(F(",Heater: "));
  Serial.print(!(gHeatState));
  Serial.print(F(",Heater-level: "));
  Serial.print(gHeatIntensity);
  
  //
}

// // // // // // // // ---->


uint8_t hour_of(uint32_t minutesTime) {
  uint8_t hrs = minutesTime/60;
  return(hrs);
}

uint8_t  minutes_of(uint32_t minutesTime) {
   uint8_t minutes = minutesTime % 60;
   return(minutes);
}


// ---- ---- ---- ---- ---- ---- ----
void setupAppPins(void) {
  
}



// ---- ---- ---- ---- ---- ---- ----
void initCustomAppPins(void) {
  //
  //
}


// ---- ---- ---- ---- ---- ---- ----
// ---- ---- ---- ---- ---- ---- ----


void handleStateRequest(uint8_t psensor) {
 // ${TO DO}
 if ( psensor == 0 )  {
    sendAllStateInfo();
  }
}

void handleLimit(uint8_t psensor,float pHIGH,float pLOW) {
 // ${TO DO}
 setLimitOnSensor(psensor,pHIGH,pLOW);
}

void appSetTime(uint16_t pminutes,uint8_t pday,uint8_t pmonth,uint8_t pyear) {
 // ${TO DO}
  uint8_t theHour = hour_of(pminutes);
  uint8_t theMinutes = minutes_of(pminutes);
  //
  setTime(theHour,theMinutes,0,pday,pmonth,pyear);

}

void handleReportCmd(bool pstate,uint16_t pinterval,uint8_t psensor) {
  gReporting = pstate;
  gTasker.cancel(sensorReport);
  gSampleInterval =  (unsigned int)(pinterval*1000.0);
  if ( gReporting ) {
    gTasker.setInterval(sensorReport, gSampleInterval);
  }
}


void handleLedsCmd(bool pstate,uint8_t plevel,uint16_t pstart,uint16_t pstop) {
 // ${TO DO}
  if ( pstart == pstop ) {
    writeLight(pstate,plevel);
  } else {
    if ( !pstate ) {

      uint8_t start_hour = hour_of(pstart);
      uint8_t start_minute = minutes_of(pstart);
      uint8_t stop_hour = hour_of(pstop);
      uint8_t stop_minute = minutes_of(pstop);

      initLightAlarms(plevel, start_hour, start_minute, stop_hour, stop_minute);
  
      uint8_t cHr = hour();
      uint8_t cMin = minute();
  
      if ( cHr >= start_hour && cMin >= start_minute ) {
        if ( cHr <= stop_hour && cMin < stop_minute ) {
          writeLight(true,plevel);
        }
      }
      //
   } else {
      cancelLightAlarms();
    }
  }
}

void handlePumpCmd(bool pdirection,uint8_t plevel,uint16_t pstart,uint16_t pstop) {
 // ${TO DO}
 
   if ( pstart != pstop ) {
    if ( pstart == 0 ) {
      pstart = 1;
      pstop += 1;
    }
    writePump(pdirection,0);
    gPumpRate = plevel;
    gTasker.setTimeout(writePumpInvert,pstart);
    gTasker.setTimeout(writePumpRevert,pstop);
  } else {
    writePump(pdirection,plevel);
  }
}

void handleHeatCmd(bool state,uint8_t plevel) {
    writeHeat(state,plevel);
}

void handleStepperCmd(bool pdirection,uint8_t plevel,uint16_t psteps) {
 // ${TO DO}
}




void commandProcessor() {

  uint8_t cmdNum = cmdTable.cmdNumber();
  
  switch( cmdNum ) {
    case QUERY_STATE_: {
      handleStateRequest(cmdTable.unload_uint8(0));
      break;
    }
    case LIMIT_: {
      handleLimit(cmdTable.unload_uint8(0),cmdTable.unload_float(1),cmdTable.unload_float(2));
      break;
    }
    case SET_TIME_: {
      appSetTime(cmdTable.unload_uint16(0),cmdTable.unload_uint8(1),cmdTable.unload_uint8(2),cmdTable.unload_uint8(3));
      break;
    }
    case TOGGLE_READ_SENSORS_: {
      handleReportCmd(cmdTable.unload_bool(0),cmdTable.unload_uint16(1),cmdTable.unload_uint8(2));
      break;
    }
    case LAMP_: {
      handleLedsCmd(cmdTable.unload_bool(0),cmdTable.unload_uint8(1),cmdTable.unload_uint16(2),cmdTable.unload_uint16(3));
      break;
    }
    case PUMP_: {
      handlePumpCmd(cmdTable.unload_bool(0),cmdTable.unload_uint8(1),cmdTable.unload_uint16(2),cmdTable.unload_uint16(3));
      break;
    }
    case HEATER_: {
      handleHeatCmd(cmdTable.unload_bool(0),cmdTable.unload_uint8(1));
      break;
    }
    case STEPPER_: {
      handleStepperCmd(cmdTable.unload_bool(0),cmdTable.unload_uint8(1),cmdTable.unload_uint16(2));
      break;
    }
    default: {
      cmdTable.reset(false);
    }
  }
  
  cmdTable.reset();
  //
}




void setup()
{
  setupAppPins();
  //
  Serial.begin(9600); //Baud rate: 9600
  while (!Serial) ; // wait for Arduino Serial Monitor
  //
  setTime(8,0,0,1,1,19); // set time to Saturday 8:29:00am Jan 1 2019

  cmdTable.reportReadState("setup");
  initCustomAppPins();
  //
}

void serialEvent() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    cmdTable.addChar(c); // get the new byte:
  }
}


//  -----------------------------


// -----------------------------


void loop()
{

  if ( !cmdTable.error() ) {
    Serial.print("ACK");
    Serial.println(cmdTable.preamble());
  } else {
    Serial.print("ERR");
    Serial.println(cmdTable.preamble());
  }
  
  gTasker.loop();
  Alarm.delay(0);
 
}
