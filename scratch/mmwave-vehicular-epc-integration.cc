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

#include <ns3/mmwave-phy-mac-common.h>
#include <ns3/mmwave-component-carrier.h>

NS_LOG_COMPONENT_DEFINE ("v2epc");

using namespace ns3;
using namespace millicar;

// this script creates two millicar vehicles moving horizontally
// while a UE is trasmitting to a eNB connected to an EPC nearby

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


uint32_t g_rxPackets2 = 0; // total number of received packets
uint32_t g_txPackets2; // total number of transmitted packets
uint32_t g_clientrxPackets2; // total number of received packets
uint32_t g_old_rxPackets2;

Time g_firstReceived2; // timestamp of the first time a packet is received
Time g_lastReceived2; // timestamp of the last received packet

static void Rx2 (Ptr<const Packet> p)
{
 g_rxPackets2++;
 if (g_rxPackets2 > 1)
 {    
    g_lastReceived2 = Simulator::Now();
 }
 else
 {
   g_firstReceived2 = Simulator::Now();
 }

}

void ThroughputComputation2 (Ptr<OutputStreamWrapper> stream)
{
    *stream->GetStream ()<<Simulator::Now ().GetSeconds ()<<" , "<< ((g_rxPackets2 - g_old_rxPackets2)*(double(packetSize)*8)/(double(deltaTime)/1000) )/1e6 << " Mbps " << g_rxPackets2 << std::endl;
    g_old_rxPackets2 = g_rxPackets2;
    Simulator::Schedule( MilliSeconds(deltaTime), &ThroughputComputation2, stream);
}

