/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 University of Padova, Dep. of Information Engineering,
*   SIGNET lab.
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
*/

#include "ns3/mmwave-sidelink-spectrum-phy.h"
#include "ns3/mmwave-vehicular-net-device.h"
#include "ns3/mmwave-vehicular-helper2.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/mobility-module.h"
#include "ns3/isotropic-antenna-model.h"
#include "ns3/spectrum-helper.h"
#include "ns3/mmwave-spectrum-value-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/node-list.h"

#include <ns3/mmwave-helper.h>
#include <ns3/mmwave-point-to-point-epc-helper.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/buildings-helper.h>

NS_LOG_COMPONENT_DEFINE ("VehicularSimpleTwo");

using namespace ns3;
using namespace millicar;


uint32_t g_rxPackets = 0; // total number of received packets
uint32_t g_txPackets; // total number of transmitted packets
uint32_t g_clientrxPackets; // total number of received packets
uint32_t g_old_rxPackets;
uint32_t packetSize =1024; // UDP packet size in bytes

u_int32_t deltaTime = 50; //milliseconds
Time g_firstReceived; // timestamp of the first time a packet is received
Time g_lastReceived; // timestamp of the last received packet

static void Rx (Ptr<const Packet> p)
{
 g_rxPackets++;
 SeqTsHeader header;
 p->PeekHeader(header);

 if (g_rxPackets > 1)
 {    
    g_lastReceived = Simulator::Now();
 }
 else
 {
   g_firstReceived = Simulator::Now();
 }

}

void ThroughputComputation (Ptr<OutputStreamWrapper> stream)
{
    *stream->GetStream ()<<Simulator::Now ().GetSeconds ()<<" , "<< ((g_rxPackets - g_old_rxPackets)*(double(packetSize)*8)/(double(deltaTime)/1000) )/1e6 << " Mbps " << g_rxPackets << std::endl;
    g_old_rxPackets = g_rxPackets;
    
    Simulator::Schedule( MilliSeconds(deltaTime), &ThroughputComputation, stream);
}

void PrintGnuplottableNodeListToFile (std::string filename);
int main (int argc, char *argv[])
{ 
 
  bool channel_shared = true;

  SeedManager::SetSeed (1);
  // system parameters
  double bandwidth = 100.0e6; // bandwidth in Hz
  double frequency = 28e9; // the carrier frequency
  uint32_t numerology = 3; // the numerology

  // applications
  uint32_t packetSize =1024; // UDP packet size in bytes
  uint32_t startTime = 50; // application start time in milliseconds
  uint32_t endTime = 10000; // application end time in milliseconds
  uint32_t interPacketInterval = 5000; // interpacket interval in microseconds (how much traffic)

  // mobility
  double speed = 20; // speed of the vehicles m/s
  double intraGroupDistance = 10; // distance between two vehicles belonging to the same group
  // double laneDistance = 5.0; // distance between the two lanes
  double antennaHeight = 0.0; // the height of the antenna

  // position
  double enb_x = 100;
  double enb_y = 5;
  double ue_x = 100;
  double ue_y = -5;

  double v1_x = 0;
  double v1_y = 0;
  double v2_x = 0 - intraGroupDistance;
  double v2_y = 0;

  // TODO
  // CommandLine cmd;
  // cmd.AddValue ("vehicleSpeed", "The speed of the vehicle", speed);
  // cmd.Parse (argc, argv);


  Ptr<mmwave::MmWavePointToPointEpcHelper>  epcHelper = CreateObject<mmwave::MmWavePointToPointEpcHelper> ();

  Config::SetDefault ("ns3::MmWaveSidelinkMac::UseAmc", BooleanValue (true));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue (frequency));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::Bandwidth", DoubleValue (bandwidth));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::Numerology", UintegerValue (numerology));
  Config::SetDefault ("ns3::MmWaveVehicularPropagationLossModel::ChannelCondition", StringValue ("l"));
  Config::SetDefault ("ns3::MmWaveVehicularPropagationLossModel::Shadowing", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveVehicularSpectrumPropagationLossModel::UpdatePeriod", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::AntennaElements", UintegerValue (4));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::AntennaElementPattern", StringValue ("3GPP-V2V"));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::IsotropicAntennaElements", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::NumSectors", UintegerValue (2));

  Config::SetDefault ("ns3::MmWaveVehicularNetDevice::RlcType", StringValue("LteRlcUm"));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::SchedulingPatternOption", EnumValue(2)); // use 2 for SchedulingPatternOption=OPTIMIZED, 1 or SchedulingPatternOption=DEFAULT
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (500*1024));

  /////////////////////////////////////// MMWAVE
  unsigned numUe = 1;

  Ptr<mmwave::MmWaveHelper> mmwave_helper = CreateObject<mmwave::MmWaveHelper> ();
  

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (numUe);

  
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (enb_x, enb_y, antennaHeight));

  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (ue_x, ue_y, antennaHeight));

  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);

  NetDeviceContainer enbNetDev = mmwave_helper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = mmwave_helper->InstallUeDevice (ueNodes);


  mmwave_helper->AttachToClosestEnb (ueNetDev, enbNetDev);
  mmwave_helper->EnableTraces ();

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::NGBR_VOICE_VIDEO_GAMING;
  EpsBearer bearer (q); 
  mmwave_helper->ActivateDataRadioBearer (ueNetDev, bearer);




  ////////////////////////////////////// MILLICAR
  // create the nodes
  NodeContainer group1;
  group1.Create (2);
 
  // create the mobility models
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (group1);

  group1.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (v1_x, v1_y, 0));
  group1.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));

  group1.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (v2_x, v2_y, 0));
  group1.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));


  // create and configure the helper
  Ptr<MmWaveVehicularHelper2> helper = CreateObject<MmWaveVehicularHelper2> ();

  if(channel_shared)
  {
    helper->SetSpectrumChannel(mmwave_helper->GetSpectrumChannel(0));
  }




  helper->SetNumerology (numerology);
  helper->SetPropagationLossModelType ("ns3::MmWaveVehicularPropagationLossModel");
  helper->SetSpectrumPropagationLossModelType ("ns3::MmWaveVehicularSpectrumPropagationLossModel");

