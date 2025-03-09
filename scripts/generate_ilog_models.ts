import { defineModelCVRP, defineModelTSP, defineModelVRPTW } from "./modeller.js";
import { parseTspLib, parseSolomon } from "./parser.js";
import { parseSolutionCVRP, parseSolutionTSP, serializeSolutionCVRP, serializeSolutionTSP } from "./solutionParser.js";
import * as CP from '@scheduleopt/optalcp';
import { spawn } from 'child_process';
import { readConfig } from "./config_loader.js";
import * as readline from 'node:readline';
import { exit } from "node:process";

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

if (problemType == "TSP") {
    let [model, vars] = defineModelTSP(instance, instanceFilename, 1);
    CP.benchmark((input) => { return model; }, [instanceFilename], params);
} else if (problemType == "CVRP") {
    let [model, vars] = defineModelCVRP(instance, instanceFilename, 1);
    CP.benchmark((input) => { return model; }, [instanceFilename], params);
} else if (problemType == "VRP-TW") {
    let [model, vars] = defineModelVRPTW(instance, instanceFilename);
    CP.benchmark((input) => { return model; }, [instanceFilename], params);
}