int main (int argc, char *argv[])
{

  int seed = 31;
  bool channel_shared = true;
  RngSeedManager::SetRun (2);

  
  CommandLine cmd;
  cmd.AddValue("rngrun", "Simulation run", seed);
  cmd.Parse (argc, argv);
  RngSeedManager::SetRun (seed);


  // system parameters
  double bandwidth = 100.0e6; // bandwidth in Hz
  double frequency = 28e9; // the carrier frequency
  uint32_t numerology = 3; // the numerologym

  // applications
  uint32_t packetSize = 1024; // UDP packet size in bytes
  uint32_t startTime = 10; // application start time in milliseconds
  uint32_t endTime = 8000; // application end time in milliseconds
  uint32_t interPacketInterval = 10000; // interpacket interval in microseconds 1000

  // mobility  
  double speed = 20; // speed of the vehicles m/s
  double intraGroupDistance = 20; // distance between two vehicles belonging to the same group
  // double laneDistance = 5.0; // distance between the two lanes
  double antennaHeight = 0; // the height of the antenna

  // position mmwave and millicar 
  double enb_x = 80;
  double enb_y = -5;
  double ue_x = 80;
  double ue_y = 5;

  double v1_x = 0;
  double v1_y = 0;
  double v2_x = 0 - intraGroupDistance;
  double v2_y = 0;

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
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::NumSectors", UintegerValue (16));
  Config::SetDefault ("ns3::MmWaveVehicularNetDevice::RlcType", StringValue("LteRlcUm"));
  Config::SetDefault ("ns3::MmWaveVehicularHelper2::SchedulingPatternOption", EnumValue(2)); // use 2 for SchedulingPatternOption=OPTIMIZED, 1 or SchedulingPatternOption=DEFAULT
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (500*1024));

  // MMWAVE Setup

  bool harqEnabled = true;
  bool rlcAmEnabled = false;

  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (rlcAmEnabled));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));



  unsigned numUe = 1;

  Ptr<mmwave::MmWaveHelper> mmwave_helper = CreateObject<mmwave::MmWaveHelper> ();
  mmwave_helper->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
  Ptr<mmwave::MmWavePointToPointEpcHelper>  epcHelper = CreateObject<mmwave::MmWavePointToPointEpcHelper> (); //created os
  mmwave_helper->SetEpcHelper (epcHelper);
  
  mmwave_helper->SetHarqEnabled (harqEnabled);
  

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  mmwave_helper->SetPathlossModelType("ns3::ThreeGppUmaPropagationLossModel");
  mmwave_helper->SetChannelConditionModelType("ns3::ThreeGppUmaChannelConditionModel");


  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node>  remoteHost = remoteHostContainer.Get (0);
 
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s"))); // 100 
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay" , TimeValue (Seconds (0.010)) );//, TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
 
  ueNodes.Create (numUe);
  
  // eNB position
  uint16_t enb_height = 0;
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (enb_x, enb_y, enb_height));
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  // UE position
  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (ue_x, ue_y, antennaHeight));
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (ueNodes);
  
  // install devices
  NetDeviceContainer enbNetDev = mmwave_helper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = mmwave_helper->InstallUeDevice (ueNodes);



  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
  
  Ptr<Node> ueNode = ueNodes.Get (0);
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  

  mmwave_helper->AttachToClosestEnb (ueNetDev, enbNetDev);
  

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1235;
  uint16_t ulPort = 2001;
  uint16_t otherPort = 3001;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  
  PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
  PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
  serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (0)));
  serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
  serverApps.Add (packetSinkHelper.Install (ueNodes.Get (0)));

  uint32_t mmwave_interpacket_time = 100;
  UdpClientHelper dlClient (ueIpIface.GetAddress (0), dlPort);
  dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (mmwave_interpacket_time)));
  dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));

  UdpClientHelper ulClient (remoteHostAddr, ulPort);
  ulClient.SetAttribute ("Interval", TimeValue (MicroSeconds (mmwave_interpacket_time)));
  ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));


  clientApps.Add (dlClient.Install (remoteHost));
  clientApps.Add (ulClient.Install (ueNodes.Get(0)));
  
  serverApps.Start (Seconds (1));
  clientApps.Start (Seconds (1));

  //mmwave_helper->EnableTraces ();
  p2ph.EnablePcapAll ("mmwave-epc");

  // millicar setup
  // create the nodes
  NodeContainer group1;
  group1.Create (2);
  

  // create the mobility models
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (group1);

  group1.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (v1_x, v1_y, antennaHeight));
  group1.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));

  group1.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (v2_x, v2_y, antennaHeight));
  group1.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));


  // create and configure the helper
  Ptr<MmWaveVehicularHelper2> helper = CreateObject<MmWaveVehicularHelper2> ();

  // share the channel
  if(channel_shared)
  {
    helper->SetSpectrumChannel(mmwave_helper->GetSpectrumChannel(0));
    
    helper->SetNumerology (numerology);
    helper->SetPropagationLossModelType("ns3::MmWaveVehicularPropagationLossModel");
    helper->SetSpectrumPropagationLossModelType("ns3::MmWaveVehicularSpectrumPropagationLossModel");

    // choose the pathloss model to use
    helper->SetPathlossModelType("ns3::ThreeGppUmaPropagationLossModel");

    // choose the channel condition model to use
    helper->SetChannelConditionModelType("ns3::ThreeGppUmaChannelConditionModel");

    // choose the spectrum propagation loss model
    helper->SetChannelModelType("ns3::ThreeGppSpectrumPropagationLossModel");

    helper->SetSpectrumPropagationLossModelType("ns3::ThreeGppSpectrumPropagationLossModel");

  } else {

    helper->SetNumerology (numerology);
    helper->SetPropagationLossModelType("ns3::MmWaveVehicularPropagationLossModel");
    helper->SetSpectrumPropagationLossModelType("ns3::MmWaveVehicularSpectrumPropagationLossModel");

    // choose the pathloss model to use
    helper->SetPathlossModelType("ns3::ThreeGppUmaPropagationLossModel");

    // choose the channel condition model to use
    helper->SetChannelConditionModelType("ns3::ThreeGppUmaChannelConditionModel");

    // choose the spectrum propagation loss model
    helper->SetChannelModelType("ns3::ThreeGppSpectrumPropagationLossModel");

    helper->SetSpectrumPropagationLossModelType("ns3::ThreeGppSpectrumPropagationLossModel");
  }

 
  NetDeviceContainer devs1 = helper->InstallMmWaveVehicularNetDevices (group1);

  // Get rnti to print snri
  //std::cout << "rnt car 1 "<< DynamicCast<MmWaveVehicularNetDevice>(devs1.Get(0))->GetMac()->GetRnti() << std::endl;
  //std::cout << "rnt car 2 "<< DynamicCast<MmWaveVehicularNetDevice>(devs1.Get(1))->GetMac()->GetRnti() << std::endl;

  // install the internet stack
  InternetStackHelper internet2; 
  internet2.Install (group1);
  // assign the IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devs1);

  
  // connect the devices belonging to the same group
  helper->PairDevices(devs1);
  
  Ipv4StaticRoutingHelper ipv4RoutingHelper2; 
  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper2.GetStaticRouting (group1.Get (0)->GetObject<Ipv4> ());
  staticRouting->SetDefaultRoute (group1.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () , 2 );


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
    filename = "epc_throughput.txt";
  } else {
    filename = "epc_throughput-not-shared.txt";
  }
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (filename);
  apps.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback (&Rx));
  Simulator::Schedule(MilliSeconds(deltaTime), &ThroughputComputation, stream);


  AsciiTraceHelper asciiTraceHelper2;
  std::string filename2;
  if(channel_shared){
    filename2 = "epc_throughput2.txt";
  } else {
    filename2 = "epc_throughput-not-shared.txt2";
  }
  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper2.CreateFileStream (filename2);
  apps.Get(1)->TraceConnectWithoutContext ("Rx", MakeCallback (&Rx2));
  Simulator::Schedule(MilliSeconds(deltaTime), &ThroughputComputation2, stream2);

 
  Simulator::Stop (MilliSeconds (endTime + 1000));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

