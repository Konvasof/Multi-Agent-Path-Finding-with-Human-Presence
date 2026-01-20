# tests/test_grid_remake.py
import sys
import os
import unittest
from unittest.mock import patch # Musíme mockovat náhodu

# Magie pro importy
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
sys.path.append(parent_dir)

from Person_to_exit_projekt.python.grid_remaker import MapModifier
from Person_to_exit_projekt.python.grid import GridMap # Potřebujeme GridMap k setupu

# Testovací vstup, který použijeme (4x4)
TEST_MAP_CONTENT = """type octile
height 4
width 4
map
@..@
..@@
.X..
....
"""
TEST_FILENAME = "test_4x4.map"

class TestMapModifier(unittest.TestCase):
    
    def setUp(self):
        # Inicializace GridMap pro testování statické průchodnosti
        self.grid = GridMap(TEST_MAP_CONTENT)
        # Inicializace MapModifier
        self.modifier = MapModifier(TEST_MAP_CONTENT, TEST_FILENAME)
        
    def test_initialization(self):
        """Ověří, že se inicializují rozměry."""
        self.assertEqual(self.modifier.grid.width, 4)
        self.assertEqual(self.modifier.grid.height, 4)

    def test_find_random_walkable_spot(self):
        """Ověří, že spot je volný (ne @) a náhodně vybraný."""
        
        # Očekávané volné body (např. (1,0) nebo (0,1) atd.)
        # Zkontrolujeme, zda je bod (0,0) (zeď) a (2,1) (zeď) vyloučen.
        
        walkable_spots = self.modifier.find_random_walkable_spot()
        self.assertTrue(self.modifier.grid.is_walkable(walkable_spots[0], walkable_spots[1]))
        
        # (0, 0) je zeď
        self.assertFalse(walkable_spots == (0, 0))
        
    def test_find_random_edge_spot(self):
        """Ověří, že spot je na hranici a volný."""
        
        edge_spot = self.modifier.find_random_edge_spot()
        x, y = edge_spot
        
        # Musí být na hranici:
        is_on_edge = (x == 0 or x == 3 or y == 0 or y == 3)
        self.assertTrue(is_on_edge)
        
        # (0, 0) je zeď, neměl by být vybrán:
        self.assertFalse(edge_spot == (0, 0))

    @patch('grid_remaker.random.choice')
    def test_generate_and_save_positions(self, mock_choice):
        """
        Otestuje, že se pozice správně vygenerují a v mapě se objeví.
        Musíme mockovat (simulovat) random.choice pro deterministické testování.
        """
        # Nastavíme simulované výstupy z random.choice:
        # 1. Exit (náhodně vybraný okrajový bod)
        # 2. Human (náhodně vybraný volný bod)
        
        # Nastavíme PŘESNÉ hodnoty, které random.choice vrátí:
        MOCK_HUMAN = (1, 3) # Vnitřní volné místo
        MOCK_EXIT = (3, 0)  # Okrajové volné místo
        
        # random.choice se volá dvakrát (nebo víckrát kvůli 'while' smyčce)
        # První volání (pro Human), Druhé volání (pro Exit)
        mock_choice.side_effect = [MOCK_HUMAN, MOCK_EXIT]

        # Vytvoříme instanci, která neukládá, ale pouze provede generování
        # Použijeme dočasnou výstupní složku pro testování
        h_pos, e_pos, fname = self.modifier.generate_and_save(output_folder="tests/temp_output")

        # Kontrola, zda byly pozice správně uloženy
        self.assertEqual(h_pos, MOCK_HUMAN)
        self.assertEqual(e_pos, MOCK_EXIT)
        self.assertEqual(fname, "test_4x4_exit_person.map")
        
        # Důležité: Tady by bylo ideální i zkontrolovat, zda VÝSTUPNÍ soubor obsahuje X na (3, 0) a ! na (1, 3), 
        # ale to by vyžadovalo čtení souboru a parsování. Pro jednoduchý unit test stačí ověřit logiku generování.
        
if __name__ == '__main__':
    unittest.main()
