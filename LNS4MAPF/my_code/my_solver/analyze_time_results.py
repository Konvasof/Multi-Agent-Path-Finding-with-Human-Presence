import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("15h_experiment_time_results.csv")

sns.set_theme(style="whitegrid")

maps = df['Map'].unique()

for map_name in maps:
    map_data = df[df['Map'] == map_name]
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    fig.suptitle(f'Analýza mapy: {map_name}', fontsize=16, fontweight='bold')
    
    success_data = map_data.groupby(['Agents', 'TimeLimit'])['Success'].mean().reset_index()
    sns.lineplot(
        data=success_data, x='Agents', y='Success', hue='TimeLimit', 
        marker='o', ax=axes[0], palette='tab10', linewidth=2
    )
    axes[0].set_title('Úspěšnost nalezení řešení', fontsize=12)
    axes[0].set_ylabel('Success rate (0.0 - 1.0)')
    axes[0].set_xlabel('Počet agentů')
    axes[0].set_ylim(-0.05, 1.05)
    
    sns.lineplot(
        data=map_data, x='Agents', y='RealTime_s', hue='TimeLimit', 
        marker='o', ax=axes[1], palette='tab10', linewidth=2
    )
    axes[1].set_title('Skutečný výpočetní čas (s)', fontsize=12)
    axes[1].set_ylabel('Čas (s)')
    axes[1].set_xlabel('Počet agentů')
    
    success_only = map_data[map_data['Success'] == True]
    if not success_only.empty:
        sns.lineplot(
            data=success_only, x='Agents', y='Cost', hue='TimeLimit', 
            marker='o', ax=axes[2], palette='tab10', linewidth=2
        )
        axes[2].set_title('Cena řešení (Sum of Costs)', fontsize=12)
        axes[2].set_ylabel('Cost')
        axes[2].set_xlabel('Počet agentů')
    else:
        axes[2].set_title('Cena řešení (Žádná úspěšná data)', fontsize=12)
        
    plt.tight_layout()
    plt.show()