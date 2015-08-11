./run-benchmark.sh > results.50000/results.naildns
for i in $(seq 6); do
    ./run-benchmark.sh >> results.50000/results.naildns
done

