"""
Author: Sofie Konvalinová
Date: 18-11-2025
Email: konvasof@cvut.cz
Description:
    Unit tests for class LogData in module data_loader.py.
 """

import sys 
import os 
import unittest # module for unit testing

current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
sys.path.append(parent_dir) # parent directory to see data_loader.py

from Person_to_exit_projekt.python.data_loader import LogData

class TestLogData(unittest.TestCase):

    def setUp(self):
        """Sets up a standard test case with two agents having paths of equal length."""
        self.raw_input = """
                        Agent 0:(6,11)->(7,11)
                        Agent 1:(9,29)->(10,29)
                        """
        self.loader = LogData(self.raw_input)

    def test_parsing_cas_0(self):
        """Verifies that initial positions (time 0) are correctly parsed for all agents."""
        obstacles = self.loader.get_obstacles(0)
        expected = {(6, 11), (9, 29)}
        self.assertEqual(obstacles, expected, "Error: Překážky v čase 0 neodpovídají.")

    def test_parsing_cas_1(self):
        """Verifies that positions at time 1 are correctly parsed."""
        obstacles = self.loader.get_obstacles(1)
        expected = {(7, 11), (10, 29)}
        self.assertEqual(obstacles, expected, "Error: Překážky v čase 1 neodpovídají.")

    def test_max_time(self):
        """Checks if the maximum time step is correctly calculated based on the path length."""
        self.assertEqual(self.loader.get_max_time(), 1)

    def test_empty_time(self):
        self.assertEqual(self.loader.get_obstacles(999), set())

if __name__ == '__main__':
    unittest.main()
