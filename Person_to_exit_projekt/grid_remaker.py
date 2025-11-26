"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Modul pro Generování a Inicializaci Mapy. 
    Načítá statickou mapu a umisťuje startovní pozici pro člověka ('!') 
    a cílový exit ('X') na volné místo na okraji mapy. Ukládá upravenou mapu pro simulaci.
 """

import os # pro manipulaci s cestami k souborům
import random # pro náhodný výběr pozic
import sys  # pro manipulaci s cestami
from grid import GridMap 

MAP_FOLDER = 'maps'
OUTPUT_FOLDER = 'maps_exit_person'
# Definujeme soubor, který má být zpracován
DEFAULT_INPUT_FILENAME = 'maze-32-32-2.map' 

def load_file_content(folder, filename):
    # Pomocná funkce pro bezpečné načtení obsahu souboru. Sestaví cestu a ověří existenci.

    base_dir = os.path.dirname(os.path.abspath(__file__))
    full_path = os.path.join(base_dir, folder, filename)
    
    if not os.path.exists(full_path):
        # Přesná cesta k souboru je důležitá pro debugování
        raise FileNotFoundError(f"Soubor nenalezen: {full_path}")
        
    with open(full_path, 'r', encoding='utf-8') as f:
        return f.read()


class MapModifier:
    
    def __init__(self, raw_map_content: str, input_filename: str):
        # Inicializace s načtenou mapou
        self.raw_map_content = raw_map_content
        self.input_filename = input_filename
        # Vytvoření instance GridMap pro kontrolu průchodnosti
        self.grid = GridMap(raw_map_content)
        # Místa pro člověka a exit
        self.human_pos = None
        self.exit_pos = None

    def find_random_walkable_spot(self):
        # Najde náhodnou volnou buňku, kde není zeď TODO: optimalizovat výběr
        walkable_spots = []
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                if self.grid.is_walkable(x, y):
                     walkable_spots.append((x, y))
        
        if not walkable_spots:
            raise RuntimeError("Na mapě nebylo nalezeno žádné volné místo!")
            
        return random.choice(walkable_spots)

    def find_random_edge_spot(self):
        # Najde náhodnou volnou buňku na okraji mapy TODO: optimalizovat výběr
        edge_spots = []
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                is_on_edge = (x == 0 or x == self.grid.width - 1 or
                              y == 0 or y == self.grid.height - 1)
                
                if is_on_edge and self.grid.is_walkable(x, y):
                    edge_spots.append((x, y))

        if not edge_spots:
            raise RuntimeError("Na okraji mapy nebylo nalezeno žádné volné místo!")
        
        return random.choice(edge_spots)

    def generate_and_save(self, output_folder=OUTPUT_FOLDER):
        # Vygeneruje pozice X, ! a uloží upravenou mapu do souboru.
        # Určení pozic exit a human
        self.human_pos = self.find_random_walkable_spot()
        self.exit_pos = self.find_random_edge_spot()
       
        # Kontrola, aby X a ! nebyly na stejném místě
        while self.human_pos == self.exit_pos:
            self.human_pos = self.find_random_walkable_spot()

        # Úprava textové mapy pro vložení '!' a 'X'
        lines = self.raw_map_content.strip().split('\n')
        output_lines = []
        parsing_grid = False
        y = 0 
        
        for line in lines:
            if line.strip().startswith('map'):
                parsing_grid = True
                output_lines.append(line)
                continue
            
            if parsing_grid:
                line_list = list(line)
                
                # Umístění '!' (Člověk)
                if y == self.human_pos[1]:
                    x_human = self.human_pos[0]
                    if x_human < len(line_list):
                        line_list[x_human] = '!'
                    
                # Umístění 'X' (Exit)
                if y == self.exit_pos[1]:
                    x_exit = self.exit_pos[0]
                    if x_exit < len(line_list):
                        line_list[x_exit] = 'X'
                    
                output_lines.append("".join(line_list))
                y += 1 # Posun na další řádek (Y)
            else:
                output_lines.append(line)

        # Zápis výsledku a uložení do nového souboru
        base_name = os.path.splitext(self.input_filename)[0]
        new_filename = f"{base_name}_exit_person.map"
        
        # Zajištění existence výstupní složky
        base_dir = os.path.dirname(os.path.abspath(__file__))
        full_output_dir = os.path.join(base_dir, output_folder)
        os.makedirs(full_output_dir, exist_ok=True) # Vytvoří složku, pokud neexistuje

        output_path = os.path.join(full_output_dir, new_filename) # Cesta k výstupnímu souboru

        # Zápis do souboru
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(output_lines))
            
        return self.human_pos, self.exit_pos, new_filename

if __name__ == "__main__":
    
    # Načtení dat
    try:
        raw_content = load_file_content(MAP_FOLDER, DEFAULT_INPUT_FILENAME)
    except FileNotFoundError as e:
        print(f"Chyba při spuštění: {e}")
        print(f"Ujistěte se, že soubor '{DEFAULT_INPUT_FILENAME}' je ve složce '{MAP_FOLDER}/'.")
        sys.exit(1)
    
    # Inicializace a generování
    try:
        modifier = MapModifier(raw_content, DEFAULT_INPUT_FILENAME)
        
        print(f"--- START: Generování mapy z '{DEFAULT_INPUT_FILENAME}' ---")

        human_pos, exit_pos, new_filename = modifier.generate_and_save()
        
        print(f"\n Mapa úspěšně uložena do: {OUTPUT_FOLDER}/{new_filename}")
        print(f"   Pozice Člověk ('!'): {human_pos}")
        print(f"   Pozice Exit ('X'): {exit_pos}")
        
    except RuntimeError as e:
        print(f"\nCHYBA GENERÁTORU: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"\nNastala neočekávaná chyba: {e}")
        sys.exit(1)
