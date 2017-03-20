set terminal post eps color enhanced
set output "power-aparf.eps"
set title "Average transmit power (AP to STA) vs time"
set xlabel "Time (seconds)"
set ylabel "Power (mW)"
plot "-"  title "Average Transmit Power" with lines
5 0.737904
15 4.45312
25 20.4121
35 16.5655
45 19.4897
e
