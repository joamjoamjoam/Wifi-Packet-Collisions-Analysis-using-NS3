set terminal post eps color enhanced
set output "throughput-aparf.eps"
set title "Throughput (AP to STA) vs time"
set xlabel "Time (seconds)"
set ylabel "Throughput (Mb/s)"
plot "-"  title "Throughput Mbits/s" with lines
5 29.0362
15 23.7538
25 23.163
35 19.1302
45 14.1773
e
