import os
import json
import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# experiment_name = "influence_of_ap"
# experiment_name = "repair"
# experiment_name = "destroy"
# experiment_name = "feasible_restarts"
# experiment_name = "sipp_pp"
experiment_name = "overall_improvement"


# Count total rows before deletion
cursor.execute("SELECT COUNT(*) FROM " + experiment_name)
total_before = cursor.fetchone()[0]
deleted = 0

cursor.execute("DELETE FROM " + experiment_name)
deleted = total_before

conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Removed: " + str(deleted) + "/" + str(total_before) + " experiments")
