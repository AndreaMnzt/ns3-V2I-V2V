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

#include "mmwave-vehicular-helper2.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/mmwave-vehicular-net-device.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mmwave-vehicular-antenna-array-model.h"
#include "ns3/pointer.h"
#include "ns3/config.h"

#include "ns3/multi-model-spectrum-channel.h"
#include <ns3/channel-condition-model.h>
#include <ns3/mmwave-beamforming-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/three-gpp-propagation-loss-model.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveVehicularHelper2"); // TODO check if this has to be defined here

namespace millicar {

NS_OBJECT_ENSURE_REGISTERED (MmWaveVehicularHelper2); // TODO check if this has to be defined here

MmWaveVehicularHelper2::MmWaveVehicularHelper2 ()
: m_phyTraceHelper{CreateObject<MmWaveVehicularTracesHelper>("sinr-mcs.txt")} // TODO name as attribute
{
  NS_LOG_FUNCTION (this);
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
  m_bfModelFactory.SetTypeId (mmwave::MmWaveSvdBeamforming::GetTypeId ());
  
}

MmWaveVehicularHelper2::~MmWaveVehicularHelper2 ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MmWaveVehicularHelper2::GetTypeId ()
{
  static TypeId tid =
  TypeId ("ns3::MmWaveVehicularHelper2")
  .SetParent<Object> ()
  .AddConstructor<MmWaveVehicularHelper2> ()
  .AddAttribute ("PropagationLossModel",
                 "The type of path-loss model to be used. "
                 "The allowed values for this attributes are the type names "
                 "of any class inheriting from ns3::PropagationLossModel.",
                 StringValue (""),
                 MakeStringAccessor (&MmWaveVehicularHelper2::SetPropagationLossModelType),
                 MakeStringChecker ())
  .AddAttribute ("SpectrumPropagationLossModel",
                 "The type of fast fading model to be used. "
                 "The allowed values for this attributes are the type names "
                 "of any class inheriting from ns3::SpectrumPropagationLossModel.",
                 StringValue (""),
                 MakeStringAccessor (&MmWaveVehicularHelper2::SetSpectrumPropagationLossModelType),
                 MakeStringChecker ())
  .AddAttribute ("PropagationDelayModel",
                 "The type of propagation delay model to be used. "
                 "The allowed values for this attributes are the type names "
                 "of any class inheriting from ns3::PropagationDelayModel.",
                 StringValue (""),
                 MakeStringAccessor (&MmWaveVehicularHelper2::SetPropagationDelayModelType),
                 MakeStringChecker ())
  .AddAttribute ("Numerology",
                 "Numerology to use for the definition of the frame structure."
                 "2 : subcarrier spacing will be set to 60 KHz"
                 "3 : subcarrier spacing will be set to 120 KHz",
                 UintegerValue (2),
                 MakeUintegerAccessor (&MmWaveVehicularHelper2::SetNumerology),
                 MakeUintegerChecker<uint8_t> ())
  .AddAttribute ("Bandwidth",
                 "Bandwidth in Hz",
                 DoubleValue (1e8),
                 MakeDoubleAccessor (&MmWaveVehicularHelper2::m_bandwidth),
                 MakeDoubleChecker<double> ())
  .AddAttribute ("SchedulingPatternOption",
                 "The type of scheduling pattern option to be used for resources assignation."
                 "Default   : one single slot per subframe for each device"
                 "Optimized : each slot of the subframe is used",
                 EnumValue(DEFAULT),
                 MakeEnumAccessor (&MmWaveVehicularHelper2::SetSchedulingPatternOptionType,
                                   &MmWaveVehicularHelper2::GetSchedulingPatternOptionType),
                 MakeEnumChecker(DEFAULT, "Default",
                                 OPTIMIZED, "Optimized"))
  .AddAttribute ("PathlossModel",
                   "The type of path-loss model to be used. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::PropagationLossModel.",
                   StringValue ("ns3::ThreeGppUmaPropagationLossModel"),
                   MakeStringAccessor (&MmWaveVehicularHelper2::SetPathlossModelType),
                   MakeStringChecker ())
  .AddAttribute ("ChannelModel",
                   "The type of MIMO channel model to be used. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::SpectrumPropagationLossModel.",
                   StringValue ("ns3::ThreeGppSpectrumPropagationLossModel"),
                   MakeStringAccessor (&MmWaveVehicularHelper2::SetChannelModelType),
                   MakeStringChecker ())
  .AddAttribute ("BeamformingModel",
                   "The type of beamforming model to be used.",
                   StringValue ("ns3::MmWaveSvdBeamforming"),
                   MakeStringAccessor (&MmWaveVehicularHelper2::SetBeamformingModelType),
                   MakeStringChecker ())
  ;

  return tid;
}

void
MmWaveVehicularHelper2::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  // intialize the RNTI counter
  m_rntiCounter = 0;
  
