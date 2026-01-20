import os
from Person_to_exit_projekt.python.data_loader import LogData
from Person_to_exit_projekt.python.grid import GridMap
from Person_to_exit_projekt.python.path_finder import AStarPathFinder # Změněno z path_finder na pathfinder pro konzistenci
"""
call: 
./simple_MAPF_solver -m ../../../MAPF-benchmark/mapf-map/random-32-32-10.map -a ../../../MAPF-benchmark/mapf-scen-random/scen-random/random-32-32-10-random-1.scen -k 20 -n 5 -G false --output_paths "./test"
"""
# --- KONFIGURACE SOUBORŮ ---
MAP_FOLDER = 'maps_exit_person' # Nová složka
LOG_FOLDER = 'output_paths'
# Název mapy odpovídá výstupu z grid_remaker.py
MAP_FILENAME = 'maze-32-32-2_exit_person.map' 
LOG_FILENAME = 'test' 
VIS_SUBFOLDER = 'visualizations_paths'

def load_file_content(folder, filename):
    """Pomocná funkce pro bezpečné načtení obsahu souboru."""
    base_dir = os.path.dirname(os.path.abspath(__file__))
    full_path = os.path.join(base_dir, folder, filename)
    
    if not os.path.exists(full_path):
        # POZN: Pro debugování zde zkontrolujte, zda složka maps_exit_person existuje!
        raise FileNotFoundError(f"Soubor nenalezen: {full_path}")
        
    with open(full_path, 'r', encoding='utf-8') as f:
        return f.read()

def get_special_points(raw_map_content: str) -> tuple[tuple[int, int], tuple[int, int]]:
    """
    Parsování pozic Člověka ('!') a Exitu ('X') z textové mapy.
    Vrátí: (human_start_pos, exit_goal_pos)
    """
    lines = raw_map_content.strip().split('\n')
    parsing_grid = False
    human_start = None
    exit_goal = None
    y = 0

    for line in lines:
        if line.strip().startswith('map'):
            parsing_grid = True
            continue
        
        if parsing_grid:
            if '!' in line:
                human_start = (line.index('!'), y)
            if 'X' in line:
                exit_goal = (line.index('X'), y)
            
            y += 1
            
    if not human_start or not exit_goal:
        raise ValueError("Chyba: V mapě nebyly nalezeny značky '!' (Člověk) nebo 'X' (Exit).")

    return human_start, exit_goal

def find_blocker(ideal_path, current_obstacles, log_data, time):
    """
    Hledá agenta, který blokuje cestu.
    Fáze 1: Stojí přímo na ideální trase?
    Fáze 2: Stojí hned vedle ideální trasy (v úzkém hrdle)?
    """
    if not ideal_path:
        return None, None, "NONE"

    # --- FÁZE 1: PŘÍMÁ KOLIZE ---
    for step_coords in ideal_path:
        if step_coords in current_obstacles:
            agent_id = log_data.get_agent_at(time, step_coords)
            return agent_id, step_coords, "DIRECT"

    # --- FÁZE 2: BLÍZKÉ OKOLÍ (PROXIMITY) ---
    # Pokud jsme nenašli přímou srážku, hledáme agenty "lepící se" na trasu.
    # To odhalí agenty, kteří stojí v úzké chodbě hned vedle ideální čáry.
    for step_coords in ideal_path:
        x, y = step_coords
        # Prohledáme 4-okolí (nahoře, dole, vlevo, vpravo)
        neighbors = [(x+1, y), (x-1, y), (x, y+1), (x, y-1)]
        
        for nx, ny in neighbors:
            if (nx, ny) in current_obstacles:
                # Našli jsme agenta v těsné blízkosti trasy
                agent_id = log_data.get_agent_at(time, (nx, ny))
                return agent_id, (nx, ny), "NEARBY"

    return None, None, "NONE"


def save_visualized_path(raw_map_content, human_pos, exit_pos, path, obstacles, time_step, is_success=True):
    """
    Vykreslí mapu s roboty 'R' a cestou '+'.
    Pokud is_success=False, vykresluje cestu jen dokud nenarazí na překážku.
    """
    # 1. Příprava mřížky (List of lists pro modifikaci)
    lines = raw_map_content.strip().split('\n')
    headers = []
    grid_lines = []
    parsing = False
    
    for line in lines:
        if line.strip().startswith('map'):
            parsing = True
            headers.append(line)
            continue
        if parsing:
            grid_lines.append(list(line))
        else:
            headers.append(line)

    height = len(grid_lines)
    width = len(grid_lines[0]) if height > 0 else 0

    # 2. Vykreslení robotů (R)
    # Musíme to udělat PŘED cestou, abychom detekovali kolizi
    obstacle_set = set(obstacles) # Pro rychlé vyhledávání
    for (ox, oy) in obstacles:
        if 0 <= oy < height and 0 <= ox < width:
            # Nepřepisujeme Start ani Cíl
            if (ox, oy) != human_pos and (ox, oy) != exit_pos:
                grid_lines[oy][ox] = 'R'

    # 3. Vykreslení cesty (+)
    # Pokud máme cestu (ať už statickou nebo nalezenou)
    if path and len(path) > 1:
        for i in range(1, len(path) - 1): # Přeskočíme start
            px, py = path[i]
            
            # Kontrola hranic
            if not (0 <= py < height and 0 <= px < width):
                continue

            # LOGIKA PRO PŘÍPAD NEÚSPĚCHU (NEMOŽNÉ)
            # Pokud narazíme na robota 'R', přestaneme kreslit cestu
            if not is_success and (px, py) in obstacle_set:
                # Můžeme zde volitelně dát jiný znak pro kolizi, např. '#'
                # Ale zadání znělo "dokud se o R nezastavi", takže loop ukončíme.
                break 

            # Pokud tam není zeď, start, cíl ani robot, dáme +
            current_char = grid_lines[py][px]
            if current_char not in ['@', 'T', 'X', '!', 'R']:
                grid_lines[py][px] = '+'

    # 4. Ujištění, že Start a Cíl jsou vidět (překreslíme cokoliv)
    hx, hy = human_pos
    ex, ey = exit_pos
    if 0 <= hy < height and 0 <= hx < width: grid_lines[hy][hx] = '!'
    if 0 <= ey < height and 0 <= ex < width: grid_lines[ey][ex] = 'X'

    # 5. Uložení
    output_content = "\n".join(headers) + "\n"
    output_content += "\n".join(["".join(row) for row in grid_lines])

    base_dir = os.path.dirname(os.path.abspath(__file__))
    full_vis_dir = os.path.join(base_dir, MAP_FOLDER, VIS_SUBFOLDER)
    os.makedirs(full_vis_dir, exist_ok=True)

    # Rozlišení názvu souboru podle úspěchu
    prefix = "path_viz" if is_success else "BLOCKED_viz"
    filename = f"{prefix}_t{time_step:02d}.map"
    full_path = os.path.join(full_vis_dir, filename)

    with open(full_path, 'w', encoding='utf-8') as f:
        f.write(output_content)
    
    return filename


