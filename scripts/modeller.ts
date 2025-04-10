import { ParseResult } from "./parser.js";
import * as CP from '@scheduleopt/optalcp';
import { makeModelName } from "./utils.js";
import { strict as assert } from 'assert';

export type TspVars = {
    visits: CP.IntervalVar[]
}

export type CvrpVars = {
    visits: CP.IntervalVar[][],
    lasts: CP.IntervalVar[],
}

export function defineModelTSP(instance: ParseResult, filename: string, visit_duration: number): [CP.Model, TspVars] {
    let model = new CP.Model(makeModelName('tsp', filename));
    let visits = Array.from({ length: instance.nbNodes }, (_, i) => model.intervalVar({ length: visit_duration, name: `N_${i}` }));
    let sequence = model.sequenceVar(visits);
    model.noOverlap(sequence, instance.transitionMatrix);
    model.constraint(model.max(visits.slice(1).map(visit => visit.end())).le(visits[0].start()));
    for (let i = 1; i < instance.nbNodes; i++) {
        visits[i].setStartMin(visit_duration + instance.transitionMatrix[0][i]);
    }
    visits[0].start().minus(instance.nbNodes * visit_duration).minimize();

    return [model, { visits }];
}

let breakDirectionSymmetry = false;
let breakVehicleSymmetry = false;

export function defineModelCVRP(instance: ParseResult, filename: string, visit_duration: number): [CP.Model, CvrpVars] {
    assert(instance.depots !== undefined, "Depots are not defined in the input file");
    assert(instance.capacity !== undefined, "Capacity is not defined in the input file");
    assert(instance.demands !== undefined, "Demands are not defined in the input file");
    assert(instance.nbVehicles !== undefined, "Number of vehicles is not defined");

    // The data format supports multiple depots, but we don't have any data files with multiple depots:
    assert(instance.depots.length == 1, "Multiple depots are not supported yet");
    // In the symmetry breaking, we assume that the depot is the first node:
    assert(instance.depots[0] == 0, "Depot must be the first node");

    let nbCustomers = instance.nbNodes - 1;
    let customerMatrix = instance.transitionMatrix.slice(1).map(row => row.slice(1));

    // Compute the maximum distance in the matrix:
    let maxDistance = 0;
    for (let i = 0; i < instance.nbNodes; i++)
        for (let j = 0; j < instance.nbNodes; j++)
            maxDistance = Math.max(maxDistance, instance.transitionMatrix[i][j]);
    // The horizon doesn't seem to be needed. But let's use it anyway:
    let horizon = maxDistance * (instance.nbNodes + instance.nbVehicles);

    let model = new CP.Model(makeModelName('cvrp', filename));
    // For each customer, we have an array of potential visits by the vehicles:
    let visits: CP.IntervalVar[][] = Array.from({ length: nbCustomers }, () => []);
    // For each behicle we have the returning visit to the depot:
    let lasts: CP.IntervalVar[] = [];
    // For each vehicle, the time of the last visit:
    let endTimes: CP.IntExpr[] = [];
    // For each vehicle, we compute the max index of a customer served.
    // Used only for symmetry-breaking.
    let maxServed: CP.IntExpr[] = [];
    // Usage of each vehicle (how much capacity is used):
    let vehicleUsage: CP.IntExpr[] = [];

    for (let v = 0; v < instance.nbVehicles; v++) {
        // Visits done by the vehicle v:
        let myVisits = Array.from({ length: nbCustomers }, (_, i) =>
            model.intervalVar({ length: visit_duration, name: `V_${v + 1}_${i + 1}`, optional: true })
        );
        // Add myVisits to the visits array:
        for (let i = 0; i < nbCustomers; i++)
            visits[i].push(myVisits[i]);

        let sequence = model.sequenceVar(myVisits);
        model.noOverlap(sequence, customerMatrix);

        // Constraints for the depot:
        let last = model.intervalVar({ length: 0, name: `last_${v + 1}`, end: [0, horizon] });
        for (let i = 0; i < nbCustomers; i++) {
            // We don't model the initial depot visit at all. It is known to be at time 0.
            // Instead, we increase startMin of all the visits by the transition matrix value:
            myVisits[i].setStartMin(instance.transitionMatrix[0][i + 1]);
            // The return to depot must be after all the other visits and respect the transition matrix:
            model.endBeforeStart(myVisits[i], last, instance.transitionMatrix[i + 1][0]);
        }
        endTimes.push(last.end());
        lasts.push(last);

        // Capacity of the vehicle cannot be exceeded:
        let used = model.sum(myVisits.map((itv, i) => itv.presence().times(instance.demands![i + 1])));
        model.constraint(used.le(instance.capacity));
        vehicleUsage.push(used);

        // Compute the max index of a served customer as:
        //    min_i { (i+1) * myVisits[i].presence() }
        // There is +1 to distinguish between serving no customer (value 0) and
        // serving just the customer with index 0 (value 1).
        let maxServedCustomer = model.max(myVisits.map((itv, i) => itv.presence().times(i + 1)));
        maxServed.push(maxServedCustomer);

        if (instance.hasDirectionSymmetry && breakDirectionSymmetry) {
            // Let's compute the time of the customer with the max index served:
            //   sum_i { myVisits[i].start() * (maxServedCustomer == i+1) }
            // Here we use boolean expression maxServedCustomer == i+1 as 0/1 integer expression.
            let timeOfMaxServedCustomer =
                model.sum(myVisits.map((itv, i) => itv.start().times(maxServedCustomer.eq(i + 1))));
            // The route taken in the reverse order is also a solution.
            // So we may insist that the time of this visit is in the first half of the route:
            model.constraint(timeOfMaxServedCustomer.times(2).le(last.end()));
        }
    }

    for (let i = 0; i < nbCustomers; i++) {
        // Every customer must be visited exactly once:
        //    sum_j visits[i][j] == 1
        // We don't need alternative constraint.
        model.constraint(model.sum(visits[i].map(vis => vis.presence())).eq(1));
    }

    // All the demands must be satisfied by some vehicle. Therefore the sum of
    // their usage must be equal to the total demand.  It is a redundant
    // constraint. It allows the solver to see a problem when some vehicles are
    // underused and there is no way to satisfy the remaining demands by the
    // remaining vehicles.
    let totalDemand = instance.demands.slice(1).reduce((a, b) => a + b, 0);
    model.constraint(model.sum(vehicleUsage).eq(totalDemand));

    if (breakVehicleSymmetry) {
        // The values of the maxServed variables must be increasing with the vehicle number.
        // For the case the two vehicles are not used at all, i.e., both maxServed
        // are 0, there is max2(1) on the right side.
        for (let c = 1; c < instance.nbVehicles; c++)
            model.constraint(maxServed[c - 1].le(maxServed[c].max2(1)));
        // Customer with the biggest index must be served by the last vehicle.
        // Customer with the second biggest index must be served by the last or the second last vehicle.
        // etc.
        for (let i = nbCustomers - 1; i > nbCustomers - instance.nbVehicles; i--) {
            // How many possible vehicles can serve this customer:
            let nbPossibleVehicles = nbCustomers - i;
            let nbForbiddenVehicles = instance.nbVehicles - nbPossibleVehicles;
            for (let v = 0; v < nbForbiddenVehicles; v++)
                visits[i][v].makeAbsent();
        }
    }


    model.minimize(model.sum(endTimes).minus(nbCustomers * visit_duration));


    return [model, { visits, lasts }];
}

