/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Sébastien Deronne
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/config-store-module.h"

// This example considers two hidden stations in an 802.11n network which supports MPDU aggregation.
// The user can specify whether RTS/CTS is used and can set the number of aggregated MPDUs.
//
// Example: ./waf --run "simple-ht-hidden-stations --enableRts=1 --nMpdus=8"
//
// Network topology:
//
//   Wifi 192.168.1.0
//
//        AP
//   *    *    *
//   |    |    |
//   n1   n2   n3
//
// Packets in this simulation aren't marked with a QosTag so they are considered
// belonging to BestEffort Access Class (AC_BE).

using namespace ns3;
// trace sink
double packetsRecieved1 = 0, packetsRecieved2 = 0, packetsRecieved3 = 0, packetsRecieved4 = 0, totalPacketsRecieved = 0;
double packetsSent1 = 0, packetsSent2 = 0, packetsSent3 = 0, packetsSent4 = 0, totalPacketsSent = 0;

// trace sink call back functions to count number of sent and recieved packets. Used to measure network performance

void serverRxRecieved(std::string context, Ptr<const Packet> p){
     char recievedFromNode = context.at(context.length() - 24);
     //std::cout << "Packet Recieved from " << recievedFromNode << " at " << Simulator::Now().GetSeconds() << " Seconds" << std::endl;
     if (recievedFromNode == '0'){
        packetsRecieved1++;
     }
     else if (recievedFromNode == '1'){
        packetsRecieved2++;
     }
     else if (recievedFromNode == '2'){
        packetsRecieved3++;
     }
     else if (recievedFromNode == '3'){
        packetsRecieved4++;
     }
     totalPacketsRecieved++;
}

void clientTxSent(std::string context, Ptr<const Packet> p){
   char sentFromNode = context.at(10);
   //std::cout << "Packet Sent from Node " << sentFromNode << std::endl;

   if (sentFromNode == '0'){
      packetsSent1++;
   }
   else if (sentFromNode == '1'){
      packetsSent2++;
   }
   else if (sentFromNode == '2'){
      packetsSent3++;
   }
   else if (sentFromNode == '3'){
      packetsSent4++;
   }
   totalPacketsSent++;

}

void printLocations(NodeContainer container, std::string title){
     std::cout << title << std::endl;
     for(NodeContainer::Iterator it = container.Begin(); it != container.End(); it++){
        Ptr<Node> node = *it;
        Ptr<MobilityModel> position = node->GetObject<MobilityModel> ();
        NS_ASSERT (position != 0);
        Vector pos = position->GetPosition ();
        std::cout << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
     }
}

void printAddresses(Ipv4InterfaceContainer container, std::string title){
     std::cout << title << std::endl;
     uint32_t nNodes = container.GetN ();
     for (uint32_t i = 0; i < nNodes; ++i)
     {
         std::cout << container.GetAddress(i, 0) << std::endl;
     }
     std::cout << std::endl;
}



NS_LOG_COMPONENT_DEFINE ("Final");


