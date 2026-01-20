import csv
import sqlite3
from collections import defaultdict
import json

def read_file(name):
    lines = []
    with open(name, mode ='r') as file:
      csvFile = csv.reader(file)
      for line in csvFile:
          lines.append(line)
    return lines

# exp_name = "feasible"
exp_name = "feasible_restarts"
# exp_name = "sipp_pp"

if (exp_name == "feasible"):
    file_res = read_file("results_feasible.csv")
elif (exp_name == "feasible_restarts"):
    file_res = read_file("results_feasible_restarts.csv")
elif (exp_name == "sipp_pp"):
    file_res = read_file("results_sipp_pp.csv")
else:
    file_res = read_file("results_lns2.csv")

col_names = file_res[0]
file_res = file_res[1:]

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()
counts = defaultdict(int)
algo_name =  "SIPP_lns_orig"
added = 0
skipped = 0

for experiment in file_res:
# check whether the experiment is already in the database
    instance_name = experiment[16]
    agent_num = int(experiment[21])
    idx1 = len("../MAPF-benchmark/mapf-scen-random/scen-random/")
    idx2 = instance_name.find(".scen")
    instance_name_stripped = instance_name[idx1:idx2]
    file_name = exp_name + "_" + str(agent_num) + "agents_"+ instance_name_stripped + "_" + algo_name
    num = counts[file_name]
    counts[file_name] += 1
    file_name += "_" + str(num)
    idx3 = instance_name_stripped.find("-random")
    map_name = instance_name_stripped[:idx3]

    if (exp_name == "feasible_restarts"):
        feasible = (int(experiment[2]) > 0) # feasible when solution cost is not -1 and number of restarts is 0
    else:
        feasible = (int(experiment[2]) > 0 and int(experiment[8]) == 0) # feasible when solution cost is not -1 and number of restarts is 0

    algo_parameters = "[]"
    computation_time_cpu = float(experiment[19])
    computation_time_wall = float(experiment[0])
    expanded = int(experiment[10])
    generated = int(experiment[11]) + int(experiment[12])
    makespan = int(experiment[18])
    preprocessing_time_cpu = float(experiment[20])
    preprocessing_time_wall = float(experiment[14])
    seed = int(experiment[17])
    sum_of_cost = int(experiment[2])
    sum_of_dist = int(experiment[4])


    cursor.execute("""
    SELECT 1 FROM """ + exp_name + """ WHERE 
        file_name = ?
    """, (file_name,))

    if cursor.fetchone() is None:  # If no record found, insert new entry
        # Dynamically create the SQL query
        cursor.execute("""
        INSERT INTO """ + exp_name + """ (
            file_name, experiment_name, map_name, scene_name, num_agents,
algo_name, algo_parameters, sum_of_dist, feasible, init_sol_sum_of_cost, init_sol_makespan, experiment_time_cpu, experiment_time_wall, expanded, generated, preprocessing_time_cpu, preprocessing_time_wall, seed, sum_of_cost, makespan, iteration_time_cpu, iteration_time_wall, operators
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            file_name, exp_name, map_name, instance_name_stripped + ".scen", agent_num, algo_name, algo_parameters, sum_of_dist, feasible, sum_of_cost, makespan, computation_time_cpu, computation_time_wall, expanded, generated, preprocessing_time_cpu, preprocessing_time_wall, seed, json.dumps([sum_of_cost]), json.dumps([makespan]), json.dumps([computation_time_cpu]), json.dumps([computation_time_wall]), json.dumps(["ADAPTIVE"])
        ))
        added += 1
    else:
        skipped += 1
conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Added: " + str(added) + "/" + str(added + skipped) + " experiments")
print("Skipped: " + str(skipped) + "/" + str(added + skipped) + " experiments")
