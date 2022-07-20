#!/usr/bin/env python3

import matplotlib.pyplot as plt
import re

# to be runned after ./waf --run scratch/mmwave-vehicular-video-integration.cc 

def main():

  # time | rnti | 10 * std::log10 (sinrAvg) | numSym | tbSize | mcs 

  f = open("sinr-mcs.txt")
  lines = f.readlines()

  times = {}
  sinr = {}

  #parse lines
  for line in lines:
    l = line.split()

    if len(l) != 6:
      print(f"ERROR, len={len(l)}")
      exit

    rnti = l[1]

    # add times to times dictionary
    if rnti not in times:
      times[rnti] = []
    
    t = l[0]
    times[rnti].append(float(t))

    # add sirn to sinr dictionary
    if rnti not in sinr:
      sinr[rnti] = []
    
    s = l[2]
    sinr[rnti].append(float(s))
  
  
  plt.figure()
  print(len(times['1']))
  print(len(times['2']))

  for rnti in ['1']:
    plt.plot(times[rnti], sinr[rnti], alpha=0.5, marker='x', label="SINR Vehicular Device 1")

  for rnti in ['2']:
  
    plt.plot(times[rnti], sinr[rnti], alpha=0.7, marker='x', label="SINR Vehicular Device 2")
  

  plt.legend()
  #plt.title("SNR, $x_{eNB}= x_{UE} = 40m, x_{UE}=-y_{eNB}=10m$")

  plt.ylabel("SINR [dB]")
  plt.xlabel("Time [s]")
  plt.show()


  f.close()


if __name__=="__main__":
  main()

