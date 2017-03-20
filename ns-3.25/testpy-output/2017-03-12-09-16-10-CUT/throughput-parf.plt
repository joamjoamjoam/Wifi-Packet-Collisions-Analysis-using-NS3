set terminal post eps color enhanced
set output "throughput-parf.eps"
set title "Throughput (AP to STA) vs time"
set xlabel "Time (seconds)"
set ylabel "Throughput (Mb/s)"
plot "-"  title "Throughput Mbits/s" with lines
5 29.0362
15 22.9018
25 22.6405
35 20.3571
45 18.176
e