  // if the PHY layer configuration object was not set manually, create it 
  if (!m_phyMacConfig)
  {
    m_phyMacConfig = CreateObjectWithAttributes<mmwave::MmWavePhyMacCommon> ("Numerology", EnumValue (m_numerologyIndex),   
                                                                             "Bandwidth", DoubleValue (m_bandwidth));
  }

  if (!m_channel){
    MmWaveChannelModelInitialization ();      // channel initialization
  }
}

void
MmWaveVehicularHelper2::SetConfigurationParameters (Ptr<mmwave::MmWavePhyMacCommon> conf)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_rntiCounter == 0, "The PHY configuration should be set before the installation of any device.");
  m_phyMacConfig = conf;
}

Ptr<mmwave::MmWavePhyMacCommon>
MmWaveVehicularHelper2::GetConfigurationParameters () const
{
  NS_LOG_FUNCTION (this);
  return m_phyMacConfig;
}

void
MmWaveVehicularHelper2::SetNumerology (uint8_t index)
{
  NS_LOG_FUNCTION (this);
  m_numerologyIndex = index;
}

NetDeviceContainer
MmWaveVehicularHelper2::InstallMmWaveVehicularNetDevices (NodeContainer nodes)
{
  NS_LOG_FUNCTION (this);

  Initialize (); // run DoInitialize if necessary

  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
    {
      Ptr<Node> node = *i;

      // create the device
      Ptr<MmWaveVehicularNetDevice> device = InstallSingleMmWaveVehicularNetDevice (node, ++m_rntiCounter);

      // assign an address
      device->SetAddress (Mac64Address::Allocate ());

      devices.Add (device);
    }

  return devices;
}

Ptr<MmWaveVehicularNetDevice>
MmWaveVehicularHelper2::InstallSingleMmWaveVehicularNetDevice (Ptr<Node> node, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  // create the antenna
  u_int16_t AntennaNum = 16;
  Ptr<ThreeGppAntennaArrayModel> antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumRows", UintegerValue (sqrt (AntennaNum)), "NumColumns", UintegerValue (sqrt (AntennaNum)));



  // create and configure the tx spectrum phy
  Ptr<MmWaveSidelinkSpectrumPhy> ssp = CreateObject<MmWaveSidelinkSpectrumPhy> ();
  NS_ASSERT_MSG (node->GetObject<MobilityModel> (), "Missing mobility model");
  ssp->SetMobility (node->GetObject<MobilityModel> ());
  ssp->UseThreeGppAntenna(true);
  ssp->SetThreeGppAntenna(antenna);
  NS_ASSERT_MSG (m_channel, "First create the channel");
  ssp->SetChannel (m_channel);

  
  // create and configure the chunk processor
  Ptr<mmwave::mmWaveChunkProcessor> pData = Create<mmwave::mmWaveChunkProcessor> ();
  pData->AddCallback (MakeCallback (&MmWaveSidelinkSpectrumPhy::UpdateSinrPerceived, ssp));
  ssp->AddDataSinrChunkProcessor (pData);

  // create the phy
  NS_ASSERT_MSG (m_phyMacConfig, "First set the configuration parameters");
  Ptr<MmWaveSidelinkPhy> phy = CreateObject<MmWaveSidelinkPhy> (ssp, m_phyMacConfig);

  // connect the rx callback of the spectrum object to the sink
  ssp->SetPhyRxDataEndOkCallback (MakeCallback (&MmWaveSidelinkPhy::Receive, phy));

  // connect the callback to report the SINR
  ssp->SetSidelinkSinrReportCallback (MakeCallback (&MmWaveSidelinkPhy::GenerateSinrReport, phy));

  // add the spectrum phy to the spectrum channel
  m_channel->AddRx (ssp);

  if(m_phyTraceHelper != 0)
  {
    ssp->SetSidelinkSinrReportCallback (MakeCallback (&MmWaveVehicularTracesHelper::McsSinrCallback, m_phyTraceHelper));
  }

  // create the mac
  Ptr<MmWaveSidelinkMac> mac = CreateObject<MmWaveSidelinkMac> (m_phyMacConfig);
  mac->SetRnti (rnti);

  // connect phy and mac
  phy->SetPhySapUser (mac->GetPhySapUser ());
  mac->SetPhySapProvider (phy->GetPhySapProvider ());

  // create and configure the device
  Ptr<MmWaveVehicularNetDevice> device = CreateObject<MmWaveVehicularNetDevice> (phy, mac);
  node->AddDevice (device);
  device->SetNode (node);
  ssp->SetDevice (device);

  // connect the rx callback of the mac object to the rx method of the NetDevice
  mac->SetForwardUpCallback(MakeCallback(&MmWaveVehicularNetDevice::Receive, device));

  // initialize the 3GPP channel model
  Ptr<SpectrumPropagationLossModel> splm = m_channel->GetSpectrumPropagationLossModel ();
  Ptr<ThreeGppSpectrumPropagationLossModel> threeGppSplm = DynamicCast<ThreeGppSpectrumPropagationLossModel> (splm);
  if (threeGppSplm)
  {
    threeGppSplm->AddDevice (device, antenna);
  }

  auto channelModel = threeGppSplm->GetChannelModel();
  Ptr<mmwave::MmWaveBeamformingModel> bfModel = m_bfModelFactory.Create<mmwave::MmWaveBeamformingModel> ();
  bfModel->SetAttributeFailSafe ("Device", PointerValue (device));
  bfModel->SetAttributeFailSafe ("Antenna", PointerValue (antenna));
  bfModel->SetAttributeFailSafe ("ChannelModel", PointerValue (channelModel));
  ssp->SetBeamformingModel(bfModel);      

  return device;
}

