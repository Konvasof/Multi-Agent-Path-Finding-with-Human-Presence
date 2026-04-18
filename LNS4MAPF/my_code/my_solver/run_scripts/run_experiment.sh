#!/bin/bash

OUTPUT_DIR="logy_stare"

MAP="../../MAPF-benchmark/mapf-map/warehouse-10-20-10-2-1.map"
SCENARIO="../../MAPF-benchmark/mapf-scen-even/scen-even/warehouse-10-20-10-2-1-even-1.scen"
TIME_LIMIT=60

AGENTS=(60 80 100)
SEEDS=(1 2 3)

mkdir -p "$OUTPUT_DIR"

echo ">>> ZAČÍNÁM EXPERIMENT <<<"

for a in "${AGENTS[@]}"; do
    for s in "${SEEDS[@]}"; do
        echo "--------------------------------------------------------"
        echo "Spouštím: Agenti = $a | Seed = $s"
        echo "--------------------------------------------------------"
        ./build/simple_MAPF_solver -m "$MAP" -a "$SCENARIO" -k "$a" -n 10 --seed "$s" --safetyCheck 1 --timeLimit "$TIME_LIMIT" -i 100000000 -G 0
    done
done
echo ">>> Výpočty hotovy. Přesouvám logy do složky: $OUTPUT_DIR <<<"
mv log_*.json "$OUTPUT_DIR"/ 2>/dev/null
echo ">>> EXPERIMENT DOKONČEN <<<"