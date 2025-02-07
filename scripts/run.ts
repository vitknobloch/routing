import { defineModel } from "./modeller";
import { parse } from "./parsetsp";
import { parseSolution, serializeSolution } from "./solutionParser";
import * as CP from '@scheduleopt/optalcp';
import { spawn } from 'child_process';
import * as readline from 'node:readline';

let params: CP.BenchmarkParameters = {
    usage: "Usage: node tsp.js [OPTIONS] INPUT_FILE [INPUT_FILE2] .."
};
let restArgs = CP.parseSomeBenchmarkParameters(params);

let filename = restArgs[0];
let logFilename = restArgs[1];

let solver = new CP.Solver;

let seed = params.randomSeed == undefined ? 0 : params.randomSeed;

let instance = parse(filename, {});

let heuristics = spawn('./build/bin/TSP-Heuristic', [filename, "log.txt", `${seed}`], { windowsHide: true });
process.on('exit', () => { heuristics.kill(); });
let heuristicsPipe = readline.createInterface({ input: heuristics.stdout, terminal: false, crlfDelay: Infinity });
let heuristicsDebugPipe = readline.createInterface({ input: heuristics.stderr, terminal: false, crlfDelay: Infinity });
heuristicsDebugPipe.on('line', async line => { console.log("strerr: " + line); });

let [model, vars] = defineModel(instance, filename, 1);

heuristicsPipe.on('line', async line => {
    let solution = parseSolution(line, vars, instance);
    solver.sendSolution(solution);
});

solver.on('solution', async (msg: CP.SolutionEvent) => {
    let solution_string = serializeSolution(msg.solution, vars, instance);
    heuristics.stdin.write(`${solution_string}\n`);
});

solver.solve(model, params);