def main():
    print("--- INICIALIZACE ---")

    # 1. Načtení a parsování Mapy
    try:
        raw_map_content = load_file_content(MAP_FOLDER, MAP_FILENAME)
        
        # Zjištění pozic START a CÍL z textu mapy
        HUMAN_START, EXIT_GOAL = get_special_points(raw_map_content)

        # Předání mapy do GridMap (značky 'X' a '!' jsou GridMap ignorovány, protože nejsou '@')
        grid = GridMap(raw_map_content) 
        
        print(f"Mapa načtena: {MAP_FILENAME} ({grid.width}x{grid.height})")
    except Exception as e:
        print(f"Chyba při načítání mapy: {e}")
        return

    # Nastavení cíle (Exit)
    try:
        # Nastavíme cíl na pozici 'X' nalezenou v mapě
        grid.set_exit(EXIT_GOAL[0], EXIT_GOAL[1]) 
        print(f"Exit nastaven na: {grid.exit_point}")
    except ValueError as e:
        print(f"Chyba nastavení exitu: {e}. Zkontrolujte, zda bod 'X' není v interiéru mapy.")
        return

    # 2. Načtení a parsování Logu Agentů
    try:
        raw_log_content = load_file_content(LOG_FOLDER, LOG_FILENAME)
        log = LogData(raw_log_content)
        max_time = log.get_max_time()
        print(f"Data agentů načtena: {LOG_FILENAME} (Max čas: {max_time})")
    except Exception as e:
        print(f"Chyba při načítání logu: {e}")
        return

    # 3. Inicializace PathFinderu
    finder = AStarPathFinder()
    
    static_path = finder.find_path(HUMAN_START, grid.exit_point, grid, [])

    if not static_path:
        print("CRITICAL ERROR: Cesta neexistuje ani v prázdné mapě!")
        return
    
    print(f"\n--- START SIMULACE (0 až {max_time}) ---")
    print(f"Člověk start: {HUMAN_START} -> Cíl: {grid.exit_point}\n")

    last_valid_path = None
    last_valid_time = -1

    for t in range(max_time + 1):
        obstacles = log.get_obstacles(t)
        path = finder.find_path(HUMAN_START, grid.exit_point, grid, obstacles)
        
        time_str = f"Čas {t:02d}:"
        
        if path:
            print(f"{time_str} Únik  MOŽNÉ")
            last_valid_path = path
            last_valid_time = t
        else:
            # --- ZDE JE ZMĚNA PRO PŘÍPADY 4, 5 ---
            # Cesta není možná. Zjistíme proč a ULOŽÍME VIZUALIZACI.
            agent_id, location, collision_type = find_blocker(static_path, obstacles, log, t)
            
            # Vytvoříme vizualizaci kolize
            # Použijeme static_path (ideální cestu), aby bylo vidět, kam chtěl jít
            vis_file = save_visualized_path(
                raw_map_content, HUMAN_START, EXIT_GOAL, 
                static_path, obstacles, t, is_success=False
            )

            msg = ""
            if agent_id is not None:
                if collision_type == "DIRECT":
                    msg = f"Blokuje Agent {agent_id} na {location}"
                else: 
                    msg = f"Blokuje Agent {agent_id} (blízko {location})"
            else:
                msg = "Komplexní blokáda (ucpáno)"

            print(f"{time_str} Únik  NEMOŽNÉ -> {msg} [Mapa: {vis_file}]")

    # --- KONEC SMYČKY (VÝPIS ÚSPĚŠNÉ CESTY) ---
    print("\n--- FINÁLNÍ VÝPIS ---")
    if last_valid_path:
        # Tady uložíme tu poslední úspěšnou cestu, abychom ji viděli taky
        # Použijeme obstacles z daného času
        final_obstacles = log.get_obstacles(last_valid_time)
        vis_file = save_visualized_path(
            raw_map_content, HUMAN_START, EXIT_GOAL, 
            last_valid_path, final_obstacles, last_valid_time, is_success=True
        )
        print(f"Poslední úspěšná cesta (t={last_valid_time}) uložena do: {vis_file}")
    else:
        print("Žádná cesta nebyla nalezena.")
if __name__ == "__main__":

    main()
