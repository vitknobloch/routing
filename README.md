Heuristics for Routing Problems for use with OptalCp optimizer
==============================================================
This repository contains the implementation of several heuristics for TSP, CVRP and VRP-TW problems, models for OptalCP optimizer and IPC in between the heuristic portfolio and the OptalCP solver.

Running docker
--------------
```bash
docker run --user $(id -u) -it -v $PWD/logs:/workspace/logs -v $PWD/data:/workspace/data:ro routing:latest
```
