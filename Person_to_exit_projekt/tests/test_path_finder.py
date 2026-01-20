import sys
import os
import unittest

# Magie s cestami
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
sys.path.append(parent_dir)

from Person_to_exit_projekt.python.grid import GridMap
from Person_to_exit_projekt.python.path_finder import AStarPathFinder

class TestAStar(unittest.TestCase):

    def setUp(self):
        # Mapa 5x5 bez zdí (zdi přidáme v testu nebo jsou prázdné)
        raw_map = """type octile
height 5
width 5
map
.....
.....
.....
.....
.....
"""
        self.grid = GridMap(raw_map)
        self.finder = AStarPathFinder()

    def test_simple_path(self):
        """Cesta z (0,0) do (0,4) v prázdné mapě."""
        start = (0, 0)
        goal = (0, 4)
        obstacles = set() # Žádní agenti
        
        path = self.finder.find_path(start, goal, self.grid, obstacles)
        
        self.assertIsNotNone(path)
        self.assertEqual(len(path), 5) # (0,0), (0,1), (0,2), (0,3), (0,4)
        self.assertEqual(path[-1], goal)

    def test_avoid_dynamic_obstacle(self):
        """
        Start (0,0), Cíl (2,0).
        Cesta by byla rovně: (0,0)->(1,0)->(2,0).
        Ale na (1,0) postavíme Agenta.
        A* musí jít okolo: (0,0)->(0,1)->(1,1)->(2,1)->(2,0) (nebo podobně).
        """
        start = (0, 0)
        goal = (2, 0)
        
        # Agent blokuje přímou cestu
        agents = {(1, 0)} 
        
        path = self.finder.find_path(start, goal, self.grid, agents)
        
        self.assertIsNotNone(path)
        # Ověříme, že cesta nejde přes (1,0)
        self.assertNotIn((1, 0), path)
        self.assertEqual(path[-1], goal)

    def test_no_path(self):
        """Cíl je obklíčen agenty."""
        start = (0, 0)
        goal = (4, 4)
        
        # Uděláme zeď z agentů kolem startu
        agents = {(0, 1), (1, 0)}
        
        path = self.finder.find_path(start, goal, self.grid, agents)
        self.assertIsNone(path)

if __name__ == '__main__':
    unittest.main()
