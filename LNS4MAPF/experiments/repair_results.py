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

def average_runs(x, times, max_time):
    t = np.linspace(0, max_time, 1000)
    costs = np.array([np.interp(t, time, run) for time, run in zip(times, x)])
    average_costs = np.mean(costs, axis=0)
    
    return t, average_costs

def func(x, a, b, c):
    return a * np.exp(-b * x) + c

# def func(x, a, b, c):
#     return a * np.power(x, 2) + b * x + c 

def str_vec_to_float_vec(vec):
    return [float(x) for x in vec]

def plot_algos(algorithms, algo_aliases, times, metric, df_filtered):
    fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(7, 8), sharey=False)
    axes = axes.flatten()

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    for i, (map_name, map_group) in enumerate(df_filtered.groupby('map_name')):
        # print("map_name:", map_name)
        ax = axes[i]

        # Plot each algorithm
        for algo_name, algo_group in map_group.groupby('algo_name'):
            found = False
            for alg in algorithms:
                if alg in algo_name:
                    found = True
                    break
            if not found: continue
            display_name = algo_name
            for a in algo_aliases.keys():
                if a in algo_name:
                    display_name = algo_aliases[a]
                    break
            metric_list = algo_group[metric].to_list()
            time_list = algo_group["time_cumsum"].to_list()
            mean_time, mean_run = average_runs(metric_list, time_list, times[i])
            ax.plot(mean_time, mean_run, label=f"{display_name}", linewidth=2.5)

        ax.set_title(map_name)
        ax.set_xlabel("Time [s]")
        ax.set_ylabel("GAP [-]")

        ax.grid(True)

# Put a legend below current axis
    handles, labels = axes[0].get_legend_handles_labels()
    order = np.argsort(labels)
    handles, labels = [handles[idx] for idx in order], [labels[idx] for idx in order]
    ncols = 3
    if (len(handles) == 4): ncols = 4 
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=ncols)
    plt.tight_layout()
    if (len(handles) > 4):
        plt.subplots_adjust(bottom=0.14)
    else:
        plt.subplots_adjust(bottom=0.11)
    plt.show()

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM repair", conn)

# close the database
conn.close()

# convert the json strings to vectors vector
df['iteration_time_cpu'] = df['iteration_time_cpu'].apply(json.loads)
df['iteration_time_wall'] = df['iteration_time_wall'].apply(json.loads)
df['iteration_time_wall'] = df['iteration_time_wall'].apply(lambda x: [0] + x[1:] if x else x) # set the first value to 0
df['time_cumsum'] = df['iteration_time_wall'].apply(np.cumsum)
# df['makespan'] = df['makespan'].apply(json.loads)
df['sum_of_cost'] = df['sum_of_cost'].apply(json.loads)

df['last_sum_of_cost'] = df['sum_of_cost'].apply(lambda x: x[-1] if x else None)
df["BKS"] = df.groupby(["map_name", "num_agents", "scene_name"])["last_sum_of_cost"].transform('min')
df["GAP"] = df.apply(lambda row: [(val - row["BKS"]) / row["BKS"] for val in row["sum_of_cost"]], axis=1)

df_feasible = df[df["feasible"] == 1]

group_sizes = df_feasible.groupby(["map_name", "num_agents", "seed"]).size()
total_algorithms = df["algo_name"].nunique()
valid_groups = group_sizes[group_sizes == total_algorithms].index
df_filtered = df_feasible.set_index(["map_name", "num_agents", "seed"]).loc[valid_groups]

df_feasible_grouped = df_filtered.groupby(["map_name", "num_agents", "algo_name"])

# Define the metrics to plot
metric_names = ["sum_of_cost"]

# Get unique maps
unique_maps = df["map_name"].unique()
num_maps = len(unique_maps)
x_axis_iteration = False
algos = ["p_5"]
algo_aliases = {"w_1,00" : "w = 1.0", "w_1,02" : "w = 1.02", "w_1,05" : "w = 1.05", "w_1,10" : "w = 1.1" }
times = [30, 1, 0.2, 30, 1, 1.5]
plot_algos(algos, algo_aliases, times, "GAP", df)
algos = ["lns_orig", "mapf_lns", "mine_ap", "p_1", "w_1,00_p_5"]
algo_aliases = {"lns_orig" : "MAPF-LNS2", "mapf_lns" : "original heuristic", "mine_ap" : "multiple heuristics", "p_1" : "suboptimal p = 1", "w_1,00_p_5" : "suboptimal p = 5"}
times = [30, 1, 0.2, 30, 1, 1]
plot_algos(algos, algo_aliases, times, "GAP", df)

