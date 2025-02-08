import { TspVars } from "./modeller.js";
import { ParseResult } from "./parsetsp.js";
import * as CP from '@scheduleopt/optalcp';

export function parseSolution(solution_string: string, vars: TspVars, instance: ParseResult): CP.Solution {
    let solution = new CP.Solution;
    let solution_json = JSON.parse(solution_string)
    solution.setObjective(solution_json.objective);
    for (let node of solution_json.routes[0].nodes.slice(1)) {
        solution.setValue(vars.visits[node.idx], node.start, node.end);
    }
    return solution;
}

export function serializeSolution(solution: CP.Solution, vars: TspVars, instance: ParseResult): string {
    let path: { idx: number, start_time: number, end_time: number }[] = [];
    for (let i = 0; i < instance.nbNodes; i++) {
        const v = vars.visits[i];
        if (solution.isAbsent(v)) {
            console.log(`Node ${v.getName()} unvisited in solution`);
            continue;
        }
        path.push({ idx: i, start_time: solution.getStart(v)!, end_time: solution.getEnd(v)! })
    }

    path.sort((a, b) => {
        if (a.start_time < b.start_time)
            return -1;
        else
            return 1;
    });

    let objective = solution.getObjective();
    let time = solution.getStart(vars.visits[0]);
    let length = objective;

    let solution_object = {
        travel_time_sum: length,
        end_time_sum: time,
        objective: objective,
        routes: [
            {
                travel_time: length,
                demand: -1,
                end_time: time,
                route_nodes: [{ idx: 0, start_time: 0, end_time: 0 }].concat(path)
            }
        ]
    };
    return JSON.stringify(solution_object);
}