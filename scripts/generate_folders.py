import os
import sys

PROBLEMS = ["TSP", "CVRP", "VRP-TW"]

project_root = os.path.normpath(os.path.join(os.path.split(__file__)[0], '..'))

for problem in PROBLEMS:
    instances = os.listdir(os.path.join(project_root, "data", problem))
    configurations = os.listdir(os.path.join(project_root, "configs", problem))
    configurations = [os.path.splitext(conf)[0] for conf in configurations]
    for instance in instances:
        for configuration in configurations:
            os.makedirs(os.path.join(project_root, "logs", problem, instance, configuration), exist_ok=True)


