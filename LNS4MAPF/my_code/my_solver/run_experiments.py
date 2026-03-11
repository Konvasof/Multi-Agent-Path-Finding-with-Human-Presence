import subprocess
import itertools
import csv
import time
import re

executable = "./build/simple_MAPF_solver" 
maps_and_scens = [
    ("../../MAPF-benchmark/mapf-map/room-32-32-4.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/room-32-32-4-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/room-64-64-8.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/room-64-64-8-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/room-64-64-16.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/room-64-64-16-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/maze-32-32-4.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/maze-32-32-4-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/maze-32-32-2.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/maze-32-32-2-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/random-32-32-10.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/random-32-32-10-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/random-32-32-20.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/random-32-32-20-even-1.scen"),
    ("../../MAPF-benchmark/mapf-map/random-64-64-10.map", "../../MAPF-benchmark/mapf-scen-even/scen-even/random-64-64-10-even-1.scen")
]
agent_counts = [20, 30, 40, 50, 60] 
seeds = range(1, 11) 

output_file = "experiment_results.csv"

with open(output_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Map", "Agents", "Seed", "Success", "Cost", "Time_s"])

    total_runs = len(maps_and_scens) * len(agent_counts) * len(seeds)
    current_run = 0

    for (map_file, scen_file), agents, seed in itertools.product(maps_and_scens, agent_counts, seeds):
        current_run += 1
        print(f"Running {current_run}/{total_runs} | Map: {map_file.split('/')[-1]} | Agents: {agents} | Seed: {seed}")

        current_n = 5
        command = [
            executable, 
            "-m", map_file, 
            "-a", scen_file, 
            "-k", str(agents), 
            "-n", str(current_n),
            "--seed", str(seed), 
            "--safetyCheck", "1",
            "-G", "0" 
        ]
        start_time = time.time()
        
        result = subprocess.run(command, capture_output=True, text=True)
        
        end_time = time.time()
        elapsed_time = end_time - start_time

        output = result.stdout
        success = False
        cost = -1

        cost_match = re.search(r"Final solution has sum of costs: (\d+)", output)
        if cost_match:
            success = True
            cost = int(cost_match.group(1)) # (např. 574)
        
        #CSV
        map_name = map_file.split('/')[-1]
        writer.writerow([map_name, agents, seed, success, cost, elapsed_time])

print(f"\n Výsledky jsou uloženy v {output_file}")