void
MmWaveVehicularHelper2::PairDevices (NetDeviceContainer devices)
{
  NS_LOG_FUNCTION (this);

  // TODO update this part to enable a more flexible configuration of the
  // scheduling pattern

  std::vector<uint16_t> pattern = CreateSchedulingPattern(devices);

  uint8_t bearerId = 1;

  for (NetDeviceContainer::Iterator i = devices.Begin (); i != devices.End (); ++i)
    {

      Ptr<MmWaveVehicularNetDevice> di = DynamicCast<MmWaveVehicularNetDevice> (*i);
      Ptr<Node> iNode = di->GetNode ();
      Ptr<Ipv4> iNodeIpv4 = iNode->GetObject<Ipv4> ();
      NS_ASSERT_MSG (iNodeIpv4 != 0, "Nodes need to have IPv4 installed before pairing can be activated");

      di->GetMac ()->SetSfAllocationInfo (pattern); // this is called ONCE for each NetDevice

      for (NetDeviceContainer::Iterator j = i + 1; j != devices.End (); ++j)
      {
        Ptr<MmWaveVehicularNetDevice> dj = DynamicCast<MmWaveVehicularNetDevice> (*j);
        Ptr<Node> jNode = dj->GetNode ();
        Ptr<Ipv4> jNodeIpv4 = jNode->GetObject<Ipv4> ();
        NS_ASSERT_MSG (jNodeIpv4 != 0, "Nodes need to have IPv4 installed before pairing can be activated");

        // initialize the <IP address, RNTI> map of the devices
        int32_t interface =  jNodeIpv4->GetInterfaceForDevice (dj);
        Ipv4Address diAddr = iNodeIpv4->GetAddress (interface, 0).GetLocal ();
        Ipv4Address djAddr = jNodeIpv4->GetAddress (interface, 0).GetLocal ();

        // register the associated devices in the PHY
        di->GetPhy ()->AddDevice (dj->GetMac ()->GetRnti (), dj);
        dj->GetPhy ()->AddDevice (di->GetMac ()->GetRnti (), di);

        // bearer activation by creating a logical channel between the two devices
        NS_LOG_DEBUG("Activation of bearer between " << diAddr << " and " << djAddr);
        NS_LOG_DEBUG("Bearer ID: " << uint32_t(bearerId) << " - Associate RNTI " << di->GetMac ()->GetRnti () << " to " << dj->GetMac ()->GetRnti ());

        di->ActivateBearer(bearerId, dj->GetMac ()->GetRnti (), djAddr);
        dj->ActivateBearer(bearerId, di->GetMac ()->GetRnti (), diAddr);
        bearerId++;
      }


    }
}

