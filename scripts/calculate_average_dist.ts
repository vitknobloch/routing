import { defineModelCVRP, defineModelTSP, defineModelVRPTW } from "./modeller.js";
import { parseTspLib, parseSolomon } from "./parser.js";
import { parseSolutionCVRP, parseSolutionTSP, serializeSolutionCVRP, serializeSolutionTSP } from "./solutionParser.js";
import * as CP from '@scheduleopt/optalcp';
import { spawn } from 'child_process';
import { readConfig } from "./config_loader.js";
import * as readline from 'node:readline';
import { exit } from "node:process";
import * as fs from 'node:fs';
import * as path from 'node:path';

let params: CP.BenchmarkParameters = {
    usage: "Usage: node tsp.js [OPTIONS] INPUT_FILE [INPUT_FILE2] .."
};
let restArgs = CP.parseSomeBenchmarkParameters(params);
let configFilename = restArgs[0];
let instanceFilename = restArgs[1];
let logFilename = restArgs[2];

let [params_, problemType] = readConfig(configFilename, params);
params = params_;

let solver = new CP.Solver;
solver.setMaxListeners(1000);

let seed = params.randomSeed == undefined ? 0 : params.randomSeed;

let instance = undefined;
if (problemType == "TSP") {
    instance = parseTspLib(instanceFilename, {});
} else if (problemType == "CVRP") {
    instance = parseTspLib(instanceFilename, {});
} else if (problemType == "VRP-TW") {
    instance = parseSolomon(instanceFilename, {});
}

if (instance == undefined) {
    console.log(`Unrecognized problem type: ${problemType}`);
    exit(100);
}

let maxDist = instance.transitionMatrix.reduce((acc, val) => { return Math.max(acc, val.reduce((acc, val) => { return Math.max(acc, val); }, 0)); }, 0);
fs.appendFileSync(logFilename, `${problemType} ${path.basename(instanceFilename)} ${maxDist}\n`);