// choose the pathloss model to use
  helper->SetPathlossModelType ("ns3::ThreeGppUmaPropagationLossModel");

  // choose the channel condition model to use
  helper->SetChannelConditionModelType ("ns3::ThreeGppUmaChannelConditionModel");

  // choose the spectrum propagation loss model
  helper->SetChannelModelType ("ns3::ThreeGppSpectrumPropagationLossModel");
  
  helper->SetSpectrumPropagationLossModelType ("ns3::ThreeGppSpectrumPropagationLossModel");

 
  // helper->SetSpectrumPropagationLossModelType ("ns3::FriisSpectrumPropagationLossModel");
  NetDeviceContainer devs1 = helper->InstallMmWaveVehicularNetDevices (group1);
  
  // install the internet stack
  InternetStackHelper internet2; //already declared
  internet2.Install (group1);
  
  // assign the IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devs1);

  
  // connect the devices belonging to the same group
  helper->PairDevices(devs1);
  
  Ipv4StaticRoutingHelper ipv4RoutingHelper2; //already declared

  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper2.GetStaticRouting (group1.Get (0)->GetObject<Ipv4> ());
  staticRouting->SetDefaultRoute (group1.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () , 2 );

  NS_LOG_DEBUG("IPv4 Address node 0 group 1: " << group1.Get (0)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());
  NS_LOG_DEBUG("IPv4 Address node 1 group 1: " << group1.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());

  
  
  

  // setup the applications
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (0xFFFFFFFF));
  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MicroSeconds (interPacketInterval)));
  Config::SetDefault ("ns3::UdpClient::PacketSize", UintegerValue (packetSize));

  // create the applications for group 1
  uint32_t port = 4000;
  UdpServerHelper server11 (port);
  ApplicationContainer apps = server11.Install (group1.Get (1));

  UdpServerHelper server10 (port);
  apps.Add (server10.Install (group1.Get (0)));

  UdpClientHelper client10 (group1.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), port);
  apps.Add (client10.Install (group1.Get (0)));

  UdpClientHelper client11 (group1.Get (0)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), port);
  apps.Add (client11.Install (group1.Get (1)));





  // set the application start/end time
  apps.Start (MilliSeconds (startTime));
  apps.Stop (MilliSeconds (endTime));

  AsciiTraceHelper asciiTraceHelper;
  std::string filename;
  if(channel_shared){
    filename = "voice_throughput.txt";
  } else {
    filename = "voice_throughput-not-shared.txt";
  }
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (filename);
  apps.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback (&Rx));
  Simulator::Schedule(MilliSeconds(deltaTime), &ThroughputComputation, stream);


  PrintGnuplottableNodeListToFile ("scenario.txt");

  Simulator::Stop (MilliSeconds (endTime + 1000));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

void
PrintGnuplottableNodeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << "set xrange [-200:200]; set yrange [-200:200]" << std::endl;
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<MmWaveVehicularNetDevice> vdev = node->GetDevice (j)->GetObject <MmWaveVehicularNetDevice> ();
          if (vdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << vdev->GetMac ()->GetRnti ()
                      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 7 ps 0.3 lc rgb \"black\" offset 0,0"
                      << std::endl;

              // Simulator::Schedule (Seconds (1), &PrintHelper::UpdateGnuplottableNodeListToFile, filename, node);
            }
        }
    }
}
