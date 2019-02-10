var fs = require('fs')


var cmdDefFile = process.argv[2];
var outputDir = process.argv[3];

if ( cmdDefFile == undefined ) {
    console.error("Err: file name required")
    process.exit(0)
}



// // //
if ( outputDir == undefined ) {
    console.error("Err: an output directory is required")
    console.error("Usage: node <this file> input-def-file output-dir")
    process.exit(0)
}


function outputCaseHandler(body) {

    var str = `
void commandProcessor() {

  uint8_t cmdNum = cmdTable.cmdNumber();
  ${body}
  cmdTable.reset();
}
        `

   return(str);
}


function writeSketchFile(sketchCmdDefs,sketchCases,functionDefs,sensorDefs) {
    //
    var output = sketchCmdDefs;
    output += "\n\n"
    output += sensorDefs
    output += "\n\n"
    output += functionDefs
    output += "\n\n"
    output += outputCaseHandler(sketchCases);
    //
    fs.writeFileSync("sketchFile.txt",output,"ascii")
    //
}


function writeJSFile(jsCmdOutput,jsSensorOutput) {
    //
    var output = jsCmdOutput;
    output += '\n'
    output += jsSensorOutput
    //
    fs.writeFileSync("mcuConfig.js",output,"ascii")
    //
}



var cmdData = {};

try {
    //
    cmdData = fs.readFileSync(cmdDefFile);
    //
    if ( cmdData == undefined ) {
        console.error("Err: file not known")
        process.exit(0)
    }
    //
    cmdData = JSON.parse(cmdData.toString());
    //
} catch(e) {
    console.error(e.toString())
    process.exit(0)
}

// // // // // // // // // // // //
var commands = cmdData.commands;
var valueManagers = cmdData["value-management"];
var valueMap = cmdData["value-map"];
var sensors = cmdData.sensors;
//
//

var cmdTemplate = "${preamble},${cmd},${par1},${par2},${par3},${par4}";

function sketchCaseBody(cmdInfo,indent) {
    //
    var caller = cmdInfo.sketchHandler
    var callerParts = caller.split('(');

    var callInstance = indent + callerParts[0] + '(';
    var callPars = callerParts[1].replace(')','');
    callPars = callPars.split(',')

    var sep = ''
    for ( var i = 0; i < callPars.length; i++ ) {
        //
        var parType = callPars[i].trim();
        parType = parType.split(' ');

        var pp = parType[0].replace('_t','')

        callInstance += sep + `cmdTable.unload_${pp}(${i})`;
        sep = ','
    }

    callInstance += ');';

    return(callInstance)
}



function fixupParameters(sketchHandler) {
    var fdefParts = sketchHandler.split('(')

    var handlerDef = fdefParts[0] + '('
    var pars = fdefParts[1].replace(')','');
    pars = pars.split(',');
    for ( var i = 0; i < pars.length; i++ ) {
        var par = pars[i];
        par = par.trim();
        var pparts = par.split(' ');
        pparts[1] = 'p' + pparts[1];
        par = pparts.join(' ');
        pars[i] = par
    }

    handlerDef += pars.join(',');
    handlerDef += ')'

    return(handlerDef)
}


function generateCommands(cmdMap) {
    var sketchDefs = "";
    var sketchCases = "\n\tswitch( cmdNum ) {"
    var functionDefs = "\n";
    var JSDefs = "const commandNumMap = {";

    var index = 0;
    var sep = ''
    for ( var cmd in cmdMap ) {
        //
        var capsCMD = cmd.toUpperCase() + '_';
        while ( capsCMD.indexOf('-') >= 0 ) {
            capsCMD = capsCMD.replace('-','_');
        }

        sketchDefs += `\n#define ${capsCMD} ${index}`;

        sketchCases += `\n\t\tcase ${capsCMD}: {`;
        sketchCases += sketchCaseBody(cmdMap[cmd],'\n\t\t\t');
        sketchCases += `\n\t\t\tbreak;\n\t\t}`;

        var handlerParFixUp = fixupParameters(cmdMap[cmd].sketchHandler)
        functionDefs += "\nvoid " + handlerParFixUp + ' {'
        functionDefs += "\n // ${TO DO}";
        functionDefs += "\n}";

        JSDefs += sep + `\n\t"${cmd}" : ${index}`;
        index++;
        sep = ',';
        //
    }

    JSDefs += "\n}\n"

    sketchCases += "\n\t}"

    functionDefs += "\n\n"

    return([sketchDefs,JSDefs,sketchCases,functionDefs])
}



function generateSensors(sensors) {
    //
    var sketchDefs = ''
    var jsDefs = 'var sensorMap = {'

    var sep = ''
    for ( var i = 0; i < sensors.length; i++ ) {
        sketchDefs += `\n#define SENSOR_${sensors[i]}_ ${i}`;
        jsDefs += sep + `\n\t"${sensors[i]}" : ${i}`;
        sep = ','
    }

    jsDefs += '\n}';

    return([sketchDefs,jsDefs])
}


var [sketch_defs,js_defs,sketchCases,functionDefs] = generateCommands(commands)

var [sketch_sensors,js_sensors] = generateSensors(sensors)

writeSketchFile(sketch_defs,sketchCases,functionDefs,sketch_sensors)
//

writeJSFile(js_defs,js_sensors)
//




