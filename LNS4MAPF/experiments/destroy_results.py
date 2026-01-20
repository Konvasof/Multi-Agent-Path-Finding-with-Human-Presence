import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import json
import re

def str_vec_to_float_vec(vec):
    return [float(x) for x in vec]

def find_last_number(s: str) -> int:
    """Extracts the last number from a string. Returns -1 if no number is found."""
    numbers = re.findall(r'\d+', s)  # Find all numbers in the string
    if numbers:
        return int(numbers[-1])
    return -1

def find_last_number2(s: str) -> int:
    """Extracts the last number from a string. Returns -1 if no number is found."""
    numbers = re.findall(r'\d+', s)  # Find all numbers in the string
    if numbers:
        return (int(numbers[-1]), s)
    return (-1, s) 

def sort_strings_by_last_number(strings: list) -> list:
    """Sorts a list of strings based on the last number found in each string."""
    return sorted(strings, key=find_last_number2)

def argsort_strings_by_last_number(strings: list) -> list:
    """Sorts a list of strings based on the last number found in each string."""
    rng  = list(range(len(strings)))
    return sorted(rng, key=lambda a, strings=strings : find_last_number2(strings[a]))

def average_runs(x, times, max_t):
    t = np.linspace(0, max_t, 1000)
    costs = np.array([np.interp(t, time, run) for time, run in zip(times, x)])
    average_costs = np.mean(costs, axis=0)
    
    return t, average_costs

def plot_algos(algorithms, algo_aliases, neighborhood_sizes, metric, df_filtered):
    # iterate over neighborhood sizes
    for neighborhood_size in neighborhood_sizes:
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
                if find_last_number(algo_name) != neighborhood_size:
                    continue
                display_name = algo_name
                for a in algo_aliases.keys():
                    if a in algo_name:
                        display_name = algo_aliases[a]
                        break
                found = False
                for alg in algorithms:
                    if alg in algo_name:
                        found = True
                        break
                if not found: continue
                metric_list = algo_group[metric].to_list()
                max_len = np.max(np.asarray([len(x) for x in metric_list]))
                metric_list = [x + [x[-1] for l in range(max_len - len(x))] for x in metric_list]
                metric_np = np.asarray(metric_list)
                mean_vec = np.mean(metric_np, axis=0)
                ax.plot(np.asarray(range(mean_vec.shape[0])), mean_vec, label=f"{display_name}", linewidth=2.5)

            ax.set_title(map_name)
            ax.set_xlabel("Iteration [-]")
            ax.set_ylabel("GAP [-]")
            ax.grid(True)

# Put a legend below current axis
        handles, labels = axes[0].get_legend_handles_labels()
        order = np.argsort(labels)
        handles, labels = [handles[idx] for idx in order], [labels[idx] for idx in order]
        ncols = 3
        if (len(handles) == 4): ncols = 2
        fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=ncols)
        plt.tight_layout()
        if (len(handles) >= 4):
            plt.subplots_adjust(bottom=0.14)
        else:
            plt.subplots_adjust(bottom=0.12)
        plt.show()

def plot_algos_time(algorithms, algo_aliases, max_times, metric, df_filtered):
    # iterate over neighborhood sizes
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
            display_name = algo_name
            for a in algo_aliases.keys():
                if a in algo_name:
                    display_name = algo_aliases[a]
                    break
            found = False
            for alg in algorithms:
                if alg in algo_name:
                    found = True
                    break
            if not found: continue
            metric_list = algo_group[metric].to_list()
            time_list = algo_group["time_cumsum"].to_list()
            mean_time, mean_run = average_runs(metric_list, time_list, max_times[i])
            ax.plot(mean_time, mean_run, label=f"{display_name}", linewidth=2.5)

        ax.set_title(map_name)
        ax.set_xlabel("Time [s]")
        ax.set_ylabel("GAP [-]")
        ax.grid(True)

