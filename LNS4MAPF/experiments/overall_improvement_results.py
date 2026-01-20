import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import json
import re
from scipy.optimize import curve_fit
import itertools

def mean_of_runs(x): # x is a list of lists
    # make them equal length
    length = max(map(len, x))
    y=np.array([list(xi)+[np.nan]*(length-len(xi)) for xi in x])
    return np.nanmean(y, axis=0)

def average_runs(x, times, time_limit):
    times_np = np.asarray([t[0] for t in times if t])
    t_start = np.mean(times_np)
    if (t_start > time_limit):
        return np.asarray([np.nan]), np.asarray([np.nan])
    t = np.linspace(t_start, time_limit, 1000)
    costs = np.array([np.interp(t, time, run) for time, run in zip(times, x)])
    average_costs = np.mean(costs, axis=0)
    
    return t, average_costs

def func(x, a, b, c):
    return a * np.exp(-b * x) + c

def str_vec_to_float_vec(vec):
    return [float(x) for x in vec]

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM overall_improvement", conn)

# close the database
conn.close()

# convert the json strings to vectors vector
df['iteration_time_wall'] = df['iteration_time_wall'].apply(json.loads)
df['time_cumsum'] = df['iteration_time_wall'].apply(np.cumsum)
df['sum_of_cost'] = df['sum_of_cost'].apply(json.loads)
df['last_sum_of_cost'] = df['sum_of_cost'].apply(lambda x: x[-1] if x and x[-1] > 0 else float("inf"))
df["BKS"] = df.groupby(["map_name", "num_agents", "scene_name"])["last_sum_of_cost"].transform('min')
# df["GAP"] = (df["sum_of_cost"] - df["BKS"]) / df["BKS"]
df["GAP"] = df.apply(lambda row: [(val - row["BKS"]) / row["BKS"] for val in row["sum_of_cost"]], axis=1)
total_algorithms = df["algo_name"].nunique()

prop_cycle = plt.rcParams['axes.prop_cycle']
colors_list = prop_cycle.by_key()['color']

# Get unique maps
unique_maps = df["map_name"].unique()
num_maps = len(unique_maps)
x_axis_iteration = False

metric = "GAP"

algo_aliases = {"MAPF-LNS2" : "MAPF-LNS2", "MAPF-LNS" : "MAPF-LNS", "EECBS" : "Anytime EECBS", "BCBS" : "Anytime BCBS", "ADDRESS" : "ADDRESS", "subopt" : "my solver"}
algo_colors = {"MAPF-LNS2" : colors_list[1], "MAPF-LNS" : colors_list[0], "EECBS" : colors_list[3], "BCBS" : colors_list[4], "ADDRESS" : colors_list[2], "subopt" : colors_list[5]}

# iterate over neighborhood sizes
fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(7, 8), sharey=False)
axes = axes.flatten()

if num_maps == 1:  # If only one map, axes is not an array
    axes = [axes]

# Iterate over each map
for j, (map_name, map_group) in enumerate(df.groupby('map_name')):
    # print("map_name:", map_name)
    ax = axes[j]
    
    # Plot each algorithm
    for algo_name, algo_group in map_group.groupby('algo_name'):
        display_name = algo_name
        for a in algo_aliases.keys():
            if a in algo_name:
                display_name = algo_aliases[a]
                color = algo_colors[a]
                break
        metric_list = algo_group[metric].to_list()
        time_list = algo_group["time_cumsum"].to_list()
        for i in range(len(metric_list)):
            x = np.asarray(metric_list[i])
            time = np.asarray(time_list[i])
            x_new = x[x >= 0]
            time = time[x >= 0]
            metric_list[i] = x_new.tolist()
            time_list[i] = time.tolist()
        if map_name == "empty-8-8":
            time_limit = 0.2
        elif map_name in {"ost003d", "den520d"}:
            time_limit = 30
        else:
            time_limit = 5
        mean_time, mean_run = average_runs(metric_list, time_list, time_limit)
        
        ax.plot(mean_time, mean_run, label=f"{display_name}", linewidth=2.5, color=color)

    ax.set_title(map_name)
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("GAP [-]")  # Only label Y-axis on first subplot

    # ax.legend()
    # ax.legend(loc='center down', bbox_to_anchor=(0.5, 1))

    ax.grid(True)

# Put a legend below current axis
handles, labels = axes[1].get_legend_handles_labels()
fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=3)
plt.tight_layout()
plt.subplots_adjust(bottom=0.14)
# plt.tight_layout()
#plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

#fig.suptitle(f"{metric} Across Maps", fontsize=16)
#plt.tight_layout()
plt.show()
