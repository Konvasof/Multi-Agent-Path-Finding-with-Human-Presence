
import heapq
from typing import List, Tuple, Set, Optional
# Importujeme jen pro type hinting, v běhu to bude instance předaná jako argument
from Person_to_exit_projekt.python.grid import GridMap 

class AStarPathFinder:
    """
    Implementace algoritmu A* (A-Star) pro hledání nejkratší cesty na mřížce.
    """

    @staticmethod
    def heuristic(a: Tuple[int, int], b: Tuple[int, int]) -> int:
        """
        Vypočítá Manhattan distance (vzdálenost v pravoúhlé síti).
        Vzorec: |x1 - x2| + |y1 - y2|
        Pro pohyb do 4 směrů je toto přesnější než Euklidovská vzdálenost.
        """
        (x1, y1) = a
        (x2, y2) = b
        return abs(x1 - x2) + abs(y1 - y2)

    def find_path(self, 
                  start: Tuple[int, int], 
                  goal: Tuple[int, int], 
                  grid_map: GridMap, 
                  dynamic_obstacles: Set[Tuple[int, int]]) -> Optional[List[Tuple[int, int]]]:
        """
        Najde cestu ze start do goal s vyhnutím se statickým i dynamickým překážkám.
        
        Args:
            start: (x, y) začátek
            goal: (x, y) cíl
            grid_map: Instance mapy (pro kontrolu zdí a hranic)
            dynamic_obstacles: Set souřadnic (x, y), kde jsou v TENTO MOMENT agenti
            
        Returns:
            List souřadnic [(x,y), (x,y)...] představující cestu, nebo None, pokud cesta neexistuje.
        """
        
        # Rychlá kontrola: Start nebo Cíl jsou ve zdi/překážce?
        if not grid_map.is_walkable(start[0], start[1]) or start in dynamic_obstacles:
            return None
        if not grid_map.is_walkable(goal[0], goal[1]) or goal in dynamic_obstacles:
            return None

        # Priority Queue: ukládá n-tice (f_score, x, y). 
        # heapq automaticky řadí podle první položky (f_score).
        open_set = []
        heapq.heappush(open_set, (0, start))
        
        # Slovník 'came_from' slouží k rekonstrukci cesty zpětně
        came_from = {}
        
        # g_score: Cena cesty ze startu do daného bodu. Defaultně nekonečno.
        g_score = {start: 0}
        
        # f_score: g_score + heuristika. Defaultně nekonečno.
        f_score = {start: self.heuristic(start, goal)}
        
        # Množina pro rychlou kontrolu, co už je v open_setu (heapq to neumí efektivně)
        open_set_hash = {start}

        while open_set:
            # Vybereme uzel s nejnižším f_score
            current_f, current = heapq.heappop(open_set)
            open_set_hash.remove(current)

            # JSME V CÍLI?
            if current == goal:
                return self._reconstruct_path(came_from, current)

            # Projdeme sousedy (4 směry: Nahoru, Dolů, Vlevo, Vpravo)
            neighbors = [
                (current[0], current[1] - 1), # Up
                (current[0], current[1] + 1), # Down
                (current[0] - 1, current[1]), # Left
                (current[0] + 1, current[1])  # Right
            ]

            for neighbor in neighbors:
                nx, ny = neighbor
                
                # 1. Je soused na mapě a není to zeď?
                if not grid_map.is_walkable(nx, ny):
                    continue
                
                # 2. Je soused volný od agentů?
                if (nx, ny) in dynamic_obstacles:
                    continue

                # Cena přechodu na souseda je vždy +1
                tentative_g_score = g_score[current] + 1

                # Pokud jsme našli lepší cestu k tomuto sousedovi (nebo jsme tu prvně)
                if tentative_g_score < g_score.get(neighbor, float('inf')):
                    # Uložíme, odkud jsme přišli
                    came_from[neighbor] = current
                    g_score[neighbor] = tentative_g_score
                    f = tentative_g_score + self.heuristic(neighbor, goal)
                    f_score[neighbor] = f
                    
                    if neighbor not in open_set_hash:
                        heapq.heappush(open_set, (f, neighbor))
                        open_set_hash.add(neighbor)

        # Pokud dojde fronta a cíl nenalezen
        return None

    def _reconstruct_path(self, came_from: dict, current: Tuple[int, int]) -> List[Tuple[int, int]]:
        """Zpětně poskládá cestu od cíle ke startu."""
        total_path = [current]
        while current in came_from:
            current = came_from[current]
            total_path.append(current)
        return total_path[::-1] # Otočíme seznam (Start -> Cíl)
    