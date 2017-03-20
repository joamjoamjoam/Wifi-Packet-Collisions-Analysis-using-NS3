set terminal post eps color enhanced
set output "power-parf.eps"
set title "Average transmit power (AP to STA) vs time"
set xlabel "Time (seconds)"
set ylabel "Power (mW)"
plot "-"  title "Average Transmit Power" with lines
5 1.09814
15 4.23876
25 19.634
35 32.9513
45 33.8462
e
