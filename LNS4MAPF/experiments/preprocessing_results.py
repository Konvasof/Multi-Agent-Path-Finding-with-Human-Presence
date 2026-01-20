import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM feasible_restarts", conn)
df["delay"] = df["init_sol_sum_of_cost"] - df["sum_of_dist"]

# close the database
conn.close()

df_grouped = df.groupby(["map_name", "num_agents", "algo_name"])
stats = df_grouped.agg({
    "preprocessing_time_cpu": ["mean", "std", "median", "min", "max"],
    "preprocessing_time_wall": ["mean", "std", "median", "min", "max"],
}).reset_index()

# Get unique maps
unique_maps = stats["map_name"].unique()
num_maps = len(unique_maps)

# plot the wall time
# create a single figure where the maps are on the x-axis and the preprocessing time on the y-axis
fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(6, 7), sharey=False)
axes = axes.flatten()
# metric = "preprocessing_time_wall"
metric = "preprocessing_time_cpu"


if num_maps == 1:  # If only one map, axes is not an array
    axes = [axes]
for i, (map_name, map_group) in enumerate(stats.groupby('map_name')):
    ax = axes[i]
    values_mine = []
    for algo_name, algo_group in map_group.groupby('algo_name'):
        agent_nums = algo_group["num_agents"].values
        if "orig" in algo_name:
            values_orig = algo_group[(metric, "mean")].values
        else:
            values_mine.append(algo_group[(metric, "mean")].values)
    # average the mine values across algorithms
    values_mine = np.asarray(values_mine)
    values_mine = np.mean(values_mine, axis=0)
    # plot the values
    ax.plot(agent_nums, values_orig, label=f"MAPF-LNS2", marker='o')
    ax.plot(agent_nums, values_mine, label=f"my solver", marker='o')
    ax.set_title(map_name)
    ax.set_xlabel("Number of agents [-]")
    if "cpu" in metric:
        ax.set_ylabel("Cpu time [s]")  # Only label Y-axis on first subplot
    else:
        ax.set_ylabel("Wall time [s]")  # Only label Y-axis on first subplot
    ax.grid(True)

handles, labels = axes[0].get_legend_handles_labels()
fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=2)
plt.tight_layout()
plt.subplots_adjust(bottom=0.13)
# plt.tight_layout()
#plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

#plt.tight_layout()
plt.show()


    


