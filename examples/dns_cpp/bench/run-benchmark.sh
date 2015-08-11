sudo ../server naildns.zone 1>&2 &
sleep 5
queryperf/queryperf -l 60 2>&1 < queryperf.input
