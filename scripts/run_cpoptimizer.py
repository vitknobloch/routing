import subprocess
import os
import sys
import json

PROJECT_ROOT = os.path.normpath(os.path.join(os.path.split(__file__)[0], '..'))
MODEL_DIR = os.path.join(PROJECT_ROOT, "ilog_models")
INSTANCE_DIR = os.path.join(PROJECT_ROOT, "data")
RUN_SCRIPT_PATH = os.path.join(PROJECT_ROOT, "solve_CPOs", "solveCPOs")
SEEDS = ["0", "1", "2"]
CONFIG_NAME = "12CPO"
TEMP_LOG_FILENAME = os.path.join(PROJECT_ROOT, "solve_CPOs", "test.json")
CPO_ARGUMENTS = ["--workers", "12", "--timeLimit", "120"]

def get_command(seed, model_filename):
    return [f"{RUN_SCRIPT_PATH}", "--output", TEMP_LOG_FILENAME, *CPO_ARGUMENTS, "--RandomSeed", seed, model_filename]


def run_file(model_filename, log_dir):
    os.makedirs(log_dir, exist_ok=True)
    for seed in SEEDS:
        command = get_command(seed, model_filename)
        print(f"Running command: {" ".join(command)}")
        subprocess.run(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        objectives = []
        times = []
        with open(TEMP_LOG_FILENAME, "r") as f:
            json_obj = json.load(f)
            objective_history = json_obj[0]["objectiveHistory"]
            for solution in objective_history:
                objectives.append(solution["objective"])
                times.append(solution["solveTime"])
        
        with open(os.path.join(log_dir, f"{seed}.log"), "w") as f:
            for time, objective in zip(times, objectives):
                if problem == "VRP-TW":
                    f.write(f"{time} {objective % 1_000_000} {objective // 1_000_000}\n")
                else:
                    f.write(f"{time} {objective}\n")

def run_problem(problem):
    assert problem in ["TSP", "CVRP", "VRP-TW"]
    model_dir = os.path.join(MODEL_DIR, problem)
    instance_dir = os.path.join(INSTANCE_DIR, problem)

    for instance_name in os.listdir(instance_dir):
        log_dir = os.path.join(PROJECT_ROOT, "logs", problem, instance_name, CONFIG_NAME)
        model_filename = os.path.join(model_dir, f"{os.path.splitext(instance_name)[0]}.cpo")
        print(f"running file: {instance_name}")
        run_file(model_filename, log_dir)

if __name__ == '__main__':
    problems = sys.argv[1:]
    for problem in problems:
        run_problem(problem)





