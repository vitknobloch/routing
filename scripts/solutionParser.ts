import { assert } from "node:console";
import { TspVars, CvrpVars } from "./modeller.js";
import { ParseResult } from "./parser.js";
import * as CP from '@scheduleopt/optalcp';

export function parseSolutionTSP(solution_string: string, vars: TspVars, instance: ParseResult): CP.Solution {
    let solution = new CP.Solution;
    let solution_json = JSON.parse(solution_string)
    solution.setObjective(solution_json.objective);
    let prev_node = 0;
    for (let node of solution_json.routes[0].route_nodes.slice(1)) {
        //console.log(`${node.idx} ${node.start_time} ${node.end_time} ${instance.transitionMatrix[prev_node][node.idx]}`);
        solution.setValue(vars.visits[node.idx], node.start_time, node.end_time);
        prev_node = node.idx;
    }

    return solution;
}

export function serializeSolutionTSP(solution: CP.Solution, vars: TspVars, instance: ParseResult): string {
    let path: { idx: number, start_time: number, end_time: number }[] = [];
    path.push({ idx: 0, start_time: 0, end_time: 0 });
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
                route_nodes: path
            }
        ]
    };
    return JSON.stringify(solution_object);
}

export function parseSolutionCVRP(solution_string: string, vars: CvrpVars, instance: ParseResult): CP.Solution {
    let solution = new CP.Solution;
    let solution_json = JSON.parse(solution_string);
    assert(solution_json.routes.length == instance.nbVehicles, `Vehicles present: ${solution_json.routes.length}, expected ${instance.nbVehicles}`)
    solution.setObjective(solution_json.objective);
    for (let i = 0; i < instance.nbVehicles!; i++) {
        const route = solution_json.routes[i];
        for (const node of route.route_nodes.slice(1, -1)) {
            solution.setValue(vars.visits[node.idx - 1][i], node.start_time, node.end_time);
        }
        const node = route.route_nodes.at(-1);
        solution.setValue(vars.lasts[i], node.start_time, node.end_time);
    }

    return solution;
}

export function serializeSolutionCVRP(solution: CP.Solution, vars: CvrpVars, instance: ParseResult): string {
    let routes = [];
    let nbCustomers = instance.nbNodes - 1;

    for (let r = 0; r < instance.nbVehicles!; r++) {
        let path: { idx: number, start_time: number, end_time: number }[] = [];
        path.push({ idx: 0, start_time: 0, end_time: 0 });
        for (let i = 0; i < nbCustomers; i++) {
            const v = vars.visits[i][r];
            if (!solution.isAbsent(v)) {
                path.push({ idx: i + 1, start_time: solution.getStart(v)!, end_time: solution.getEnd(v)! });
            }
        }
        path.push({ idx: 0, start_time: solution.getStart(vars.lasts[r])!, end_time: solution.getEnd(vars.lasts[r])! });

        path.sort((a, b) => {
            if (a.start_time < b.start_time)
                return -1;
            else
                return 1;
        });

        let travel_time = 0;
        let demand = 0;
        let end_time = path.at(-1)!.end_time;
        for (let i = 0; i < path.length - 1; i++) {
            travel_time += instance.transitionMatrix[path[i].idx][path[i + 1].idx];
            demand += instance.demands![path[i].idx];
        }

        routes.push({
            travel_time: travel_time,
            demand: demand,
            end_time: end_time,
            route_nodes: path
        });
    }

    let travel_time_sum = 0;
    let end_time_sum = 0;
    let objective = solution.getObjective();
    for (const route of routes) {
        travel_time_sum += route.travel_time;
        end_time_sum += route.end_time;
    }

    let solution_object = {
        travel_time_sum: travel_time_sum,
        end_time_sum: end_time_sum,
        objective: objective,
        routes: routes
    };

    return JSON.stringify(solution_object);
}
