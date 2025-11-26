"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Třída pro správu statické topologie mřížky. 
    Zpracovává vstupní mapy ve formátu (header + ASCII mřížka) 
    a ukládá pevné zdi do optimalizované množiny. Poskytuje robustní 
    metody pro kontrolu průchodnosti a hranic, které využívá PathFinder
 """

class GridMap:

    def __init__(self, raw_map_content: str):
        self.width = 0
        self.height = 0
        self.walls = set()  # Uložíme souřadnice (x, y), kde je zeď
        self.exit_point = None # Cíl nastavíme později     
        self._parse_map(raw_map_content)

    def _parse_map(self, content: str):
        lines = content.strip().split('\n')      
        # Čekáme na začátek mřížky 
        parsing_grid = False
        y = 0 # Řádek v mřížce
        
        for line in lines:
            line = line.strip() # ostrání bílé znaky
            if not line:
                continue
            
            # Parsování hlavičky
            if line.startswith('height'):
                self.height = int(line.split()[1])
                continue
            elif line.startswith('width'):
                self.width = int(line.split()[1])
                continue
            elif line.startswith('map'):
                parsing_grid = True # Začínáme parsovat mřížku
                continue
            
            # Parsování samotné mřížky
            if parsing_grid:
                # Projdeme znaky v řádku
                for x, char in enumerate(line):
                    if char == '@': # @ i T se často berou jako zdi
                        self.walls.add((x, y))
                y += 1

    def set_exit(self, x: int, y: int):
        # Nastaví cíl
        # Nesmí být ve zdi ani mimo mapu
        if not self.in_bounds(x, y):
            raise ValueError(f"Exit {x},{y} je mimo mapu!")
        if (x, y) in self.walls:
            raise ValueError(f"Exit {x},{y} je ve zdi!")
        self.exit_point = (x, y)

    def in_bounds(self, x: int, y: int) -> bool:
        # Zkontroluje, zda jsou souřadnice uvnitř mapy
        return 0 <= x < self.width and 0 <= y < self.height

    def is_walkable(self, x: int, y: int) -> bool:
        # Zkontroluje, zda je pozice průchozí (uvnitř mapy a není zeď)
        return self.in_bounds(x, y) and (x, y) not in self.walls
    