export function defineModelVRPTW(instance: ParseResult, filename: string): [CP.Model, CvrpVars] {
    assert(instance.depots !== undefined, "Depots are not defined in the input file");
    assert(instance.capacity !== undefined, "Capacity is not defined in the input file");
    assert(instance.demands !== undefined, "Demands are not defined in the input file");
    assert(instance.nbVehicles !== undefined, "Number of vehicles is not defined");
    assert(instance.ready_times !== undefined, "Ready times are not defined in input file");
    assert(instance.due_times !== undefined, "Ready times are not defined in input file");
    assert(instance.service_times !== undefined, "Ready times are not defined in input file");

    // The data format supports multiple depots, but we don't have any data files with multiple depots:
    assert(instance.depots.length == 1, "Multiple depots are not supported yet");
    // In the symmetry breaking, we assume that the depot is the first node:
    assert(instance.depots[0] == 0, "Depot must be the first node");

    let nbCustomers = instance.nbNodes - 1;
    let customerMatrix = instance.transitionMatrix.slice(1).map(row => row.slice(1));

    // Compute the maximum distance in the matrix:
    let maxDistance = 0;
    for (let i = 0; i < instance.nbNodes; i++)
        for (let j = 0; j < instance.nbNodes; j++)
            maxDistance = Math.max(maxDistance, instance.transitionMatrix[i][j]);
    // The horizon doesn't seem to be needed. But let's use it anyway:
    let horizon = maxDistance * (instance.nbNodes + instance.nbVehicles);

    let model = new CP.Model(makeModelName('cvrp', filename));
    // For each customer, we have an array of potential visits by the vehicles:
    let visits: CP.IntervalVar[][] = Array.from({ length: nbCustomers }, () => []);
    // For each behicle we have the returning visit to the depot:
    let lasts: CP.IntervalVar[] = [];
    // For each vehicle, the time of the last visit:
    let endTimes: CP.IntExpr[] = [];
    // For each vehicle, we compute the max index of a customer served.
    // Used only for symmetry-breaking.
    let maxServed: CP.IntExpr[] = [];
    // Usage of each vehicle (how much capacity is used):
    let vehicleUsage: CP.IntExpr[] = [];

    for (let v = 0; v < instance.nbVehicles; v++) {
        // Visits done by the vehicle v:
        let myVisits = Array.from({ length: nbCustomers }, (_, i) =>
            model.intervalVar({ length: instance.service_times![i + 1], name: `V_${v + 1}_${i + 2}`, optional: true })
        );
        // Add myVisits to the visits array:
        for (let i = 0; i < nbCustomers; i++)
            visits[i].push(myVisits[i]);

        let sequence = model.sequenceVar(myVisits);
        model.noOverlap(sequence, customerMatrix);

        // Constraints for the depot:
        let last = model.intervalVar({ length: 0, name: `last_${v + 1}`, end: [0, horizon] });
        for (let i = 0; i < nbCustomers; i++) {
            // We don't model the initial depot visit at all. It is known to be at time 0.
            // Instead, we increase startMin of all the visits by the transition matrix value:
            myVisits[i].setStartMin(Math.max(instance.transitionMatrix[0][i + 1], instance.ready_times![i + 1]));
            myVisits[i].setStartMax(instance.due_times![i + 1]);
            // The return to depot must be after all the other visits and respect the transition matrix:
            model.endBeforeStart(myVisits[i], last, instance.transitionMatrix[i + 1][0]);
        }
        endTimes.push(last.end());
        lasts.push(last);

        // Capacity of the vehicle cannot be exceeded:
        let used = model.sum(myVisits.map((itv, i) => itv.presence().times(instance.demands![i + 1])));
        model.constraint(used.le(instance.capacity));
        vehicleUsage.push(used);

        // Compute the max index of a served customer as:
        //    min_i { (i+1) * myVisits[i].presence() }
        // There is +1 to distinguish between serving no customer (value 0) and
        // serving just the customer with index 0 (value 1).
        let maxServedCustomer = model.max(myVisits.map((itv, i) => itv.presence().times(i + 1)));
        maxServed.push(maxServedCustomer);

        if (instance.hasDirectionSymmetry && breakDirectionSymmetry) {
            // Let's compute the time of the customer with the max index served:
            //   sum_i { myVisits[i].start() * (maxServedCustomer == i+1) }
            // Here we use boolean expression maxServedCustomer == i+1 as 0/1 integer expression.
            let timeOfMaxServedCustomer =
                model.sum(myVisits.map((itv, i) => itv.start().times(maxServedCustomer.eq(i + 1))));
            // The route taken in the reverse order is also a solution.
            // So we may insist that the time of this visit is in the first half of the route:
            model.constraint(timeOfMaxServedCustomer.times(2).le(last.end()));
        }
    }

    for (let i = 0; i < nbCustomers; i++) {
        // Every customer must be visited exactly once:
        //    sum_j visits[i][j] == 1
        // We don't need alternative constraint.
        model.constraint(model.sum(visits[i].map(vis => vis.presence())).eq(1));
    }

    // All the demands must be satisfied by some vehicle. Therefore the sum of
    // their usage must be equal to the total demand.  It is a redundant
    // constraint. It allows the solver to see a problem when some vehicles are
    // underused and there is no way to satisfy the remaining demands by the
    // remaining vehicles.
    let totalDemand = instance.demands.slice(1).reduce((a, b) => a + b, 0);
    model.constraint(model.sum(vehicleUsage).eq(totalDemand));

    if (breakVehicleSymmetry) {
        // The values of the maxServed variables must be increasing with the vehicle number.
        // For the case the two vehicles are not used at all, i.e., both maxServed
        // are 0, there is max2(1) on the right side.
        for (let c = 1; c < instance.nbVehicles; c++)
            model.constraint(maxServed[c - 1].le(maxServed[c].max2(1)));
        // Customer with the biggest index must be served by the last vehicle.
        // Customer with the second biggest index must be served by the last or the second last vehicle.
        // etc.
        for (let i = nbCustomers - 1; i > nbCustomers - instance.nbVehicles; i--) {
            // How many possible vehicles can serve this customer:
            let nbPossibleVehicles = nbCustomers - i;
            let nbForbiddenVehicles = instance.nbVehicles - nbPossibleVehicles;
            for (let v = 0; v < nbForbiddenVehicles; v++)
                visits[i][v].makeAbsent();
        }
    }

    model.minimize(model.sum(endTimes));


    return [model, { visits, lasts }];
}
