import os
import json
import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# experiment_name = "influence_of_ap"
# experiment_name = "influence_of_p"
# experiment_name = "influence_of_w"
# experiment_name = "feasible"
# experiment_name = "feasible_restarts"
# experiment_name = "sipp_pp"
# experiment_name = "destroy"
# experiment_name = "repair"
experiment_name = "overall_improvement"

execute_str = "CREATE TABLE IF NOT EXISTS " + experiment_name + """ (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_name TEXT UNIQUE, -- each experiment can be stored only once
    experiment_name TEXT,
    map_name TEXT,
    scene_name TEXT,
    num_agents INTEGER,
    algo_name TEXT,
    algo_parameters TEXT,  -- Stored as JSON string
    sum_of_dist INTEGER,
    feasible BOOLEAN,
    init_sol_sum_of_cost INTEGER,
    init_sol_makespan INTEGER,
    experiment_time_cpu REAL,
    experiment_time_wall REAL,
    expanded INTEGER,
    generated INTEGER,
    preprocessing_time_cpu REAL,
    preprocessing_time_wall REAL,
    seed INTEGER,
    sum_of_cost TEXT,  --time evolutions, JSON string
    makespan TEXT,
    iteration_time_cpu TEXT,
    iteration_time_wall TEXT,
    operators TEXT
)
"""

# Create a table for the experiments
cursor.execute(execute_str)

conn.commit()

# path to the directory with the experiments
experiment_dir = "../my_code/my_solver/experiments/"+ experiment_name

# go through the experiments and add them to the database
map_dirs = os.listdir(experiment_dir)

added = 0
skipped = 0
for map_dir in map_dirs:
    map_path = os.path.join(experiment_dir, map_dir)
    #print("Map:", map_path)
    files = os.listdir(map_path)
    for file in files:
        #print(file)
        if (file[0:len(experiment_name)] != experiment_name): continue # skip files that are not experiments
        with open(os.path.join(map_path, file), "r") as f:
            data = json.load(f) # loads the json as dictionary
            data["file_name"] = file


            # check whether the experiment is already in the database
            cursor.execute("SELECT 1 FROM " + experiment_name + """ WHERE 
                file_name = ?
            """, (file,))
            if cursor.fetchone() is None:  # If no record found, insert new entry

                # convert the algorithm parameters, solution time evolution etc to json string
                data["algo_parameters"] = json.dumps(data["algo_parameters"])
                data["init_sol_sum_of_cost"] = data["sum_of_cost"][0]
                if (experiment_name == "feasible_restarts"):
                    data["init_sol_sum_of_cost"] = data["sum_of_cost"][-1]
                if (experiment_name != "feasible_restarts" and experiment_name != "overall_improvement" and data["feasible"] and data["sum_of_cost"][0] == -1):
                    data["feasible"] = False
                data["sum_of_cost"] = json.dumps(data["sum_of_cost"])
                data["init_sol_makespan"] = data["makespan"][0]
                data["makespan"] = json.dumps(data["makespan"])
                data["iteration_time_cpu"] = json.dumps(data["iteration_time_cpu"])
                data["iteration_time_wall"] = json.dumps(data["iteration_time_wall"])
                data["operators"] = json.dumps(data["operators"])


                # Dynamically create the SQL query
                columns = ", ".join(data.keys())  # Get column names from JSON keys
                placeholders = ", ".join(["?"] * len(data))  # Create placeholders (?, ?, ?, ...)

                sql = "INSERT INTO " + experiment_name + f" ({columns}) VALUES ({placeholders})"
                cursor.execute(sql, tuple(data.values()))
                added += 1
            else:
                skipped += 1
conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Added: " + str(added) + "/" + str(added + skipped) + " experiments")
print("Skipped: " + str(skipped) + "/" + str(added + skipped) + " experiments")
