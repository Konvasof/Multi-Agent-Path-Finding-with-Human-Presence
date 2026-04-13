import json
import glob
import pandas as pd

data = []
for file in glob.glob("log_*.json"):
    with open(file, "r") as f:
        log = json.load(f)
        data.append({
            "Map": log["experiment"]["map"],
            "Agents": log["experiment"]["agents"],
            "Cost": log["results"]["sum_of_costs"],
            "Safety_Enabled": log["safety"]["enabled"]
        })

df = pd.DataFrame(data)
print(df)
