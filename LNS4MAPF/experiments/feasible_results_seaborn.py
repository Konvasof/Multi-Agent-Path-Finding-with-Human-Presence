import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
exp_name = "feasible_restarts"
df = pd.read_sql_query("SELECT * FROM " + exp_name, conn)
df["delay"] = df["init_sol_sum_of_cost"] - df["sum_of_dist"]

# close the database
conn.close()

df_feasible = df[df["feasible"] == 1]

group_sizes = df_feasible.groupby(["map_name", "num_agents", "seed"]).size()
total_algorithms = df["algo_name"].nunique()
valid_groups = group_sizes[group_sizes == total_algorithms].index
df_filtered = df_feasible.set_index(["map_name", "num_agents", "seed"]).loc[valid_groups]
df_filtered = df_filtered.reset_index()

# Define the metrics to plot
metric_names = ["feasible", "experiment_time_cpu", "experiment_time_wall", "expanded", "init_sol_sum_of_cost", "delay"]

# Get unique maps
unique_maps = df_filtered["map_name"].unique()
num_maps = len(unique_maps)

# Iterate over each metric
for metric in metric_names:
    fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(6, 12), sharey=False)
    axes = axes.flatten()

    # Dummy plot for legend (don't display it, just get handles)
    sns.lineplot(
        data=df_filtered,
        x="num_agents",
        y=metric,
        hue="algo_name",
        estimator="mean",
        legend=True,  # Allow this one to generate the legend
        color='none',  # Set to 'none' to not display the line
        ax=axes[0]  # We'll use the first axis to get the legend handles
    )
    axes[0].get_legend().set_visible(False)
    handles, labels = axes[0].get_legend_handles_labels()

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    for i, (map_name, map_group) in enumerate(df.groupby('map_name')):
        ax = axes[i]
        ax.grid()
        sns.lineplot(data=map_group, x="num_agents", y=metric, estimator="mean", errorbar=None, hue="algo_name", ax=ax, legend=False)

    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.05),ncol=2)
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.20)
    fig.suptitle(f"{metric} Across Maps", fontsize=16)
    plt.show()


