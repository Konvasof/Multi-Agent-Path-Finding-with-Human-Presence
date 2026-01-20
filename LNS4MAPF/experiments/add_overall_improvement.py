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

files = ["results_overall_improvement.csv", "results_overall_improvement_ADDRESS.csv", "results_overall_improvement_BCBS.csv", "results_overall_improvement_EECBS.csv"]
solver_names = ["MAPF-LNS2", "ADDRESS", "BCBS", "EECBS", "MAPF-LNS"]
instance_idx = [16, 16, 10, 10, 11]
num_agents = {"empty-8-8" : 32, "empty-32-32" : 300, "random-32-32-20" : 150, "warehouse-10-20-10-2-1" : 300,"ost003d" : 600, "den520d" : 500}
exp_dirs = ["./overall_improvement_exp/", "./overall_improvement_exp_address/", "./overall_improvement_exp_bcbs/", "./overall_improvement_exp_eecbs/", "./overall_improvement_exp_lns/"]


conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()
counts = defaultdict(int)
added = 0
skipped = 0
for f_idx in range(len(files)):
    file_res = read_file(files[f_idx])
    solver_name = solver_names[f_idx]
    col_names = file_res[0]
    file_res = file_res[1:]

    for experiment in file_res:
# check whether the experiment is already in the database
        instance_name = experiment[instance_idx[f_idx]]
        idx1 = len("../MAPF-benchmark/mapf-scen-random/scen-random/")
        idx2 = instance_name.find(".scen")
        instance_name_stripped = instance_name[idx1:idx2]
        idx3 = instance_name_stripped.find("-random")
        map_name = instance_name_stripped[:idx3]
        agent_num = num_agents[map_name]
        file_name = "overall_improvement_" + str(agent_num) + "agents_"+ map_name + "_" + instance_name_stripped[idx3+1:] + "_" + solver_name
        num = counts[file_name]
        counts[file_name] += 1
        file_name += "_" + str(num)

        sum_of_costs = []
        computation_times = []
        i = 0
        last = -1
        time_idx = 2
        if f_idx == 1: time_idx = 3
        try:
            if (f_idx == 0):
                file_res2 = read_file(exp_dirs[f_idx] + file_name + "-LNS.csv")
            elif (f_idx == 1):
                file_res2 = read_file(exp_dirs[f_idx] + file_name + ".csv-LNS.csv")
            else:
                file_res2 = read_file(exp_dirs[f_idx] + file_name + ".csv")
            for row in file_res2:
                if (i == 0): 
                    i += 1
                    continue
                sum_of_costs.append(int(row[1]))
                if (i == 1):
                    computation_times.append(float(row[time_idx]))
                else:
                    computation_times.append(float(row[time_idx]) - last)
                i += 1
                last = float(row[time_idx])
        except:
            print("Could not load:", file_name)
            continue
        
        if (len(sum_of_costs) == 0 or sum_of_costs[-1] == -1):
            print("Did not find a solution, discarding: ", file_name)
            continue

        algo_parameters = "[]"
        try:
            seed = int(experiment[17])
        except:
            seed = 0

        algo_name = solver_name
        cursor.execute("""
        SELECT 1 FROM overall_improvement WHERE 
            file_name = ?
        """, (file_name,))

        if cursor.fetchone() is None:  # If no record found, insert new entry
            # Dynamically create the SQL query
            cursor.execute("""
            INSERT INTO overall_improvement (
                file_name, experiment_name, map_name, scene_name, num_agents,
    algo_name, algo_parameters, sum_of_dist, feasible, init_sol_sum_of_cost, init_sol_makespan, experiment_time_cpu, experiment_time_wall, expanded, generated, preprocessing_time_cpu, preprocessing_time_wall, seed, sum_of_cost, makespan, iteration_time_cpu, iteration_time_wall, operators
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                file_name, "overall_improvement", map_name, instance_name_stripped + ".scen", agent_num, algo_name, algo_parameters, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, seed, json.dumps(sum_of_costs), json.dumps([0]), json.dumps(computation_times), json.dumps(computation_times), json.dumps(["ADAPTIVE"])
            ))
            added += 1
        else:
            skipped += 1
conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Added: " + str(added) + "/" + str(added + skipped) + " experiments")
print("Skipped: " + str(skipped) + "/" + str(added + skipped) + " experiments")
