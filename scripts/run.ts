import { defineModelCVRP, defineModelTSP, defineModelVRPTW } from "./modeller.js";
import { parseTspLib, parseSolomon } from "./parser.js";
import { parseSolutionCVRP, parseSolutionTSP, parseSolutionVRPTW, serializeSolutionCVRP, serializeSolutionTSP, serializeSolutionVRPTW } from "./solutionParser.js";
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

let heuristics = spawn('./build/Heuristic', [configFilename, instanceFilename, logFilename, `${seed}`], { windowsHide: true });
process.on('exit', () => { heuristics.kill(); });
let heuristicsPipe = readline.createInterface({ input: heuristics.stdout, terminal: false, crlfDelay: Infinity });
let heuristicsDebugPipe = readline.createInterface({ input: heuristics.stderr, terminal: false, crlfDelay: Infinity });
heuristicsDebugPipe.on('line', async line => { console.log("strerr: " + line); });

let optimalSolution: Promise<CP.SolveResult>;

if (problemType == "TSP") {
    let [model, vars] = defineModelTSP(instance, instanceFilename, 1);

    heuristicsPipe.on('line', async line => {
        let solution = parseSolutionTSP(line, vars, instance);
        solver.sendSolution(solution);
    });

    solver.on('solution', async (msg: CP.SolutionEvent) => {
        let solution_string = serializeSolutionTSP(msg.solution, vars, instance);
        heuristics.stdin.write(`${solution_string}\n`);
    });

    if (params.nbWorkers != 0)
        solver.solve(model, params).then(_ => { if (!heuristics.killed) heuristics.kill(); });
} else if (problemType == "CVRP") {
    let [model, vars] = defineModelCVRP(instance, instanceFilename, 1);

    heuristicsPipe.on('line', async line => {
        let solution = parseSolutionCVRP(line, vars, instance);
        //console.log(line);
        solver.sendSolution(solution);
    });

    solver.on('solution', async (msg: CP.SolutionEvent) => {
        let solution_string = serializeSolutionCVRP(msg.solution, vars, instance);
        heuristics.stdin.write(`${solution_string}\n`);
    });

    if (params.nbWorkers != 0)
        solver.solve(model, params).then(_ => { if (!heuristics.killed) heuristics.kill(); });
} else if (problemType == "VRP-TW") {
    let [model, vars] = defineModelVRPTW(instance, instanceFilename);

    heuristicsPipe.on('line', async line => {
        let solution = parseSolutionVRPTW(line, vars, instance);
        solver.sendSolution(solution);
    });

    solver.on('solution', async (msg: CP.SolutionEvent) => {
        let solution_string = serializeSolutionVRPTW(msg.solution, vars, instance);
        heuristics.stdin.write(`${solution_string}\n`);
    });

    if (params.nbWorkers != 0)
        solver.solve(model, params).then(_ => { if (!heuristics.killed) heuristics.kill(); });
}




