import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM sipp_pp", conn)
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
metric_names = ["experiment_time_cpu", "experiment_time_wall", "expanded", "init_sol_sum_of_cost", "delay"]

# Get unique maps
unique_maps = stats["map_name"].unique()
num_maps = len(unique_maps)

# Iterate over each metric
for metric in metric_names:
    fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(6, 10), sharey=False)
    axes = axes.flatten()

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    for i, (map_name, map_group) in enumerate(stats_feasible.groupby('map_name')):
        # print("map_name:", map_name)
        ax = axes[i]

        # Plot each algorithm
        for algo_name, algo_group in map_group.groupby('algo_name'):
            ax.plot(algo_group["num_agents"], algo_group[(metric, "mean")], marker='o', label=f"{algo_name}")
            # print("Algo name:", algo_name)
            # print(algo_group[(metric, "mean")])

        ax.set_title(map_name)
        ax.set_xlabel("Number of Agents")
        ax.set_ylabel(metric)  # Only label Y-axis on first subplot

        ax.grid(True)

    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=2)
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.20)
    plt.show()


