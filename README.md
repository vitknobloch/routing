Heuristics for Routing Problems for use with OptalCp optimizer
==============================================================
This repository contains the implementation of several heuristics for TSP, CVRP and VRP-TW problems, models for OptalCP optimizer and IPC in between the heuristic portfolio and the OptalCP solver.

- The `scripts` folder contains some Pyton scripts which help with experiment running or evaluation, and the Typescript files for OptalCP running, as well as the entry point for execution called `run.ts`.
- The heuristic portfolio is implemented in `include` and `src` folders.

Building
--------
OptalCP installation in `node_modules` folder required, then
```bash
docker build -t routing .
```

Running docker
--------------
```bash
docker run --user $(id -u) -it -v $PWD/logs:/workspace/logs -v $PWD/data:/workspace/data:ro routing:latest
```
