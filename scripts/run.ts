import { defineModel } from "./modeller.js";
import { parse } from "./parsetsp.js";
import { parseSolution, serializeSolution } from "./solutionParserTSP.js";
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

let seed = params.randomSeed == undefined ? 0 : params.randomSeed;

let instance = undefined;
if (problemType == "TSP") {
    instance = parse(instanceFilename, {});
} else if (problemType == "CVRP") {

} else if (problemType == "VRP-TW") {

}

if (instance == undefined) {
    console.log(`Unrecognized problem type: ${problemType}`);
    exit(100);
}


let heuristics = spawn('./build/Heuristic', [configFilename, instanceFilename, `${seed}`], { windowsHide: true });
process.on('exit', () => { heuristics.kill(); });
let heuristicsPipe = readline.createInterface({ input: heuristics.stdout, terminal: false, crlfDelay: Infinity });
let heuristicsDebugPipe = readline.createInterface({ input: heuristics.stderr, terminal: false, crlfDelay: Infinity });
heuristicsDebugPipe.on('line', async line => { console.log("strerr: " + line); });


if (problemType == "TSP") {
    let [model, vars] = defineModel(instance, instanceFilename, 1);

    heuristicsPipe.on('line', async line => {
        let solution = parseSolution(line, vars, instance);
        solver.sendSolution(solution);
    });

    solver.on('solution', async (msg: CP.SolutionEvent) => {
        let solution_string = serializeSolution(msg.solution, vars, instance);
        heuristics.stdin.write(`${solution_string}\n`);
    });

    solver.solve(model, params);
} else if (problemType == "CVRP") {

} else if (problemType == "VRP-TW") {

}



