# Integration of Mmwave and Millicar modules #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") module for the simulation
of 5G cellular and vehicular networks operating at mmWaves.

Features:

* The module provides an integration of mmwave and millicar modules to simulate vehicular scenarios with Vehicle to Vehicle (V2V) and Vehicle to Infrastructure (V2I) communication. This project is discribed in the related project paper, to get a copy please contact Andrea Manzato (GitHub: AndreaMnzt).

  
## Installation
This repository contains a complete ns-3 installation with the addition of the mmwave and millicar modules. 

Use these commands to download and build `ns3-V2I-V2V`:
```
git clone https://github.com/AndreaMnzt/ns3-V2I-V2V.git
cd ns3-V2I-V2V
./waf configure --disable-python --enable-examples && ./waf build
```

## Usage example 
The following commands outputs SINR data and throughput data, which can be plot through the plot below,
The SINR data are saved in the sinr-mcs.txt, which is the standard output file for the millicar module,

1. Scenario A:

./waf --run scratch/mmwave-vehicular-video-integration.cc

plot:
python3 millicar_video_plot_sinr.py
// python3 throughput_video_plot.py //not used

2. Scenario B:

// The following command is not necessary:
// you just need to run the first epc plot python script, which will run internally the 
// ./waf --run scratch/mmwave-vehicular-epc-integration.cc 

// plot ecp scripts:
python3 millicar_epc_plot_sinr.py //this will also run the simulation
python3 throughput_epc_plot.py 

## Related modules
- MilliCar is an ns-3 module for the simulation of mmWave NR V2X networks. Check [this repo](https://github.com/signetlabdei/millicar) for further details.
- A seperate module is being developed for [mmWave UE Energy Consumption](https://github.com/arghasen10/mmwave-energy "mmwave-energy"). You can use this module for analyzing 
Energy Consumption behaviour of mmwave UE. Check this repository for further details.
- `ns3-mmwave-iab` is an extended version of `ns3-mmWave` adding wireless relaying capabilities to an ns-3 NetDevice, and the possibility of simulating in-band relaying at mmWave frequencies. Check [this repo](https://github.com/signetlabdei/ns3-mmwave-iab) for further details.

## References 
The following papers describe in detail the features implemented in the mmWave
module:
- [End-to-End Simulation of 5G mmWave Networks](https://ieeexplore.ieee.org/document/8344116/ "comst paper") is a comprehensive tutorial with a detailed description of the whole module. We advise the researchers interested in this module to start reading from this paper;
- [Integration of Carrier Aggregation and Dual Connectivity for the ns-3 mmWave Module](https://arxiv.org/abs/1802.06706 "wns3 2018") describes the Carrier Aggregation implementation;
- [Implementation of A Spatial Channel Model for ns-3](https://arxiv.org/abs/2002.09341 "wns3 2020") describes the integration of the spatial channel model based on the 3GPP specifications TR 38.901 V15.0.0;
- [Performance Comparison of Dual Connectivity and Hard Handover for LTE-5G Tight Integration](https://arxiv.org/abs/1607.05425 "simutools paper") describes the Dual Connectivity feature.

These other papers describe features that were implemented in older releases: 
- [ns-3 Implementation of the 3GPP MIMO Channel Model for Frequency Spectrum above 6 GHz](https://dl.acm.org/citation.cfm?id=3067678 "wns3 2017") describes the implementation of the 3GPP channel model based on TR 38.900;
- [Multi-Sector and Multi-Panel Performance in 5G mmWave Cellular Networks](https://arxiv.org/abs/1808.04905 "globecom2018") describes the multi-sector addition to the 3GPP channel model;


## About
The mmwave and millicar modules are being developed by [NYU Wireless](http://wireless.engineering.nyu.edu/) and the [University of Padova](http://mmwave.dei.unipd.it/).


<!-- The new-handover branch offers integration between LTE and mmWave and dual connectivity features.
 -->

## Authors ##

The ns-3 mmWave module is the result of the development effort carried out by different people. The main contributors are: 
- Tommaso Zugno, University of Padova
- Michele Polese, University of Padova
- Matteo Pagin, University of Padova
- Mattia Lecci, University of Padova
- Matteo Drago, University of Padova
- Mattia Rebato, University of Padova
- Menglei Zhang, NYU Wireless
- Marco Giordani, University of Padova
- Marco Mezzavilla, NYU Wireless
- Sourjya Dutta, NYU Wireless
- Russell Ford, NYU Wireless
- Gabriel Arrobo, Intel

The integration of MmWave and MilliCar modules in this repository was developed by Andrea Manzato, with the support of professors and assistants from University of Padua (UniPd).

## License ##

This software is licensed under the terms of the GNU GPLv2, as like as ns-3. See the LICENSE file for more details.

## Author Notes

This extension was developed for the Network Analysis and Simulation course project. 
The aim of the project is to integrate mmwave and millicar simulation in the same simulation script to evaluate interference effects.
An example of the integration can be found in the following files:
- Scenario A: scratch/mmwave-vehicular-video-integration.cc
- Scenario B: scratch/mmwave-vehicular-epc-integration.cc
This module DOES NOT integrate millicar and mmwave communication in the same node (e.g. a vehicle can still not communicate with an eNB),
However this might be achieved by integrate mmwave and millicar netdevice instances in the same Node.

If you need help to understand how this module was developed or you want to get in touch, please open an issue on this repository or PM me on LinkedIn.
All files in this repository are distributed in the hope that they will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

This program is distributed under the same license and conditions of ns-3 simulator, MilliCar and MmWave module at 24 July 2022.

Per aspera ad astra.
Andrea Manzato