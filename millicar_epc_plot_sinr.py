import os
import time
import matplotlib.pyplot as plt
import numpy as np
import operator
import pandas as pd
from random import randint

### PARAMETERS TO DEFINE ###

# Define if the simulation should restart or use old data in the sinr-mcs.txt traces file
simulate = True

# Define RNTI of the devices under analysis
rnti_1 = '1'
rnti_2 = '2'


### SCRIPT ###

if simulate: 
  runs = [2]
  print(f"RngRun: {runs}")

  # Create a dictionary to record data for both devices
  exp = {}
  exp[rnti_1] = {}
  exp[rnti_2] = {}

  # Record data for each run to get an avarage at the end (when more runs are defined)
  for rndrun in runs:
    
    # ns3 command line to run
    ret = os.system(f'./waf --run "scratch/mmwave-vehicular-epc-integration.cc --rngrun={rndrun}"')
    
    print(f"Finished {rndrun}")

    curr_out = open("sinr-mcs.txt")
    
    data = curr_out.readlines()
    for line in data:
      
      # split the data to have simulation data
      l = line.split()

      # rnti of the device
      dev = l[1]
      
      # timeslot of the sinr measurement
      timeslot = float("{:.2f}".format(float(l[0])))
      


      ## Add Data to the data dictionaries
      # Get data from dev with rnti 1
      if dev == rnti_1:
        
        # add timeslot to data tictionary
        if timeslot not in exp[rnti_1]:
          exp[rnti_1][timeslot] = []  
        exp[rnti_1][timeslot].append(l[2]) 

      elif dev == rnti_2:      
        if timeslot not in exp[rnti_2]:
          exp[rnti_2][timeslot] = []  
        exp[rnti_2][timeslot].append(l[2])
      
      else:
        print("Rnti defined are not correct.")
        exit()

    curr_out.close()
    time.sleep(1)

  # elaborate data to get statistics
  time_data = {}   
  time_data[rnti_1] = []   
  time_data[rnti_2] = []   


  for dev in [rnti_1,rnti_2]:
    for t in exp[dev]:
      time_data[dev].append({
                  "time":t,
                  "sinr":np.mean([float(s) for s in exp[dev][t]]),
                  "std":np.std([float(s) for s in exp[dev][t]]),
                  "N":len([float(s) for s in exp[dev][t]])
                  })

  dev1_df = pd.DataFrame.from_dict(time_data[rnti_1])
  dev2_df = pd.DataFrame.from_dict(time_data[rnti_2])
  dev1_df.to_csv("dev1_df.csv")
  dev2_df.to_csv("dev2_df.csv")

dev1_df = pd.read_csv("dev1_df.csv")
dev2_df = pd.read_csv("dev2_df.csv")


dev1_df = dev1_df.sort_values('time')
dev2_df = dev2_df.sort_values('time')
#print(dev1_df)
#print(dev2_df)

# print final graph
plt.figure()
plt.plot(dev1_df['time'].values, dev1_df['sinr'].values, alpha=0.7, marker='x', label="SINR Vehicular Device 1")
plt.plot(dev2_df['time'].values, dev2_df['sinr'].values, alpha=0.7, marker='x', label="SINR Vehicular Device 2")
plt.legend()

plt.ylabel("SINR [dB]")
plt.xlabel("Time [s]")
#plt.title("SNR, $x_{eNB}= x_{UE} = 80m, x_{UE}=-y_{eNB}=5m$")
plt.show()
