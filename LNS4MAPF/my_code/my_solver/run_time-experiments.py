import subprocess
import itertools
import csv
import time
import re

executable = "./build/simple_MAPF_solver" 
maps_and_scens = [
    ("../../MAPF-benchmark/mapf-map/room-32-32-4.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/room-32-32-4-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/maze-32-32-4.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/random-32-32-10.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/random-32-32-10-even-1.scen")
]

agent_counts = [30, 60, 90] 
time_limits = [30, 90, 150] 
#time_limits = [30] 
seeds = range(1,2) 

output_file = "second_experiment_time_results.csv"

with open(output_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Map", "Agents", "TimeLimit", "Seed", "Success", "Cost", "RealTime_s"])

    total_runs = len(maps_and_scens) * len(agent_counts) * len(time_limits) * len(seeds)
    current_run = 0

    for (map_file, scen_file), agents, t_limit, seed in itertools.product(maps_and_scens, agent_counts, time_limits, seeds):
        current_run += 1
        map_name = map_file.split('/')[-1]
        print(f"Running {current_run}/{total_runs} | Map: {map_name} | Agents: {agents} | TimeLimit: {t_limit}s | Seed: {seed}")

        current_n = min(5, agents - 1)
        command = [
            executable, 
            "-m", map_file, 
            "-a", scen_file, 
            "-k", str(agents), 
            "-n", str(current_n),
            "--seed", str(seed), 
            "--safetyCheck", "1",
            "--timeLimit", str(t_limit), 
            "-i", "1000000",
            "-G", "0" 
        ]
        
        start_time = time.time()
        result = subprocess.run(command, capture_output=True, text=True)
        elapsed_time = time.time() - start_time

        if result.returncode != 0:
            print(f"   !!! C++ PROGRAM SPADL (Kód {result.returncode}): {result.stderr.strip()}")

        output = result.stdout
        success = False
        cost = -1

        cost_match = re.search(r"Final solution has sum of costs: (\d+)", output)
        if cost_match:
            success = True
            cost = int(cost_match.group(1))
        
        writer.writerow([map_name, agents, t_limit, seed, success, cost, elapsed_time])

print(f"\nDONE! Results are saved in {output_file}")
