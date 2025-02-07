import { ParseResult } from "./parsetsp";
import * as CP from '@scheduleopt/optalcp';
import { makeModelName } from "./utils";

export type TspVars = {
    visits: CP.IntervalVar[]
}

export function defineModel(instance: ParseResult, filename: string, visit_duration: number): [CP.Model, TspVars] {
    let model = new CP.Model(makeModelName(instance.type, filename));
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