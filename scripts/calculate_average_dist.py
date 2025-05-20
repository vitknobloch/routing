import os
import sys
import subprocess
from time import sleep

PROJECT_ROOT = os.path.normpath(os.path.join(os.path.split(__file__)[0], '..'))
RUN_SCRIPT_PATH = os.path.join(PROJECT_ROOT, "scripts", "calculate_average_dist.js")
LOG = os.path.join(PROJECT_ROOT, "logs", "max_dist.txt")

class Benchmark:
    def __init__(self):
        self.finished_runs = 0
        self.total_runs = 0

    def run_file(self, config, instance):
        
        command = ["node", RUN_SCRIPT_PATH, config, instance, LOG]
        print(f"Generating ilog files: {self.finished_runs + 1}/{self.total_runs}")
        proc = subprocess.run(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.finished_runs += 1

    def run_config(self, config, data_folder):
        instance_names = os.listdir(data_folder)
        config_name = os.path.splitext(os.path.basename(config))[0]
        self.total_runs = len(instance_names)
        for instance_name in instance_names:
            instance = os.path.join(data_folder, instance_name)
            self.run_file(config, instance)


if __name__ == '__main__':
    problems = ["TSP", "CVRP", "VRP-TW"]
    configs = [os.path.join(PROJECT_ROOT, "configs", problem, "1LNS.json") for problem in problems]

    with open(LOG, "w") as log:
        log.write("")

    for problem, config in zip(problems, configs):
        data_folder = os.path.join(PROJECT_ROOT, "data", problem)
        finished_runs = 0
        total_runs = 0

        benchmark = Benchmark()
        benchmark.run_config(config, data_folder)
    