std::vector<uint16_t>
MmWaveVehicularHelper2::CreateSchedulingPattern (NetDeviceContainer devices)
{
  // The maximum supported number of vehicles in the group is equal to the available number of slots per subframe
  // TODO implement scheduling pattern that support groups with a number of vehicle greater than the number of slots per subframe
  NS_ABORT_MSG_IF (devices.GetN () > m_phyMacConfig->GetSlotsPerSubframe (), "Too many devices");

  // NOTE fixed scheduling pattern, set in configuration time and assumed the same for each subframe

  uint8_t slotPerSf = m_phyMacConfig->GetSlotsPerSubframe ();
  std::vector<uint16_t> pattern;

  switch (m_schedulingOpt)
  {
    case DEFAULT:
    {
      // Each slot in the subframe is assigned to a different user.
      // If (numDevices < numSlots), the remaining available slots are unused
      pattern = std::vector<uint16_t> (slotPerSf);
      for (uint16_t i = 0; i < devices.GetN (); i++)
      {
        Ptr<MmWaveVehicularNetDevice> di = DynamicCast<MmWaveVehicularNetDevice> (devices.Get (i));
        pattern.at(i) = di->GetMac ()->GetRnti ();
        NS_LOG_DEBUG ("slot " << i << " assigned to rnti " << di->GetMac ()->GetRnti ());
      }
      break;
    }
    case OPTIMIZED:
    {
      // Each slot in the subframe is used
      uint8_t slotPerDev = std::floor ( slotPerSf / devices.GetN ());
      uint8_t remainingSlots = slotPerSf % devices.GetN ();

      NS_LOG_DEBUG("Minimum number of slots per device = " << (uint16_t)slotPerDev);
      NS_LOG_DEBUG("Available slots = " << (uint16_t)slotPerSf);

      uint8_t slotCnt = 0;

      for (uint16_t i = 0; i < devices.GetN (); i++)
      {
        Ptr<MmWaveVehicularNetDevice> di = DynamicCast<MmWaveVehicularNetDevice> (devices.Get (i));

        for (uint8_t j = 0; j < slotPerDev; j++)
        {
          pattern.push_back(di->GetMac ()->GetRnti ());
          NS_LOG_DEBUG ("slot " << uint16_t(slotCnt) << " assigned to rnti " << di->GetMac ()->GetRnti ());
          slotCnt++;
        }
      }

      NS_LOG_DEBUG("Remaining slots = " << (uint16_t)remainingSlots);
      for (uint16_t i = 0; i < remainingSlots; i++)
      {
        Ptr<MmWaveVehicularNetDevice> di = DynamicCast<MmWaveVehicularNetDevice> (devices.Get (i));
        pattern.push_back(di->GetMac ()->GetRnti ());
        NS_LOG_DEBUG ("slot " << uint16_t(slotCnt) << " assigned to rnti " << di->GetMac ()->GetRnti ());
        slotCnt++;
      }
      break;
    }
    default:
    {
      NS_FATAL_ERROR("Programming Error.");
    }
  }

  return pattern;
}

void
MmWaveVehicularHelper2::SetPropagationLossModelType (std::string plm)
{
  NS_LOG_FUNCTION (this);
  m_propagationLossModelType = plm;
}

void
MmWaveVehicularHelper2::SetSpectrumPropagationLossModelType (std::string splm)
{
  NS_LOG_FUNCTION (this);
  m_spectrumPropagationLossModelType = splm;
}

void
MmWaveVehicularHelper2::SetPropagationDelayModelType (std::string pdm)
{
  NS_LOG_FUNCTION (this);
  m_propagationDelayModelType = pdm;
}

void
MmWaveVehicularHelper2::SetSchedulingPatternOptionType (SchedulingPatternOption_t spo)
{
  NS_LOG_FUNCTION (this);
  m_schedulingOpt = spo;
}

MmWaveVehicularHelper2::SchedulingPatternOption_t
MmWaveVehicularHelper2::GetSchedulingPatternOptionType () const
{
  NS_LOG_FUNCTION (this);
  return m_schedulingOpt;
}

/// New Methods ///

// spectrum propagation Loss model methods

void
MmWaveVehicularHelper2::SetChannelModelType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_spectrumPropagationLossModelType = type;
  if (!type.empty ())
    {
      m_spectrumPropagationLossModelFactory = ObjectFactory ();
      m_spectrumPropagationLossModelFactory.SetTypeId (type);
    }
}

void
MmWaveVehicularHelper2::SetChannelModelAttribute (std::string name, const AttributeValue &value)
{
  NS_LOG_FUNCTION (this);
  m_spectrumPropagationLossModelFactory.Set (name, value);
}

// channel condition model methods

void
MmWaveVehicularHelper2::SetChannelConditionModelType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_channelConditionModelType = type;
  if (!type.empty ())
    {
      m_channelConditionModelFactory = ObjectFactory ();
      m_channelConditionModelFactory.SetTypeId (type);
    }
}

// pathloss methods

