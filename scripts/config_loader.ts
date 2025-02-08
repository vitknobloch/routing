import * as CP from '@scheduleopt/optalcp';
import { readFile } from './utils.js';

export function readConfig(filename: string, params: CP.BenchmarkParameters): [CP.BenchmarkParameters, string] {
    let config_json = JSON.parse(readFile(filename));
    let problem_type = config_json.problem;
    let lns_count = config_json.optal.LNS;
    let fds_count = config_json.optal.FDS;
    let neighborhoodStrategy = 1;
    if ("neighborhoodStrategy" in config_json.optal)
        neighborhoodStrategy = config_json.optal.neighborhoodStrategy;

    let worker_count = lns_count + fds_count;
    params.nbWorkers = worker_count;
    let workers_params = [];
    for (let i = 0; i < lns_count; i++) {
        let worker_params: CP.WorkerParameters = {};
        worker_params.searchType = "LNS"
        worker_params._lnsNeighborhoodStrategy = neighborhoodStrategy;
        workers_params.push(worker_params);
    }
    for (let i = 0; i < fds_count; i++) {
        let worker_params: CP.WorkerParameters = {};
        worker_params.searchType = "FDS"
        workers_params.push(worker_params);
    }
    params.workers = workers_params;

    return [params, config_json.problem];
}