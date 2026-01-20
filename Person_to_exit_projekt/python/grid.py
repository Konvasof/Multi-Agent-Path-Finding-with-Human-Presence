"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Manages the static topology of the grid environment.
    This class parses input map files (header + ASCII grid layout) and stores 
    fixed wall coordinates in an optimized set structure.
 """

class GridMap:

    def __init__(self, raw_map_content: str):
        self.width = 0
        self.height = 0
        self.walls = set()  # (x, y)
        self.exit_point = None    
        self._parse_map(raw_map_content)

    def _parse_map(self, content: str):
        """ Parses the map string to set dimensions and wall positions."""     
        lines = content.strip().split('\n')  # split file to lines, for better processing
        parsing_grid = False      
        y = 0  # Starting y coordinate

        for line in lines:
            line = line.strip() # delete whitespace
            if not line:
                continue
            
            if line.startswith('height'):
                self.height = int(line.split()[1])
                continue
            elif line.startswith('width'): 
                self.width = int(line.split()[1])
                continue
            elif line.startswith('map'):
                parsing_grid = True
                continue
            
            if parsing_grid:
                for x, char in enumerate(line):
                    if char == '@':
                        self.walls.add((x, y))
                y += 1

    def set_exit(self, x: int, y: int):
        """ Sets exit 'X' point on the map. Not in another wall or outside bounds. """  
        if not self.in_bounds(x, y):
            raise ValueError(f"Exit {x},{y} is outside the map.")
        self.walls.discard((x, y))  # Ensure exit is not a wall
        self.exit_point = (x, y)

    def in_bounds(self, x: int, y: int) -> bool:
        """ Touple (x,y) is/isn't inside the map boundaries."""  
        return 0 <= x < self.width and 0 <= y < self.height

    def is_walkable(self, x: int, y: int) -> bool:
        """ Is the touple (x,y) walkable (inside bounds and not a wall)."""
        return self.in_bounds(x, y) and (x, y) not in self.walls
    