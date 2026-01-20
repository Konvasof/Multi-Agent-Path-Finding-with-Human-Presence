# an experiment to compare original implementation and improved implementation
import subprocess
import re
import numpy as np
import tqdm
import os
import sqlite3
import pandas as pd

def construct_experiment(solver_path, map_path, scen_path, output_file_name, agent_num, seed):
    """ Construct the input of subprocess.check_output to perform the experiment"""
    return [solver_path, '-m', map_path, '-a', scen_path, "-o", output_file_name + ".csv", "-k", str(agent_num), "--initAlgo", "PP", "--maxIterations",  str(0), "--initLNS", "0", "--seed", str(seed), "-t", str(1)]

def find_last_number(s: str) -> int:
    """Extracts the last number from a string. Returns -1 if no number is found."""
    numbers = re.findall(r'\d+', s)  # Find all numbers in the string
    return int(numbers[-1]) if numbers else -1  # Return last number if found, else -1

def sort_strings_by_last_number(strings: list) -> list:
    """Sorts a list of strings based on the last number found in each string."""
    return sorted(strings, key=find_last_number)

def construct_scen_path(scen_dir, map_name, k):
    """ Construct the path to a scen file """
    return scen_dir + map_name + "-random-" + str(k) + ".scen"

# parameters
map_names = [ "empty-8-8", "empty-32-32", "random-32-32-20", "warehouse-10-20-10-2-1", "ost003d", "den520d" ]
agent_nums = [[8, 16, 24], [100, 150, 200, 250, 300], [20, 40, 60, 80, 100], [50, 100, 150, 200], [100, 200, 300, 400, 500],
    [ 100, 200, 300, 400, 500 ]]
num_runs = 100 
results_file = "results_sipp_pp"
mapf_benchmark_dir = "../MAPF-benchmark/"
scen_dir = mapf_benchmark_dir + "mapf-scen-random/scen-random/"
solver_path2 = "../papers/MAPF-LNS2/lns"

# map_names = ["den520d"]


conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

map_idx = 0
# list all maps
for map_name in tqdm.tqdm(map_names, desc="maps", position=0):
    # print("Map:", map_name)
    map_path = mapf_benchmark_dir + "mapf-map/" + map_name + ".map"

    # list all scenes
    scenes = sort_strings_by_last_number([f for f in os.listdir(scen_dir) if f.startswith(map_name)])

    for agent_num in tqdm.tqdm(agent_nums[map_idx], desc="agent nums", position=1, leave=False):
        for i in tqdm.tqdm(range(num_runs), desc="runs", position=2, leave=False):
            scen_name = os.path.splitext(scenes[i % len(scenes)])[0]
            scen_name_stripped = scen_name[len(map_name)+1:]
            scen_path = scen_dir + scenes[i % len(scenes)]

            # find the seed
            experiment_name = "sipp_pp_" + str(agent_num) + "agents_" + map_name + "_" + scen_name_stripped +  "_SIPP_mapf_lns_" + str(int(i/len(scenes))) + ".json"
            df = pd.read_sql_query(f"SELECT * FROM sipp_pp WHERE file_name = '{experiment_name}'", conn)
            seeds = df["seed"]
            if (len(seeds) != 1):
                print("ERROR: Wrong seeds", seeds, experiment_name)
                exit(1)
            seed = seeds[0]
            exp = construct_experiment(solver_path2, map_path, scen_path, results_file, agent_num, seed)
            try:
                res = subprocess.check_output(exp)[:-1].decode('UTF-8')
            except:
                print("Solver returned error for map: ", map_name, ", scene: ", s)
                print(" ".join(map(str, exp)))
    map_idx += 1