# Put a legend below current axis
    handles, labels = axes[0].get_legend_handles_labels()
    order = argsort_strings_by_last_number(labels)
    handles, labels = [handles[idx] for idx in order], [labels[idx] for idx in order]
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=3)
    plt.tight_layout()
    if (len(handles) > 5):
        plt.subplots_adjust(bottom=0.14)
    else:
        plt.subplots_adjust(bottom=0.12)
    plt.show()

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM destroy", conn)

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
# df["GAP"] = (df["sum_of_cost"] - df["BKS"]) / df["BKS"]
df["GAP"] = df.apply(lambda row: [(val - row["BKS"]) / row["BKS"] for val in row["sum_of_cost"]], axis=1)

df_feasible = df[df["feasible"] == 1]

group_sizes = df_feasible.groupby(["map_name", "num_agents", "seed"]).size()
total_algorithms = df["algo_name"].nunique()

# Get unique maps
unique_maps = df["map_name"].unique()
num_maps = len(unique_maps)

# the influence of adaptivness
algos1 = ["Random", "RANDOM_n", "RandomWalk", "RANDOMWALK", "Intersection", "INTERSECTION"]
algo_aliases = {"Adaptive_" : "adaptive original", "Random_" : "random original", "RandomWalk_" : "randomwalk original", "Intersection_" : "intersection original", "ADAPTIVE_" : "adaptive mine", "RANDOM_n" : "random mine", "RANDOMWALK_" : "randomwalk mine", "RANDOM_CHOOSE_" : "randomchoose", "INTERSECTION_" : "intersection mine", "BLOCKED_":"blocked"}
plot_algos(algos1, algo_aliases, [4], "GAP", df)
algos2 = ["Adaptive", "ADAPTIVE", "RANDOM_CHOOSE", "RandomWalk"]
plot_algos(algos2, algo_aliases, [8], "GAP", df)
algos3 = ["RandomWalk", "RANDOMWALK", "BLOCKED"]
plot_algos(algos3, algo_aliases, [4, 8, 16], "GAP", df)
algo_aliases = {"RandomWalk_neighborhood_size_4" : "MAPF-LNS2 N = 4", "RandomWalk_neighborhood_size_8" : "MAPF-LNS2 N = 8", "RandomWalk_neighborhood_size_16" : "MAPF-LNS2 N = 16", "RANDOMWALK_neighborhood_size_4" : "my solver N = 4", "RANDOMWALK_neighborhood_size_8" : "my solver N = 8", "RANDOMWALK_neighborhood_size_16" : "my solver N = 16"}
algos4 = ["RANDOMWALK_neighborhood_size_4", "RANDOMWALK_neighborhood_size_8", "RANDOMWALK_neighborhood_size_16", "RandomWalk_neighborhood_size_4", "RandomWalk_neighborhood_size_8", "RandomWalk_neighborhood_size_16"]
plot_algos_time(algos4, algo_aliases, [45, 1, 0.2, 55, 1, 2], "GAP", df)
algo_aliases = {"BLOCKED_neighborhood_size_4" : "blocked N = 4", "BLOCKED_neighborhood_size_8" : "blocked N = 8", "BLOCKED_neighborhood_size_16" : "blocked N = 16", "RandomWalk_neighborhood_size_4" : "randomwalk N = 4", "RandomWalk_neighborhood_size_8" : "randomwalk N = 8", "RandomWalk_neighborhood_size_16" : "randomwalk N = 16", "RANDOMWALK_neighborhood_size_16" : "randomwalk N = 16"}
algos5 = ["BLOCKED_neighborhood_size_4", "BLOCKED_neighborhood_size_8", "BLOCKED_neighborhood_size_16", "RandomWalk_neighborhood_size_4", "RandomWalk_neighborhood_size_8", "RANDOMWALK_neighborhood_size_16"]
plot_algos_time(algos5, algo_aliases, [45, 1, 0.2, 55, 1, 2], "GAP", df)
