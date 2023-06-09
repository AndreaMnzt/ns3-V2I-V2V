import re
import matplotlib.pyplot as plt


throughput_f = open("video_throughput.txt")
throughput_ls = throughput_f.readlines()

throuhgput = []
time = []

time_pt = re.compile("^(.+?) ")
th_pt = re.compile(" , (.+?) ")


for line in throughput_ls:
  print(th_pt.findall(line))
  throuhgput.append(float(th_pt.findall(line)[0]))
  time.append(float(time_pt.findall(line)[0]))
  
print(throuhgput)

plt.plot(time, throuhgput)
plt.show()
