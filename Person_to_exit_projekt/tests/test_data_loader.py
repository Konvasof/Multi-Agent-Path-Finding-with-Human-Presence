"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Unit testy pro třídu LogData v modulu data_loader.py
 """

import sys # pro manipulaci se systémovými cestami
import os # pro manipulaci s cestami k souborům
import unittest # modul pro tvorbu unit testů

# Získáme cestu k aktuálnímu souboru (test_data_loader.py)
current_dir = os.path.dirname(os.path.abspath(__file__))
# Získáme cestu k rodičovské složce (Person_to_exit_projekt)
parent_dir = os.path.dirname(current_dir)
# Přidáme rodičovskou složku do systémové cesty, aby Python viděl data_loader.py
sys.path.append(parent_dir)

# Chci importovat LogData z data_loader.py pro oveření
from data_loader import LogData

class TestLogData(unittest.TestCase):

    def setUp(self):
        # Spustí se před každým testem
        self.raw_input = """
                        Agent 0:(6,11)->(7,11)
                        Agent 1:(9,29)->(10,29)
                        """
        self.loader = LogData(self.raw_input)

    def test_parsing_cas_0(self):
        # Ověříme, že v čase 0 jsou správně načtení agenti
        obstacles = self.loader.get_obstacles(0)
        expected = {(6, 11), (9, 29)}
        # Pokud se nerovnají, test selže
        self.assertEqual(obstacles, expected, "Chyba: Překážky v čase 0 neodpovídají.")

    def test_parsing_cas_1(self):
        # Ověříme posun v čase 1
        obstacles = self.loader.get_obstacles(1)
        expected = {(7, 11), (10, 29)}
        self.assertEqual(obstacles, expected, "Chyba: Překážky v čase 1 neodpovídají.")

    def test_max_time(self):
        # Ověříme, že parser správně zjistil maximální čas
        # Máme jen dva kroky (0 a 1), takže max index je 1
        self.assertEqual(self.loader.get_max_time(), 1)

    def test_empty_time(self):
        # Mělo by vrátit prázdný set, ne chybu, pokud požádáme o čas bez agentů
        self.assertEqual(self.loader.get_obstacles(999), set())

if __name__ == '__main__':
    # Spustí všechny testy v tomto souboru
    unittest.main()
