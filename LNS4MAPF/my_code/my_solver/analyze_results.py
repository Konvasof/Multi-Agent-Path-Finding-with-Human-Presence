import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("experiment_results.csv")

sns.set_theme(style="whitegrid")
fig, axes = plt.subplots(1, 3, figsize=(18, 5))

# 1.Úspěšnost 
success_rates = df.groupby(['Agents', 'Map'])['Success'].mean().reset_index()
sns.barplot(data=success_rates, x='Agents', y='Success', hue='Map', ax=axes[0])
axes[0].set_title('Úspěšnost LNS se Safety filtrem')
axes[0].set_ylabel('Success Rate (0.0 - 1.0)')

# 2.Čas výpočtu
successful_runs = df[df['Success'] == True]
sns.boxplot(data=successful_runs, x='Agents', y='Time_s', hue='Map', ax=axes[1])
axes[1].set_title('Výpočetní čas (sekundy)')
axes[1].set_ylabel('Čas (s)')

# 3.Cena
sns.lineplot(data=successful_runs, x='Agents', y='Cost', hue='Map', marker='o', ax=axes[2])
axes[2].set_title('Kvalita řešení (Sum of Costs)')
axes[2].set_ylabel('Cena')


plt.tight_layout()
plt.savefig("experiment_graphs.png", dpi=300)
plt.show()
