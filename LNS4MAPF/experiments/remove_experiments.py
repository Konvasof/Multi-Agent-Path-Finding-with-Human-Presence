import os
import json
import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# experiment_name = "feasible_restarts"
# experiment_name = "repair"
experiment_name = "destroy"
# experiment_name = "overall_improvement"
# algo_names = ["SIPP_suboptimal_ap_w_1,00_p_5"]
# algo_names = ["SIPP_lns_orig"]
# algo_names = ["SIPP_mine", "SIPP_suboptimal_w_1,00", "SIPP_suboptimal_w_1,01", "SIPP_suboptimal_w_1,02", "SIPP_suboptimal_w_1,05", "SIPP_suboptimal_w_1,10"]
# algo_names = ["SIPP_mapf_lns", "SIPP_mine_ap", "SIPP_suboptimal_ap"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_BLOCKED_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_BLOCKED_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_BLOCKED_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_BLOCKED_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_ORIGO_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_RANDOM_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_RANDOM_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_RANDOM_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_RANDOM_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_INTERSECTION_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_INTERSECTION_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_INTERSECTION_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_INTERSECTION_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_RANDOMWALK_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_RANDOMWALK_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_RANDOMWALK_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_RANDOMWALK_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_ADAPTIVE_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_ADAPTIVE_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_ADAPTIVE_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_ADAPTIVE_neighborhood_size_32"]
# algo_names = ["SIPP_suboptimal_w_1,00_destroy_type_RANDOM_CHOOSE_neighborhood_size_4", "SIPP_suboptimal_w_1,00_destroy_type_RANDOM_CHOOSE_neighborhood_size_8","SIPP_suboptimal_w_1,00_destroy_type_RANDOM_CHOOSE_neighborhood_size_16", "SIPP_suboptimal_w_1,00_destroy_type_RANDOM_CHOOSE_neighborhood_size_32"]

# Count total rows before deletion
cursor.execute("SELECT COUNT(*) FROM " + experiment_name)
total_before = cursor.fetchone()[0]
deleted = 0

for algo_name in algo_names:
    cursor.execute("DELETE FROM " + experiment_name + """ WHERE 
        algo_name = ?
    """, (algo_name,))
    deleted += cursor.rowcount  # Number of deleted rows

conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Removed: " + str(deleted) + "/" + str(total_before) + " experiments")
