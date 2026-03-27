import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Načtení dat
df = pd.read_csv("second_experiment_time_results.csv")

# Získáme seznam všech unikátních map, co se testovaly
maps = df['Map'].unique()

# Nastavíme elegantní styl
sns.set_theme(style="whitegrid")

# Vytvoříme okno (figure) s podgrafy (subplots) vedle sebe. Kolik map, tolik grafů.
fig, axes = plt.subplots(1, len(maps), figsize=(6 * len(maps), 5), sharey=True)

# Pokud testuješ jen jednu mapu, axes není pole, tak ho zabalíme
if len(maps) == 1:
    axes = [axes]

# Vykreslíme graf pro každou mapu zvlášť
for ax, map_name in zip(axes, maps):
    # Data jen pro aktuální mapu
    map_data = df[df['Map'] == map_name]

    # Spočítáme průměrnou úspěšnost pro danou kombinaci Času a Agentů
    success_rates = map_data.groupby(['TimeLimit', 'Agents'])['Success'].mean().reset_index()

    # Samotné vykreslení čar
    sns.lineplot(
        data=success_rates,
        x='TimeLimit',
        y='Success',
        hue='Agents',
        marker='o',         # Dělá tečky na jednotlivých bodech
        palette='tab10',    # Hezká paleta barev pro čáry
        linewidth=2,
        ax=ax
    )

    # Popisky a formátování
    ax.set_title(f'SUCCESS RATE IN TIME: {map_name}', fontsize=14, fontweight='bold')
    ax.set_xlabel('Max Time Limit (s)', fontsize=12)
    ax.set_ylabel('Success rate', fontsize=12)
    
    # Vynutíme, aby se na ose X ukázaly přesně tyto čísla
    ax.set_xticks([30, 60, 90, 120, 150]) 
    
    # Osu Y pevně ukotvíme od 0 do 1 (0% až 100%)
    ax.set_ylim(-0.05, 1.05) 

    # Upravíme legendu, aby byla přehledná
    ax.legend(title='Count of robots', loc='lower right')

# Uložení a zobrazení
plt.tight_layout()
plt.savefig("time_limit_graphs.png", dpi=300)
plt.show()
