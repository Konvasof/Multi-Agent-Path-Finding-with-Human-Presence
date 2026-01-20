"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    A class for processing agent movement logs from a test file.
    Input: Raw string content from a text file.
    Output: A dictionary mapping time to a list of obstacles.
 """

import re # modul of regular expressions for pattern matching
from collections import defaultdict # modul for default dictionary
    
class LogData:

    def __init__(self, raw_log_content: str):
        self.raw_data = raw_log_content
        self.obstacles_at_time = defaultdict(list) # dictionary mapping time t to list of obstacle positions
        self.agent_lookup = {}
        self._parse_data() # parse the raw data upon initialization
           
    def _parse_data(self):
        """ Returns a set containing positions of all agents at the given time 'time_t'."""
        
        lines = self.raw_data.strip().split('\n') # Parses the raw log 
        all_agent_paths = [] # sequences of movements for all agents
        
        for line in lines: 
            if not line.strip(): # delete empty lines
                continue
            
            matches = re.findall(r'\((\d+),(\d+)\)', line) # finds all (x,y) pairs in the line
            path = [(int(x), int(y)) for x, y in matches] # save as list of tuples
            
            if path:
                all_agent_paths.append(path) 

        if not all_agent_paths:
            raise ValueError(f"No agent paths found in log data.")

        max_t_logged = max(len(path) - 1 for path in all_agent_paths) # Maximal time step across all agents
        
        # Build a lookup for agent positions at each time step, they will not desapear after final move
        for path in all_agent_paths:
            t_last = len(path) - 1
            Pos_last = path[-1]

            for t, pos in enumerate(path): # Agent moving along its path
                self.obstacles_at_time[t].append(pos)
            
            for t in range(t_last + 1, max_t_logged + 1): # Agent stays at last position
                self.obstacles_at_time[t].append(Pos_last)              
    
    def get_obstacles(self, time_t: int) -> set:
        """ Returns a set of unique obstacle coordinates at time 'time_t'."""
        return set(self.obstacles_at_time[time_t])

    def get_max_time(self) -> int:
        """ Gets the last recorded time step from the obstacle data."""
        if not self.obstacles_at_time:
            return 0
        return max(self.obstacles_at_time.keys())
    
    def get_agent_at(self, time: int, pos: tuple[int, int]) -> int:
        """ Return ID of the agent at the given coordinates and time step."""
        key = (time, pos[0], pos[1])
        return self.agent_lookup.get(key, None)
    