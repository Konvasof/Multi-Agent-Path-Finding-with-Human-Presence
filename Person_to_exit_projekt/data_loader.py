"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Třída pro zpracování logů pohybu agentů. z test souboru
    Vstup: Raw string z textového souboru.
    Výstup: Slovník, kde klíčem je čas a hodnotou je seznam překážek.

call: 
./simple_MAPF_solver -m ../../../MAPF-benchmark/mapf-map/random-32-32-10.map -a ../../../MAPF-benchmark/mapf-scen-random/scen-random/random-32-32-10-random-1.scen -k 20 -n 5 -G false --output_paths "./test"
 """

import re # modul nezbytný pro efektivní parsování textových řetězců
from collections import defaultdict # modul co umožňuje vytvořit slovník, který při přístupu ke neexistujícímu klíči automaticky vytvoří výchozí hodnotu
    
class LogData:

    def __init__(self, raw_log_content: str):
        self.raw_data = raw_log_content
        # slovník, kde klíč bude čas t a hodnota bude seznam souřadnic
        self.obstacles_at_time = defaultdict(list) 
        # provedeme parsování hned při inicializaci
        self.agent_lookup = {}
        self._parse_data()
           
    def _parse_data(self):
        # rozdělí na řádky a odstraní bílé znaky
        lines = self.raw_data.strip().split('\n')
        # sekvence pohybů všech agentů
        all_agent_paths = [] # Zde uložíme sekvence pohybů
        
        # Sběr sekvencí pohybů agentů, přeskočí prázdné řádky
        for line in lines: 
            if not line.strip():
                continue
            
            # Hledáme všechny (x,y) páry na řádku a uložíme si do matches
            matches = re.findall(r'\((\d+),(\d+)\)', line)
            path = [(int(x), int(y)) for x, y in matches]
            
            if path:
                all_agent_paths.append(path) # Uložíme sekvenci pohybů tohoto agenta

        if not all_agent_paths:
            return

        # Maximální čas simulace, který budeme brát v úvahu
        max_t_logged = max(len(path) - 1 for path in all_agent_paths)
        
        #  rozšíření pozic agentů na celý časový rozsah, robot nezmizí, pokud je na konci cesty
        for path in all_agent_paths:
            t_last = len(path) - 1
            Pos_last = path[-1]

            # Skutečný pohyb (t=0 až t=t_last)
            for t, pos in enumerate(path):
                self.obstacles_at_time[t].append(pos)
            
            # Agent stojící na místě (t=t_last + 1 až t=max_t_logged)
            for t in range(t_last + 1, max_t_logged + 1):
                self.obstacles_at_time[t].append(Pos_last)
                
    
    def get_obstacles(self, time_t: int) -> set:
        # Vrátí set souřadnic agentů v daném čase
        # set převádí seznam na množinu, čímž odstraní duplicitní souřadnice a umožní rychlé vyhledávání
        return set(self.obstacles_at_time[time_t])

    def get_max_time(self) -> int:
        # Vrátí maximální čas, pro který máme záznamy o překážkách
        if not self.obstacles_at_time:
            return 0
        return max(self.obstacles_at_time.keys())
    
    def get_agent_at(self, time: int, pos: tuple[int, int]) -> int:
        # Vrátí ID agenta na dané pozici a čase. Pokud tam nikdo není, vrátí None.      
        # Klíč do slovníku musí odpovídat formátu při ukládání: (t, x, y)
        key = (time, pos[0], pos[1])
        return self.agent_lookup.get(key, None)
    