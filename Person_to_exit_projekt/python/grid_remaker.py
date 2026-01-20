"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Map RE-Generation and Initialization Module.
    Loads the static map structure and automates the placement of the agent's 
    start position ('!') and the target exit ('X').
Terminal Usage:
    python main.py my_custom_map.map --input_dir maps/maze-32-32-2.map --output_dir maps_exit_person/maze-32-32-2_exit_person.map
 """

import os 
import random 
import sys  
import argparse # taking arguments from terminal
from Person_to_exit_projekt.python.grid import GridMap 

MAP_FOLDER = 'maps' # if not specified in terminal arguments
OUTPUT_FOLDER = 'maps_exit_person' # if not specified in terminal arguments

def load_file_content(folder, filename):
    """Helper function to safely load file content."""
    base_dir = os.path.dirname(os.path.abspath(__file__))
    full_path = os.path.join(base_dir, folder, filename)
    
    if not os.path.exists(full_path):
        raise FileNotFoundError(f"File not found: {full_path}")       
    with open(full_path, 'r', encoding='utf-8') as f:
        return f.read()

class MapModifier:
    
    def __init__(self, raw_map_content: str, input_filename: str):
        self.raw_map_content = raw_map_content
        self.input_filename = input_filename
        self.grid = GridMap(raw_map_content)
        self.human_pos = None
        self.exit_pos = None

    def find_random_walkable_spot(self):
        """ Scans the entire grid to identify all walkable coordinates and randomly returns coordinates."""
        walkable_spots = []
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                if self.grid.is_walkable(x, y):
                     walkable_spots.append((x, y))
        
        if not walkable_spots:
            raise RuntimeError("No walkable spots found on the map!")     
              
        return random.choice(walkable_spots)

    def find_random_edge_spot(self):
        """Finds a random walkable cell on the perimeter of the map."""
        # Najde náhodnou volnou buňku na okraji mapy TODO: optimalizovat výběr
        edge_spots = []
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                is_on_edge = (x == 0 or x == self.grid.width - 1 or
                              y == 0 or y == self.grid.height - 1)
                
                if is_on_edge and self.grid.is_walkable(x, y):
                    edge_spots.append((x, y))

        if not edge_spots:
            raise RuntimeError("No walkable cell on the perimeter of the map.")
        
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
    # 1. Definice argumentů příkazové řádky
    parser = argparse.ArgumentParser(description="Map Generator: Adds Agent (!) and Exit (X) to a map.")
    
    # Povinný argument: jméno souboru
    parser.add_argument("filename", type=str, help="Name of the input map file (e.g., berlin_1_256.map)")
    
    # Volitelné argumenty: složky (mají defaultní hodnoty)
    parser.add_argument("--input_dir", type=str, default=MAP_FOLDER, help="Folder containing input maps")
    parser.add_argument("--output_dir", type=str, default=OUTPUT_FOLDER, help="Folder for saving modified maps")

    # Zpracování argumentů
    args = parser.parse_args()
    input_filename: str = args.filename
    input_dir: str = args.input_dir
    output_dir: str = args.output_dir
    # Načtení dat
    try:
        raw_content = load_file_content(input_dir, input_filename)
    except FileNotFoundError as e:
        print(f"Error: {e}")
        print(f"Check if '{input_filename}' exists in '{input_dir}/'.")
        sys.exit(1)
    
    # Inicializace a generování
    try:
        modifier = MapModifier(raw_content, input_filename)
        print(f"--- START: Generating map from '{input_filename}' ---")
        human_pos, exit_pos, new_filename = modifier.generate_and_save(output_folder=output_dir)
        print(f"\n Map successfully saved to: {output_dir}/{new_filename}")
        print(f"   Human Position ('!'): {human_pos}")
        print(f"   Exit Position ('X'):  {exit_pos}")       
    except RuntimeError as e:
        print(f"\nCHYBA GENERÁTORU: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"\nNastala neočekávaná chyba: {e}")
        sys.exit(1)
