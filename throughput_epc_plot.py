import re
import matplotlib.pyplot as plt


throughput_f = open("epc_throughput.txt")
throughput_ls = throughput_f.readlines()[::1]
throughput_ls = throughput_ls[2:160]
throuhgput = []
time = []

time_pt = re.compile("^(.+?) ")
th_pt = re.compile(" , (.+?) ")


for line in throughput_ls:
  #print(th_pt.findall(line))
  throuhgput.append(float(th_pt.findall(line)[0]))
  time.append(float(time_pt.findall(line)[0]))


throughput_f2 = open("epc_throughput2.txt")
throughput_ls2 = throughput_f2.readlines()[::1]
throughput_ls2 = throughput_ls2[2:160]

throuhgput2 = []
time2 = []

time_pt = re.compile("^(.+?) ")
th_pt = re.compile(" , (.+?) ")


for line in throughput_ls2:
  #print(th_pt.findall(line))
  throuhgput2.append(float(th_pt.findall(line)[0]))
  time2.append(float(time_pt.findall(line)[0]))



plt.figure()
plt.plot(time, throuhgput, alpha=0.7, label="Throughput Vehicular Device 1")
plt.plot(time2, throuhgput2, alpha=0.7, label="Throughput Vehicular Device 2")
plt.legend()

plt.ylabel("Throughput [Mbps]")
plt.xlabel("Time [s]")
#plt.title("Throughput, $x_{eNB}= x_{UE} = 80m, x_{UE}=-y_{eNB}=5m$")
plt.show()


#plt.plot(time, throuhgput)
#plt.show()
