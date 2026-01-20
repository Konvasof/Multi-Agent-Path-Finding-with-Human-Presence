import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import re

def find_last_number(s: str) -> int:
    """Extracts the last number from a string. Returns -1 if no number is found."""
    numbers = re.findall(r'\d+', s)  # Find all numbers in the string
    if numbers:
        if exp_name == "influence_of_w":
            return int(numbers[-2])
        else: return int(numbers[-1])
    return -1

def sort_strings_by_last_number(strings: list) -> list:
    """Sorts a list of strings based on the last number found in each string."""
    return sorted(strings, key=find_last_number)

def argsort_strings_by_last_number(strings: list) -> list:
    """Sorts a list of strings based on the last number found in each string."""
    rng  = list(range(len(strings)))
    return sorted(rng, key=lambda a, strings=strings : find_last_number(strings[a]))

def get_display_name(algo_name):
    display_name = "MAPF-LNS2"
    if "mapf_lns" in algo_name:
        display_name = "original heuristic"
    elif "mine" in algo_name:
        if "ap" in algo_name:
            display_name = "optimal ap"
        else:
            display_name = "optimal"
    elif "suboptimal" in algo_name:
        if exp_name == "influence_of_ap":
            display_name = "suboptimal"
            if "ap" in algo_name:
                display_name += " ap"
            if "w_1,05" in algo_name:
                display_name += " w = 1.05"
            else:
                display_name += " w = 1.0"
        elif exp_name == "influence_of_p":
            display_name = "p = " + str(find_last_number(algo_name))
        elif exp_name == "influence_of_w":
            display_name = "w = "
            if "w_1,00" in algo_name: display_name += "1.0"
            elif "w_1,01" in algo_name: display_name += "1.01"
            elif "w_1,02" in algo_name: display_name += "1.02"
            elif "w_1,05" in algo_name: display_name += "1.05"
            elif "w_1,10" in algo_name: display_name += "1.1"
            elif "w_1,5" in algo_name: display_name += "1.5"
    return display_name


conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
# exp_name = "influence_of_ap"
# exp_name = "influence_of_p"
exp_name = "influence_of_w"
df = pd.read_sql_query("SELECT * FROM " + exp_name, conn)
df["delay"] = df["init_sol_sum_of_cost"] - df["sum_of_dist"]

# close the database
conn.close()

df_feasible = df[df["feasible"] == 1]

group_sizes = df_feasible.groupby(["map_name", "num_agents", "seed"]).size()
total_algorithms = df["algo_name"].nunique()
valid_groups = group_sizes[group_sizes == total_algorithms].index
df_filtered = df_feasible.set_index(["map_name", "num_agents", "seed"]).loc[valid_groups]

df_grouped = df.groupby(["map_name", "num_agents", "algo_name"])
# df_feasible_grouped = df_feasible.groupby(["map_name", "num_agents", "algo_name"])
df_feasible_grouped = df_filtered.groupby(["map_name", "num_agents", "algo_name"])
stats = df_grouped.agg({
    "experiment_time_cpu": ["mean", "std", "median", "min", "max"],
    "experiment_time_wall": ["mean", "std", "median", "min", "max"],
    "expanded": ["mean", "std", "median", "min", "max"],
    "generated": ["mean", "std", "median", "min", "max"],
    "init_sol_makespan": ["mean", "std", "median", "min", "max"],
    "init_sol_sum_of_cost": ["mean", "std", "median", "min", "max"],
    "sum_of_dist": ["mean", "std", "median", "min", "max"],
    "delay": ["mean", "std", "median", "min", "max"],
    "feasible" : ["mean"] 
}).reset_index()

stats_feasible = df_feasible_grouped.agg({
    "experiment_time_cpu": ["mean", "std", "median", "min", "max"],
    "experiment_time_wall": ["mean", "std", "median", "min", "max"],
    "expanded": ["mean", "std", "median", "min", "max"],
    "generated": ["mean", "std", "median", "min", "max"],
    "init_sol_makespan": ["mean", "std", "median", "min", "max"],
    "init_sol_sum_of_cost": ["mean", "std", "median", "min", "max"],
    "sum_of_dist": ["mean", "std", "median", "min", "max"],
    "delay": ["mean", "std", "median", "min", "max"],
    "feasible" : ["mean"] 
}).reset_index()