void
MmWaveVehicularHelper2::SetPathlossModelType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_pathlossModelType = type;
  if (!type.empty ())
    {
      m_pathlossModelFactory = ObjectFactory ();
      m_pathlossModelFactory.SetTypeId (type);
    }
}

Ptr<PropagationLossModel>
MmWaveVehicularHelper2::GetPathLossModel ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (!m_pathlossModel, "Unable to find the requested pathloss model");
  return m_pathlossModel->GetObject<PropagationLossModel> ();
}


// beamforming methods

void
MmWaveVehicularHelper2::SetBeamformingModelType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_bfModelFactory = ObjectFactory (type);
}
void
MmWaveVehicularHelper2::SetBeamformingModelAttribute (std::string name, const AttributeValue &value)
{
  NS_LOG_FUNCTION (this);
  m_bfModelFactory.Set (name, value);
}

// Spectrum Channel methods

void 
MmWaveVehicularHelper2::SetSpectrumChannel (Ptr<ns3::SpectrumChannel> channel)
{
  NS_LOG_FUNCTION (this);
  m_channel = channel;
}

Ptr<SpectrumChannel>
MmWaveVehicularHelper2::GetSpectrumChannel ()
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

// setup channel 

void
MmWaveVehicularHelper2::MmWaveChannelModelInitialization (void)
{
  NS_LOG_FUNCTION (this);
  // setup of mmWave channel & related
  
  Ptr<SpectrumChannel> channel = m_channelFactory.Create<SpectrumChannel> ();
  
  // create the channel condition model (if needed)
  Ptr<ChannelConditionModel> ccm;
  if (!m_channelConditionModelType.empty ())
  {
    ccm = m_channelConditionModelFactory.Create<ChannelConditionModel> ();
  }

  // create the propagation loss model
  if (!m_pathlossModelType.empty ())
    {
      Ptr<PropagationLossModel> plm = m_pathlossModelFactory.Create<PropagationLossModel> ();
      if (plm)
        {
          plm->SetAttributeFailSafe ("Frequency", DoubleValue (m_phyMacConfig->GetCenterFrequency ()));

          // associate the channel condition model to the propagation loss model (if needed)
          if (ccm)
          {
            plm->SetAttributeFailSafe ("ChannelConditionModel", PointerValue (ccm));
          }

          // set the propagation loss model in the channel
          channel->AddPropagationLossModel (plm);
        }

      // store the propagation loss model
      m_pathlossModel = plm;
    }
    else
    {
      NS_LOG_WARN (this << " No PropagationLossModel!");
    }

    // create and configure the SpectrumPropagationLossModel
    if (!m_spectrumPropagationLossModelType.empty ())
      {
        Ptr<SpectrumPropagationLossModel> splm = m_spectrumPropagationLossModelFactory.Create<SpectrumPropagationLossModel> ();
        
        
        // if the selected model is ThreeGppSpectrumPropagationLossModel we 
        // need a special configuration procedure, otherwise, for the other 
        // models, we try to configure the frequency
        Ptr<ThreeGppSpectrumPropagationLossModel> threeGppSplm = DynamicCast<ThreeGppSpectrumPropagationLossModel> (splm);
        if (threeGppSplm)
        {
          threeGppSplm->SetChannelModelAttribute ("Frequency", DoubleValue (m_phyMacConfig->GetCenterFrequency ()));
              
          // the ThreeGppSpectrumPropagationLossModel must have the same ChannelConditionModel as the 
          // propagation loss model instace
          if (ccm) // the channel condition model was created using the factory
          {
            threeGppSplm->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (ccm));
          }
          else if (!m_pathlossModel) // the channel condition model was created inside the propagation loss model 
          {
            PointerValue ptr; 
            m_pathlossModel->GetAttribute ("ChannelConditionModel", ptr);
            ccm = ptr.Get<ChannelConditionModel> ();
            threeGppSplm->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (ccm));
          }
          else
          {
            NS_LOG_DEBUG ("ChannelConditionModel not set for ThreeGppSpectrumPropagationLossModel");
          }
        }
        else 
        {
          splm->SetAttributeFailSafe ("Frequency", DoubleValue (m_phyMacConfig->GetCenterFrequency ()));
        }

        // set the propagation loss model in the channel
        auto matrix = threeGppSplm->GetChannelModel();
        channel->AddSpectrumPropagationLossModel (threeGppSplm);
      }
      else
      {
        NS_LOG_WARN (this << " No SpectrumPropagationLossModel!");
      }

      m_channel = channel;
    
}




} // namespace millicar
} // namespace ns3
