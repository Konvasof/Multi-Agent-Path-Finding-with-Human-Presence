"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Unit testy pro třídu GridMap, modul grid.py
 """

import sys # pro manipulaci se systémovými cestami
import os # pro manipulaci s cestami k souborům
import unittest # modul pro tvorbu unit testů

# Získáme cestu k aktuálnímu souboru (test_grid.py)
current_dir = os.path.dirname(os.path.abspath(__file__))
# Získáme cestu k rodičovské složce (Person_to_exit_projekt)
parent_dir = os.path.dirname(current_dir)
# Přidáme rodičovskou složku do systémové cesty, aby Python viděl grid.py
sys.path.append(parent_dir)

# Chci importovat GridMap z grid.py pro oveření
from grid import GridMap

class TestGridMap(unittest.TestCase):

    def setUp(self):
        # Vytvoříme malou testovací mapu 4x4
        self.raw_map = """type octile
height 4
width 4
map
@..@
....
.@@.
....
"""
        self.grid = GridMap(self.raw_map)

    def test_dimensions(self):
        # Ověříme rozměry
        self.assertEqual(self.grid.width, 4)
        self.assertEqual(self.grid.height, 4)

    def test_walls_parsing(self):
        # Ověříme, že zdi jsou správně načteny
        # Řádek 0: Zeď na (0,0) a (3,0)
        self.assertIn((0, 0), self.grid.walls)
        self.assertIn((3, 0), self.grid.walls)       
        # Řádek 2: Zeď na (1,2) a (2,2)
        self.assertIn((1, 2), self.grid.walls)       
        # Řádek 1 je prázdný
        self.assertFalse((0, 1) in self.grid.walls)

    def test_bounds(self):
        # Kontrola hranic
        self.assertTrue(self.grid.in_bounds(2, 2))
        self.assertFalse(self.grid.in_bounds(-1, 0)) # Vlevo mimo
        self.assertFalse(self.grid.in_bounds(0, 5))  # Dole mimo
        self.assertFalse(self.grid.in_bounds(4, 0))  # Vpravo mimo (indexuje se 0..3)

    def test_is_walkable(self):
        # Kombinace bounds + walls
        # (0,0) je zeď -> False
        self.assertFalse(self.grid.is_walkable(0, 0))
        # (1,0) je volno -> True
        self.assertTrue(self.grid.is_walkable(1, 0))
        # (-1,0) je mimo -> False
        self.assertFalse(self.grid.is_walkable(-1, 0))

    def test_set_exit(self):
        # Nastavení exitu
        self.grid.set_exit(1, 1)
        self.assertEqual(self.grid.exit_point, (1, 1))       
        # Pokus nastavit exit do zdi by měl vyhodit chybu
        with self.assertRaises(ValueError):
            self.grid.set_exit(0, 0)

if __name__ == '__main__':
    unittest.main()
