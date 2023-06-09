/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*/  


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include <ns3/buildings-helper.h>
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include <ns3/mmwave-vehicular-helper2.h>
#include <ns3/point-to-point-helper.h>

#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include "ns3/channel-condition-model.h"

using namespace ns3;
using namespace mmwave;

uint32_t g_rxPackets; // total number of received packets
uint32_t g_txPackets; // total number of transmitted packets
uint32_t g_clientrxPackets; // total number of received packets


Time g_firstReceived; // timestamp of the first time a packet is received
Time g_lastReceived; // timestamp of the last received packet

static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p)
{
 g_rxPackets++;
 SeqTsHeader header;

 p->PeekHeader(header);

 *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << "\t" << header.GetSeq() << "\t" << header.GetTs().GetSeconds() << std::endl;

 if (g_rxPackets > 1)
 {

   g_lastReceived = Simulator::Now();
 }
 else
 {
   g_firstReceived = Simulator::Now();
 }
}

static void Tx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p)
{
 g_txPackets++;
}


static void echoRx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p)
{
 g_clientrxPackets++;
}

int
main (int argc, char *argv[])
{

  // mmwave setup

  //CommandLine cmd;
  //cmd.Parse (argc, argv);

  /* Information regarding the traces generated:
   *
   * 1. UE_1_SINR.txt : Gives the SINR for each sub-band
   *    Subframe no.  | Slot No. | Sub-band  | SINR (db)
   *
   * 2. UE_1_Tb_size.txt : Allocated transport block size
   *    Time (micro-sec)  |  Tb-size in bytes
   * */

  NS_LOG_COMPONENT_DEFINE ("V2Xexample");

  // system parameters
  double bandwidth = 1e8; // bandwidth in Hz
  double frequency = 28e9; // the carrier frequency
  uint32_t numerology = 3; // the numerology

  // applications
  uint32_t packetSize = 1024; // UDP packet size in bytes
  uint32_t startTime = 10; // application start time in milliseconds
  uint32_t endTime = 500; // application end time in milliseconds
  uint32_t interPacketInterval = 30; // interpacket interval in microseconds

  // mobility
  double speed = 20; // speed of the vehicles m/s
  double intraGroupDistance = 1; // distance between two vehicles belonging to the same group

  CommandLine cmd;
  //
  cmd.AddValue ("bandwidth", "used bandwidth", bandwidth);
  cmd.AddValue ("iip", "inter packet interval, in microseconds", interPacketInterval);
  cmd.AddValue ("intraGroupDistance", "distance between two vehicles belonging to the same group, y-coord", intraGroupDistance);
  cmd.AddValue ("numerology", "set the numerology to use at the physical layer", numerology);
  cmd.AddValue ("frequency", "set the carrier frequency", frequency);
  cmd.Parse (argc, argv);


  Config::SetDefault ("ns3::MmWaveSidelinkMac::UseAmc", BooleanValue (true));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue (frequency));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::Bandwidth", DoubleValue (bandwidth));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::Numerology", UintegerValue (numerology));

  Config::SetDefault ("ns3::MmWaveVehicularNetDevice::RlcType", StringValue("LteRlcUm"));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::SchedulingPatternOption", EnumValue(2)); // use 2 for SchedulingPatternOption=OPTIMIZED, 1 or SchedulingPatternOption=DEFAULT
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (500*1024));


  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);

  
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0));

  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (80.0, 0.0, 0.0));

  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);
  
  NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);

  ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
  ptr_mmWave->EnableTraces ();


  // millicar setup 

  // create the nodes
  NodeContainer n;
  n.Create (2);
  // create the mobility models
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (n);

  n.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (20,0,0));
  n.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, speed, 0));

  n.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (20, intraGroupDistance,  0));
  n.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, speed, 0));

  // create and configure the helper
  Ptr<millicar::MmWaveVehicularHelper2> helper = CreateObject<millicar::MmWaveVehicularHelper2> ();
  
  
  helper->SetSpectrumChannel(ptr_mmWave->GetSpectrumChannel(0));
  

  // choose the spectrum propagation loss model
  helper->SetNumerology (3);

  helper->SetPropagationLossModelType ("ns3::ThreeGppPropagationLossModel");

  // choose the pathloss model to use
  helper->SetPathlossModelType ("ns3::ThreeGppUmaPropagationLossModel");

  // choose the channel condition model to use
  helper->SetChannelConditionModelType ("ns3::ThreeGppUmaChannelConditionModel");

  // choose the spectrum propagation loss model
  helper->SetChannelModelType ("ns3::ThreeGppSpectrumPropagationLossModel");
  
  helper->SetSpectrumPropagationLossModelType ("ns3::ThreeGppSpectrumPropagationLossModel");
  NetDeviceContainer devs = helper->InstallMmWaveVehicularNetDevices (n);

  // Install the TCP/IP stack in the two nodes

  InternetStackHelper internet;
  internet.Install (n);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devs);

  // Need to pair the devices in order to create a correspondence between transmitter and receiver
  // and to populate the < IP addr, RNTI > map.
  helper->PairDevices(devs);

  // Set the routing table
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (n.Get (0)->GetObject<Ipv4> ());
  staticRouting->SetDefaultRoute (n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () , 2 );

  NS_LOG_DEBUG("IPv4 Address node 0: " << n.Get (0)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());
  NS_LOG_DEBUG("IPv4 Address node 1: " << n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());

  Ptr<mmwave::MmWaveAmc> m_amc = CreateObject <mmwave::MmWaveAmc> (helper->GetConfigurationParameters());

  // setup the applications
  Config::SetDefault ("ns3::UdpEchoClient::MaxPackets", UintegerValue (0xFFFFFFFF));
  Config::SetDefault ("ns3::UdpEchoClient::Interval", TimeValue (MicroSeconds (interPacketInterval)));
  Config::SetDefault ("ns3::UdpEchoClient::PacketSize", UintegerValue (packetSize));

  // create the applications
  uint32_t port = 4000;

  UdpEchoServerHelper server (port);
  ApplicationContainer echoApps = server.Install (n.Get (1));
  echoApps.Start (Seconds (0.0));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("simple-one-stats.txt");
  echoApps.Get(0)->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&Rx, stream));

  UdpEchoClientHelper client (n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), port);
  ApplicationContainer apps = client.Install (n.Get (0));
  
  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("simple-two-stats.txt");
  Ptr<OutputStreamWrapper> stream3 = asciiTraceHelper.CreateFileStream ("simple-three-stats.txt");


  apps.Get(0)->TraceConnectWithoutContext("Tx", MakeBoundCallback(&Tx, stream2));
  apps.Get(0)->TraceConnectWithoutContext("Rx", MakeBoundCallback(&echoRx, stream3));


  // Activate a data radio bearer for mmwave devices
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);

  // set the application start/end time
  apps.Start (MilliSeconds (startTime));

  
  
  Simulator::Stop (MilliSeconds (endTime + 1000));
  Simulator::Run ();
  Simulator::Destroy ();

  
  std::cout << "----------- Statistics -----------" << std::endl;
  std::cout << "Packets size:\t\t" << packetSize << " Bytes" << std::endl;
  std::cout << "Packets received:\t" << g_rxPackets << std::endl;
  std::cout << "Average Throughput:\t" << (double(g_rxPackets)*(double(packetSize)*8)/double( g_lastReceived.GetSeconds() - g_firstReceived.GetSeconds()))/1e6 << " Mbps" << std::endl;

  std::cout << "Trasmitted: " << g_txPackets << std::endl;
  std::cout << "Echo received: " << g_clientrxPackets << std::endl;


  return 0;
}
