/* eslint-disable node/no-missing-require */
'use strict';

const fs = require('fs');
const SerialPort = require('serialport');


// could be required in.. but this is for a quick example.

const commandNumMap = {
    "query-state" : 0,
    "limit" : 1,
    "set-time" : 2,
    "toggle-read-sensors" : 3,
    "lamp" : 4,
    "pump" : 5,
    "heater" : 6,
    "stepper" : 7
}

var sensorMap = {
    "ALL" : 0,
    "OD" : 1,
    "Temperature" : 2
}


// ---- ---- ---- ---- ---- ----
// ---- ---- ---- ---- ---- ----
const g_tickIndicator = "TICK"
const g_errIndicator = "ERR"
const g_dbgIndicator = "DBG"
const g_crxIndicator = "CRX"
const g_queryIndicator = "QRY"
const g_ackIndicator = "ACK"
// ---- ---- ---- ---- ---- ----
// ---- ---- ---- ---- ---- ----


// ----------------  CONFIG ----------------
// // // // // // // // // // // // // // //

var confFile = process.argv[2];
var config = confFile ? fs.readFileSync(confFile).toString() : undefined;
if ( config === undefined ) {
    console.log("Cannot run without configuration")
    process.exit(0)
}

config = JSON.parse(config);


// ----------------   ----------------
// // // // // // // // // // // // // // //


var scriptFile = process.argv[3];
var script = scriptFile ? fs.readFileSync(scriptFile).toString() : undefined;
if ( script === undefined ) {
    console.log("Cannot run without script")
    process.exit(0)
}


script = JSON.parse(script);


// ----------------   ----------------
// // // // // // // // // // // // // // //


function lineParserHandler(str) {

    console.log("lineParserHandler->" + str)

    var msg = "";
    var init = null;

    str = str.toString();

    if ( str.indexOf(g_tickIndicator) == 0 ) {
        return;
    }

    if ( str.indexOf(g_errIndicator) == 0 ) {
        return;
    }

    if (  str.indexOf(g_dbgIndicator) == 0 ) {
        return;
    }

    if ( str.indexOf(g_crxIndicator) == 0 ) {
        return;
    }

    if ( str.indexOf(g_queryIndicator) == 0 ) {
        return;
    }

    if ( str.indexOf(g_ackIndicator) == 0 ) {
        return;
    }
}



function stringToPort(thisPort,str) {
    console.log(str)
    if ( thisPort ) {
        thisPort.write(str + '\n');
    }
}

function isInt(pValue) {
    if ( typeof pValue === "number" ) return(true)
    return ( (typeof pValue === "string") && !(isNaN(pValue)) && (pValue.indexOf('.') < 0) )
}

var valueManager = config["value-management"]
var valueMap = config["value-map"]
var valueConversions = config["value-conversions"]
//
//very specific to this example
for ( var ky in valueConversions ) {
    valueConversions[ky][0] = eval(valueConversions[ky][0])
}

function run_example_script(uPort,mcu_i) {

    var cmdToMCU = mcu_i + ',';

    var prog = script.script;
    var when = 2000; // very simple time delayed script
    var delta = 200
    prog.forEach( cmd => {
                    //
                    var cmdPars = {}
                    var thisCmd = cmdToMCU;
console.log(cmd)
                    var cmdNum = commandNumMap[cmd.cmd]
                    thisCmd += cmdNum + ',';
                    var pars = Object.keys(cmd.parameters);

                    for ( var i = 0; i < pars.length; i++ ) {
                        //
                        var addParam = true
                        //
                        var par = pars[i]
                        var pValue = cmd.parameters[par]

                        if ( valueManager[par] !== undefined ) {
                            // ----
                            var vm = valueManager[par]
                            var convert = vm.in;
                            if ( convert !== undefined && convert.length ) {
                                 var converter = null
                                 if ( convert == par ) {
                                     converter = valueConversions[vm.in]
                                 } else {
                                     var key = vm.out.substr("conv-".length)
                                     converter = valueConversions[key]
                                 }
                                 //
                                 pValue = (converter[0])(pValue)
                                 //
                                 if ( converter.length > 1 ) {
                                     //
                                     var depositer = converter[1]
                                     depositer = depositer.split(',');
                                     if ( depositer[0] == '+' ) {  // an op
                                         cmdPars[depositer[1]] += parseInt(pValue)
                                         addParam = false
                                     }
                                 }

                            } else {
                                if ( vm.out == "sensorMap" ) {
                                    pValue = sensorMap[pValue]
                                }
                            }
                        }
                        //
                        if ( valueMap[par] !== undefined ) {
                            var vm = valueMap[par]
                            pValue = pValue.toLowerCase();
                            if ( vm[pValue] !== undefined ) {
                                pValue = vm[pValue];
                            }
                        }
                        //
                        if ( addParam ) {
                             if ( isInt(pValue) ) pValue = parseInt(pValue);
                             cmdPars[par] = pValue
                        }
                    }
                    //
                    var sep = ''
                    for ( var p in cmdPars ) {
                        thisCmd += sep + cmdPars[p]
                        sep = ','
                    }

                    setTimeout( ((cmdstr,thisPort) => { return(() => { stringToPort(thisPort,cmdstr); }) })(thisCmd,uPort), when );
                    when += delta

                })

}


// // // // // // // // // // // // // // //

const uartsAll = ( typeof script.uarts !== 'undefined' ) ? script.uarts : undefined;

//------------ ------------ ------------ ------------ ------------ ----------

const Readline = SerialPort.parsers.Readline;

// Create a uart port interface
var uartPorts = [];
var gMCUPortMap = {}
var gMCUPreambles = {}

if ( typeof uartsAll !== 'undefined' ) {  // use Serial Port class. Baud rate in config
    uartsAll.forEach( uarti => {
                         //
                         var uartPort = new SerialPort(uarti.port, {
                           baudRate: uarti.baud
                         });
                         //
                         uartPorts.push(uartPort);
                         //

                         //  dataPortReady -- because message can be received before a port is available
                         uartPort.on('open',
                                     ((uPort,mcu_i) => {
                                          return(() => {
                                                     // This point enable any communication with the server, etc.
                                             console.log("data port open: " + mcu_i)
                                             uPort.write('\n');
                                             run_example_script(uPort,mcu_i);

                                         })
                                      })(uartPort,uarti.mcu));

                         const lineParser = new Readline();

                         //
                         uartPort.pipe(lineParser);   // configure to use the line parser
                         lineParser.on('data', lineParserHandler);    // CALL lineParserHandler

                         // log received data
                         uartPort.on('data', (data) => {
                                         // console.log("data event")
                                     });


                     })
}



//
//------------ ------------ ------------ ------------ ------------ ----------
//------------ ------------ ------------ ------------ ------------ ----------


run_example_script(null,"fake");




