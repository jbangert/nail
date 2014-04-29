grep qps $1 | awk '{print $4};'  > $1.qps
grep 'RTT average' $1 | awk '{print $3}' > $1.rttavg