# Define the metrics to plot
metric_names = ["feasible", "experiment_time_wall", "expanded", "init_sol_sum_of_cost"]
yaxis_names = {"feasible" : "Success rate [-]","experiment_time_wall" : "Time [s]", "expanded" : "Expanded nodes [-]", "init_sol_sum_of_cost" : "Sum of costs [-]" }


# Get unique maps
unique_maps = stats["map_name"].unique()
num_maps = len(unique_maps)

# Iterate over each metric
for metric in metric_names:
    fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(6, 7), sharey=False)
    axes = axes.flatten()

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    if metric == "feasible":
        for i, (map_name, map_group) in enumerate(stats.groupby('map_name')):
            ax = axes[i]

            # Plot each algorithm
            for algo_name, algo_group in map_group.groupby('algo_name'):
                display_name = get_display_name(algo_name)
                ax.plot(algo_group["num_agents"], algo_group[(metric, "mean")], marker='o', label=f"{display_name}")

            ax.set_title(map_name)
            ax.set_xlabel("Number of Agents [-]")
            ax.set_ylabel(yaxis_names["feasible"])
            ax.grid(True)
    else:
        for i, (map_name, map_group) in enumerate(stats_feasible.groupby('map_name')):
            # print("map_name:", map_name)
            ax = axes[i]

            # Plot each algorithm
            for algo_name, algo_group in map_group.groupby('algo_name'):
                display_name = get_display_name(algo_name)
                ax.plot(algo_group["num_agents"], algo_group[(metric, "mean")], marker='o', label=f"{display_name}")
                # print("Algo name:", algo_name)
                # print(algo_group[(metric, "mean")])

            ax.set_title(map_name)
            ax.set_xlabel("Number of Agents [-]")
            ax.set_ylabel(yaxis_names[metric])  # Only label Y-axis on first subplot

            ax.grid(True)

    # fig.suptitle(f"{metric} Across Maps", fontsize=16)
    handles, labels = axes[0].get_legend_handles_labels()
    order = argsort_strings_by_last_number(labels)
    fig.legend([handles[idx] for idx in order],[labels[idx] for idx in order], loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=3)
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.15)
    plt.show()

# GAP
df_filtered["BKS"] = df_filtered.groupby(["map_name", "num_agents", "scene_name"])["init_sol_sum_of_cost"].transform('min')
df_filtered["GAP"] = (df_filtered["init_sol_sum_of_cost"] - df_filtered["BKS"]) / df_filtered["BKS"]
df_gap = df_filtered.groupby(["map_name", "num_agents", "algo_name"])
stats_gap = df_gap.agg({
    "GAP" : ["mean"] 
}).reset_index()

fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(6, 7), sharey=False)
axes = axes.flatten()

if num_maps == 1:  # If only one map, axes is not an array
    axes = [axes]

# Iterate over each map
for i, (map_name, map_group) in enumerate(stats_gap.groupby('map_name')):
    # print("map_name:", map_name)
    ax = axes[i]

    # Plot each algorithm
    for algo_name, algo_group in map_group.groupby('algo_name'):
        display_name = get_display_name(algo_name)
        ax.plot(algo_group["num_agents"], algo_group[("GAP", "mean")], marker='o', label=f"{display_name}")
        # print("Algo name:", algo_name)
        # print(algo_group[(metric, "mean")])

    ax.set_title(map_name)
    ax.set_xlabel("Number of Agents [-]")
    ax.set_ylabel("GAP [-]")  # Only label Y-axis on first subplot
    ax.grid(True)

handles, labels = axes[0].get_legend_handles_labels()
fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=3)
plt.tight_layout()
plt.subplots_adjust(bottom=0.15)
plt.show()