int main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; //bytes
  uint64_t simulationTime = 10; //seconds
  uint32_t nMpdus = 1;
  uint32_t maxAmpduSize = 0;
  bool enableRts = 0;
  uint32_t nWifi = 1;
  double mInterval = .1;
  uint32_t nHpms = 0;
  std::stringstream stream; 
  std::string mIntervalString;
   
  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("DcfManager", LOG_LEVEL_INFO);
  CommandLine cmd;
  cmd.AddValue("nWifi", "Number of MS nodes", nWifi);
  cmd.AddValue ("nMpdus", "Number of aggregated MPDUs", nMpdus);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("enableRts", "Enable RTS/CTS", enableRts); // 1: RTS/CTS enabled; 0: RTS/CTS disabled
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue("mInterval","Max Interval",mInterval);
  cmd.AddValue("nHpms", "Number of hpMS Nodes", nHpms);
  cmd.Parse (argc, argv);
  stream << mInterval;
  stream >> mIntervalString;
  
  uint32_t numMS = nWifi + nHpms;
  
  //Sanity Checks
  if(numMS < 0 || numMS > 4 ){
      std::cout << "Too many Nodes. nWifi + nHpms <= 4" << std::endl;
      return -1;
  }




  if (!enableRts)
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
    }
  else
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
    }

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("990000"));

  //Set the maximum size for A-MPDU with regards to the payload size
  maxAmpduSize = nMpdus * (payloadSize + 200);

  // Set the maximum wireless range to 5 meters in order to reproduce a hidden nodes scenario, i.e. the distance between hidden stations is larger than 5 meters
  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (5));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (numMS);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  channel.AddPropagationLoss ("ns3::RangePropagationLossModel"); //wireless range limited to 5 meters!

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue ("HtMcs0"));
  WifiMacHelper mac;
  

  Ssid ssid = Ssid ("simple-mpdu-aggregation");
  NetDeviceContainer staDevices;
  uint32_t i;
  NetDeviceContainer tmp;
  // set Backoff Default for the type of node being created
  for(i = 0; i < wifiStaNodes.GetN(); i++){
     if (i < nWifi){
       //setup Regular MSs
       Config::SetDefault("ns3::Dcf::backoffExp",UintegerValue(1));
       mac.SetType ("ns3::StaWifiMac",
                  "Ssid", SsidValue (ssid),
                  "ActiveProbing", BooleanValue (false),
                  "BE_MaxAmpduSize", UintegerValue (maxAmpduSize),
                  "backoffExp", UintegerValue (0));
       tmp = wifi.Install (phy, mac, wifiStaNodes.Get(i));
       staDevices.Add(tmp);
    }
    else{
       //setup hpMSs
       Config::SetDefault("ns3::Dcf::backoffExp",UintegerValue(0));
       mac.SetType ("ns3::StaWifiMac",
                  "Ssid", SsidValue (ssid),
                  "ActiveProbing", BooleanValue (false),
                  "BE_MaxAmpduSize", UintegerValue (maxAmpduSize),
                  "backoffExp", UintegerValue (1));
       tmp = wifi.Install (phy, mac, wifiStaNodes.Get(i));
       staDevices.Add(tmp);
    }
  }

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconInterval", TimeValue (MicroSeconds (102400)),
               "BeaconGeneration", BooleanValue (true),
               "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (phy, mac, wifiApNode);

  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  // AP is between the two stations, each station being located at 5 meters from the AP.
  // The distance between the two stations is thus equal to 10 meters.
  // Since the wireless range is limited to 5 meters, the two stations are hidden from each other.
  //
  // set locations of MS Nodes
  if (numMS == 1){
     positionAlloc->Add (Vector (5.0, 10.0, 0.0)); //n1
  }
  else if (numMS == 2){
     positionAlloc->Add (Vector (5.0, 10.0, 0.0)); //n1
     positionAlloc->Add (Vector (0.0, 5.0, 0.0)); //n2
  }
  else if (numMS == 3){
     positionAlloc->Add (Vector (5.0, 10.0, 0.0)); //n1
     positionAlloc->Add (Vector (0.0, 5.0, 0.0)); //n2
     positionAlloc->Add (Vector (5.0, 0.0, 0.0));  //n3
  }
  else if (numMS == 4){
     positionAlloc->Add (Vector (5.0, 10.0, 0.0)); //n1
     positionAlloc->Add (Vector (0.0, 5.0, 0.0)); //n2
     positionAlloc->Add (Vector (5.0, 0.0, 0.0));  //n3
     positionAlloc->Add (Vector (10.0, 5.0, 0.0)); // n4
  }
  else{
   std::cout << "Total MS's must be between 1 and 4" << std::endl;
  }
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);
  // set location of AP Node
  Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  apPositionAlloc->Add (Vector (5.0, 5.0, 0.0)); // AP Node Position
  mobility.SetPositionAllocator(apPositionAlloc);
  mobility.Install (wifiApNode);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterface;
  StaInterface = address.Assign (staDevices);
  Ipv4InterfaceContainer ApInterface;
  ApInterface = address.Assign (apDevice);

  // Setting applications
  
  UdpEchoServerHelper echoserver1(9);
  UdpEchoServerHelper echoserver2(10);
  UdpEchoServerHelper echoserver3(11);
  UdpEchoServerHelper echoserver4(12);
 
  UdpEchoClientHelper echoClient1(ApInterface.GetAddress(0), 9);
  UdpEchoClientHelper echoClient2(ApInterface.GetAddress(0), 10);
  UdpEchoClientHelper echoClient3(ApInterface.GetAddress(0), 11);
  UdpEchoClientHelper echoClient4(ApInterface.GetAddress(0), 12);

  ApplicationContainer serverApps1, serverApps2, serverApps3, serverApps4, clientApps1, clientApps2, clientApps3, clientApps4;

  // install applications on the correct nodes
  if (numMS == 1){  //n1
     serverApps1 = echoserver1.Install(wifiApNode);

     serverApps1.Start(Seconds(0.0));
     serverApps1.Stop(Seconds(10.0));

     echoClient1.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient1.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient1.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     clientApps1 = echoClient1.Install (wifiStaNodes.Get (0));


     serverApps1.Start(Seconds(0.0));
     serverApps1.Stop(Seconds(simulationTime + 1));

     clientApps1.Start (Seconds (1.0));
     clientApps1.Stop (Seconds (simulationTime + 1));
  }
  else if (numMS == 2){
     serverApps1 = echoserver1.Install(wifiApNode);
     serverApps2 = echoserver2.Install(wifiApNode);

     echoClient1.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient1.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient1.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     echoClient2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient2.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient2.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     clientApps1 = echoClient1.Install (wifiStaNodes.Get (0));
     clientApps2 = echoClient2.Install (wifiStaNodes.Get (1));

     serverApps1.Start(Seconds(0.0));
     serverApps1.Stop(Seconds(simulationTime + 1));
     serverApps2.Start(Seconds(0.0));
     serverApps2.Stop(Seconds(simulationTime + 1));

     clientApps1.Start (Seconds (1.0));
     clientApps1.Stop (Seconds (simulationTime + 1));
     clientApps2.Start (Seconds (1.0));
     clientApps2.Stop (Seconds (simulationTime + 1));
  }
  else if (numMS == 3){
     serverApps1 = echoserver1.Install(wifiApNode);
     serverApps2 = echoserver2.Install(wifiApNode);
     serverApps3 = echoserver3.Install(wifiApNode);

     echoClient1.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient1.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient1.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     echoClient2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient2.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient2.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     echoClient3.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient3.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient3.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     clientApps1 = echoClient1.Install (wifiStaNodes.Get (0));
     clientApps2 = echoClient2.Install (wifiStaNodes.Get (1));
     clientApps3 = echoClient3.Install (wifiStaNodes.Get (2));


     serverApps1.Start(Seconds(0.0));
     serverApps1.Stop(Seconds(simulationTime + 1));
     serverApps2.Start(Seconds(0.0));
     serverApps2.Stop(Seconds(simulationTime + 1));
     serverApps3.Start(Seconds(0.0));
     serverApps3.Stop(Seconds(simulationTime + 1));

     clientApps1.Start (Seconds (1.0));
     clientApps1.Stop (Seconds (simulationTime + 1));
     clientApps2.Start (Seconds (1.0));
     clientApps2.Stop (Seconds (simulationTime + 1));
     clientApps3.Start (Seconds (1.0));
     clientApps3.Stop (Seconds (simulationTime + 1));
  }
  else if (numMS == 4){
     serverApps1 = echoserver1.Install(wifiApNode);
     serverApps2 = echoserver2.Install(wifiApNode);
     serverApps3 = echoserver3.Install(wifiApNode);
     serverApps4 = echoserver4.Install(wifiApNode);

     echoClient1.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient1.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient1.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     echoClient2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient2.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient2.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     echoClient3.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient3.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient3.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     echoClient4.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     echoClient4.SetAttribute ("Interval", TimeValue (Time (mIntervalString))); //packets/s
     echoClient4.SetAttribute ("PacketSize", UintegerValue (payloadSize));

     clientApps1 = echoClient1.Install (wifiStaNodes.Get (0));
     clientApps2 = echoClient2.Install (wifiStaNodes.Get (1));
     clientApps3 = echoClient3.Install (wifiStaNodes.Get (2));
     clientApps4 = echoClient4.Install (wifiStaNodes.Get (3));

     serverApps1.Start(Seconds(0.0));
     serverApps1.Stop(Seconds(simulationTime + 1));
     serverApps2.Start(Seconds(0.0));
     serverApps2.Stop(Seconds(simulationTime + 1));
     serverApps3.Start(Seconds(0.0));
     serverApps3.Stop(Seconds(simulationTime + 1));
     serverApps4.Start(Seconds(0.0));
     serverApps4.Stop(Seconds(simulationTime + 1));

     clientApps1.Start (Seconds (1.0));
     clientApps1.Stop (Seconds (simulationTime + 1));
     clientApps2.Start (Seconds (1.0));
     clientApps2.Stop (Seconds (simulationTime + 1));
     clientApps3.Start (Seconds (1.0));
     clientApps3.Stop (Seconds (simulationTime + 1));
     clientApps4.Start (Seconds (1.0));
     clientApps4.Stop (Seconds (simulationTime + 1));
  }
  else{
   std::cout << "Total MS's must be between 1 and 4" << std::endl;
  }

  
  // log attributes
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));

  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));

  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));

  ConfigStore outputConfig2;

  outputConfig2.ConfigureDefaults ();

  outputConfig2.ConfigureAttributes ();  
  // connect Trace Sources To trace sink callback functions
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&serverRxRecieved));
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/Tx", MakeCallback(&clientTxSent));



  Simulator::Stop (Seconds (simulationTime + 1));

  Simulator::Run ();
  Simulator::Destroy ();
  
  //uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApps1.Get (0))->GetReceived ();
  //double throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0);
  //std::cout << "Throughput: " << throughput << " Mbit/s" << '\n';
  
  //printAddresses(StaInterface,"IPs of MS's");
  //
  //functions to print locations of nodes on x,y coordinate plane
  printLocations(wifiStaNodes,"Location of all wifi Devices");
  printLocations(wifiApNode,"Location of AP");
  // calculate percentage of lost packets and throughput of network
  double percentLost = (totalPacketsSent-totalPacketsRecieved)/totalPacketsSent;
  double throughput1 = (packetsRecieved1 * payloadSize * 8)/(simulationTime * 1000000);
  double throughput2 = (packetsRecieved2 * payloadSize * 8)/(simulationTime * 1000000);
  double throughput3 = (packetsRecieved3 * payloadSize * 8)/(simulationTime * 1000000);
  double throughput4 = (packetsRecieved4 * payloadSize * 8)/(simulationTime * 1000000);
  double throughputTotal = (totalPacketsRecieved * payloadSize * 8)/(simulationTime * 1000000);

  
  std::cout << "Packets Lost % = " << percentLost * 100 << std::endl;
  std::cout << "Packets Recieved 1: " << packetsRecieved1 << " 2: " << packetsRecieved2 << " 3: " << packetsRecieved3 << " 4: " << packetsRecieved4 << " Recieved Total: " << totalPacketsRecieved << std::endl;
  std::cout << "Packets Lost 1: " << packetsSent1 - packetsRecieved1 << " 2: " << packetsSent2 - packetsRecieved2 << " 3: " << packetsSent3 - packetsRecieved3 << " 4: " << packetsSent4 - packetsRecieved4 << " Lost Total: " << totalPacketsSent - totalPacketsRecieved <<std::endl;
  std::cout << "Packets Sent 1: " << packetsSent1 << " 2: " << packetsSent2 << " 3: " << packetsSent3 << " 4: " << packetsSent4 << " Sent Total: " << totalPacketsSent <<std::endl;
  std::cout << "Throughput (Mbps) 1: " << throughput1 << " 2: " << throughput2 << " 3: " << throughput3 << " 4: " << throughput4 << " Throughput Total: " << throughputTotal <<std::endl;

  return 0;
}
