#!/bin/bash

Type_of_map=test_empty
Even_or_odd=even
Number_of_scenario=1

MAP="../../MAPF-benchmark/mapf-map/${Type_of_map}.map"
SCENARIO="../../MAPF-benchmark/mapf-scen-${Even_or_odd}/scen-${Even_or_odd}/${Type_of_map}-${Even_or_odd}-${Number_of_scenario}.scen"

ROBOT_COUNTS=10
NEIGHBORHOOD_SIZE=5
GUI=1
TIME_LIMIT=30
MAX_ITERATIONS=100000000
SEED=2
SAFETY_CHECK=1
DESTROY_OPERATOR=RANDOM
OUTPUT_PATH=output.txt

./build/simple_MAPF_solver -m "$MAP" -a "$SCENARIO" -k "$ROBOT_COUNTS" -n "$NEIGHBORHOOD_SIZE" --seed "$SEED" --safetyCheck "$SAFETY_CHECK" --timeLimit "$TIME_LIMIT" -G "$GUI" -i "$MAX_ITERATIONS"
