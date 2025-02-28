import os
import sys
import subprocess
from time import sleep

SEEDS = ["0", "1", "2"]
EXEC_TIME = 120
EXTRA_TIME = 2
PAUSE_TIME = 2

PROJECT_ROOT = os.path.normpath(os.path.join(os.path.split(__file__)[0], '..'))
RUN_SCRIPT_PATH = os.path.join(PROJECT_ROOT, "scripts", "run.js")

class Benchmark:
    def __init__(self):
        self.finished_runs = 0
        self.total_runs = 0

    def run_seed(self, config, instance, seed, log):
        
        command = ["node", RUN_SCRIPT_PATH, config, instance, log, "--randomSeed", seed]
        proc = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        eta = int((self.total_runs - self.finished_runs) * (EXEC_TIME + EXTRA_TIME + PAUSE_TIME) / 60)
        print(f"Running experiment: {self.finished_runs + 1}/{self.total_runs} (ETA: {int(eta/60)} hrs {eta % 60} min)")
        print(f"{os.path.basename(instance)} seed: {seed}")
        sleep(EXEC_TIME + EXTRA_TIME)
        status = proc.poll()
        if(status is not None and status != 0):
            print(f"Instance crashed!")
        proc.kill()
        self.finished_runs += 1
        sleep(PAUSE_TIME)

    def run_instance(self, config, instance, log_folder):
        for seed in SEEDS:
            self.run_seed(config, instance, seed, os.path.join(log_folder, f'{seed}.log'))

    def run_config(self, config, data_folder, log_folder):
        instance_names = os.listdir(data_folder)
        config_name = os.path.splitext(os.path.basename(config))[0]
        self.total_runs = len(instance_names) * len(SEEDS)
        self.generate_folders(config_name, instance_names, log_folder)
        for instance_name in instance_names:
            log_folder_instance = os.path.join(log_folder, instance_name, config_name)
            instance = os.path.join(data_folder, instance_name)
            self.run_instance(config, instance, log_folder_instance)

    def generate_folders(self, config_name, instance_names, log_folder):
        for instance_name in instance_names:
            log_folder_instance = os.path.join(log_folder, instance_name, config_name)
            os.makedirs(log_folder_instance, exist_ok=True)



if __name__ == '__main__':
    problem = sys.argv[1]
    config = sys.argv[2]

    assert problem in ["TSP", "CVRP", "VRP-TW"]
    assert os.path.isfile(config)

    config = os.path.abspath(config)

    data_folder = os.path.join(PROJECT_ROOT, "data", problem)
    log_folder = os.path.join(PROJECT_ROOT, "logs", problem)

    finished_runs = 0
    total_runs = 0

    benchmark = Benchmark()
    benchmark.run_config(config, data_folder, log_folder)
    