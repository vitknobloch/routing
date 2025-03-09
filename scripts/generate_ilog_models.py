import os
import sys
import subprocess
from time import sleep

PROJECT_ROOT = os.path.normpath(os.path.join(os.path.split(__file__)[0], '..'))
RUN_SCRIPT_PATH = os.path.join(PROJECT_ROOT, "scripts", "generate_ilog_models.js")

class Benchmark:
    def __init__(self):
        self.finished_runs = 0
        self.total_runs = 0

    def run_file(self, config, instance, model_file):
        log = os.path.join(PROJECT_ROOT, "logs", "test.txt")
        command = ["node", RUN_SCRIPT_PATH, config, instance, log, "--dontSolve", "--exportTxt", model_file]
        print(f"Generating ilog files: {self.finished_runs + 1}/{self.total_runs}")
        proc = subprocess.run(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.finished_runs += 1

    def run_config(self, config, data_folder, models_folder):
        instance_names = os.listdir(data_folder)
        config_name = os.path.splitext(os.path.basename(config))[0]
        self.total_runs = len(instance_names)
        for instance_name in instance_names:
            instance = os.path.join(data_folder, instance_name)
            model_file = os.path.join(models_folder, f"{os.path.splitext(instance_name)[0]}.cpo")
            self.run_file(config, instance, model_file)


if __name__ == '__main__':
    problem = sys.argv[1]
    config = sys.argv[2]

    assert problem in ["TSP", "CVRP", "VRP-TW"]
    assert os.path.isfile(config)

    config = os.path.abspath(config)

    data_folder = os.path.join(PROJECT_ROOT, "data", problem)
    log_folder = os.path.join(PROJECT_ROOT, "logs", problem)
    models_folder = os.path.join(PROJECT_ROOT, "ilog_models", problem)

    finished_runs = 0
    total_runs = 0

    benchmark = Benchmark()
    benchmark.run_config(config, data_folder, models_folder)
    