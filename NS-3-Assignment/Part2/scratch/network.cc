
#define FINISH 20.0
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <iostream>
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/enum.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "simple-client.h"
#include "ns3/mobility-module.h"

using namespace ns3;

/* Node Diagram:
   
N0 ... 100Mbps/20ms ... BS1 --- 10Mbps/100ms --- BS2 ... 100Mbps/20ms ... N1
   
*/

void testUsingPacketSize(size_t packetSize);
std::string tcpType;

double FairnessIndex(std::vector < double >);

int main(int argc, char** argv) {

    std::cout << std::fixed << std::setprecision(2);

    
    CommandLine cmd;

    size_t packetSize = 40;
    

    // Parse command line for packet size and tcp Type"
    
    cmd.AddValue("ps",
                 "Packet Size:",
                 packetSize);
    
    tcpType = "veno";

    cmd.AddValue("tcpType",
                 "Type of TCP algorithm used (westwood/veno/vegas)",
                 tcpType);
    cmd.Parse(argc, argv);

    
    if (tcpType == "westwood") {
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                            TypeIdValue(TcpWestwood::GetTypeId()));
    } else if (tcpType == "veno") {
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                            TypeIdValue(TcpVeno::GetTypeId()));
    } else if (tcpType == "vegas") {
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                            TypeIdValue(TcpVegas::GetTypeId()));
    } else {
        std::cout << "Incorrect Parameters" << std::endl;
        return 1;
    }

    // Configure Default Wifi Parameters
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("999999"));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("999999"));
    Config::SetDefault("ns3::WifiMacQueue::DropPolicy", EnumValue(WifiMacQueue::DROP_NEWEST));

    
    testUsingPacketSize(packetSize);
}

// Testing function using different packetSizes

void testUsingPacketSize(size_t packetSize) {

    NodeContainer nodes;
    nodes.Create(4);
    
    // Setup Wired link

    // Setup p2p with data given in problem
    PointToPointHelper p2pForBS;
    p2pForBS.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2pForBS.SetChannelAttribute("Delay", StringValue("100ms"));

    // Compute Bandwidth-Delay product
    double bitsPerSecond = 10 * 1e6 * 100 * 1e-3;
    double packetsPerSecond = bitsPerSecond / (8 * packetSize);
    size_t queueSize = int(std::roundl(packetsPerSecond));

    // Setup Drop Tail queue with Buffer size dependent on Bandwidth-Delay product
    p2pForBS.SetQueue("ns3::DropTailQueue",
                      "MaxSize", StringValue(std::to_string(queueSize) + "p"));
    
    NetDeviceContainer bsDevices =
        p2pForBS.Install(nodes.Get(1), nodes.Get(2));

    // Setup Wireless link
    
    YansWifiChannelHelper channelHelper0 = YansWifiChannelHelper::Default();
    Ptr<YansWifiChannel> channel0 = channelHelper0.Create();
    
    YansWifiChannelHelper channelHelper1 = YansWifiChannelHelper::Default();
    Ptr<YansWifiChannel> channel1 = channelHelper1.Create();
    
    // This will the distance between N0 and BS0 and between N1 and BS1
    double distance = 50;


    // Initialize the physical helpers for the wifi channel
    YansWifiPhyHelper phyHelper0 = YansWifiPhyHelper::Default();
    
    YansWifiPhyHelper phyHelper1 = YansWifiPhyHelper::Default();

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager"); 
    WifiMacHelper mac;

    Ssid wifiSsid = Ssid("ns3-wifi");

    // Access Point Configuration for BS0/1
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(wifiSsid));

    phyHelper0.SetChannel(channel0);

    // This is the device on the side of BS0 from N0
    NetDeviceContainer accessPoint0 =
        wifi.Install (phyHelper0, mac, nodes.Get(1));
     
    phyHelper1.SetChannel(channel1);

    // This is the device on the side of BS1 to N1
    NetDeviceContainer accessPoint1 =
        wifi.Install (phyHelper1, mac, nodes.Get(2));

    //Station configuration for N0/N1
    mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue(wifiSsid));

    // This is the device on the side of N0 to BS0
    NetDeviceContainer endDevice0 =
        wifi.Install(phyHelper0, mac, nodes.Get(0));

    // This is the device on the side of N1 from BS1
    NetDeviceContainer endDevice1 =
        wifi.Install(phyHelper1, mac, nodes.Get(3));


    // Set constant positions for all the devices
    MobilityHelper mobility;

    Ptr<ListPositionAllocator> locationVector =
        CreateObject<ListPositionAllocator> ();

    // Located at a `distance` distance from each other on a line
    
    locationVector -> Add(Vector(distance * 0, 0.0, 0.0));
    locationVector -> Add(Vector(distance * 1, 0.0, 0.0));
    locationVector -> Add(Vector(distance * 2, 0.0, 0.0));
    locationVector -> Add(Vector(distance * 3, 0.0, 0.0));

    mobility.SetPositionAllocator (locationVector);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    mobility.Install(nodes.Get(0));
    mobility.Install(nodes.Get(1));
    mobility.Install(nodes.Get(2));
    mobility.Install(nodes.Get(3));


    // Install Internet Stack in all devices
    InternetStackHelper stackHelper;
    stackHelper.Install(nodes); 

    // Set up addresses for all devices
    Ipv4AddressHelper addressHelper;

    NetDeviceContainer pathLeft(endDevice0, accessPoint0);
    NetDeviceContainer pathRight(accessPoint1, endDevice1);

    // Assign IP values to the LANs involed.
    addressHelper.SetBase("11.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ifLeft = addressHelper.Assign(pathLeft);
    
    addressHelper.SetBase("13.3.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ifRight = addressHelper.Assign(pathRight);
    
    addressHelper.SetBase("12.2.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ifMiddle = addressHelper.Assign(bsDevices);

    // Set up Routing tables so that directions are known at the start
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Random port
    uint16_t sinkPort = 4290;
 
    // Get Address assigned to endDevice1
    Address sinkAddr =
        InetSocketAddress(ifRight.GetAddress(1), sinkPort); // CHANGE


    // Install a packet sink in N1 (node 3), and setup TCP
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(),
                                                  sinkPort));
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(3)); // CHANGE

    // Set timeframe for sink Application
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(FINISH));


    // Create a TCP based client from SimpleClient that works on the Sender side.
    Ptr<Socket> tcpSocket =
        Socket::CreateSocket(nodes.Get(0),
                             TcpSocketFactory::GetTypeId());
    Ptr<SimpleClient> senderApp = CreateObject<SimpleClient>();

    // Send a large number of packets to reduce nonsteady state impacts.
    senderApp -> Setup(tcpSocket,
                       sinkAddr,
                       packetSize,
                       1000, // Number of packets
                       DataRate("100Mbps"));
    nodes.Get(0) -> AddApplication(senderApp);

    // Start 1s after sink starts to give it time.
    senderApp -> SetStartTime(Seconds(1.0));
    senderApp -> SetStopTime(Seconds(FINISH));
    
    // Install Flow Monitors on all devices
    Ptr<FlowMonitor> monitor = (new FlowMonitorHelper()) -> InstallAll();

    // Run the Simulator
    Simulator::Stop(Seconds(FINISH));
    Simulator::Run();

    // Get all flow statistics
    std::map < FlowId, FlowMonitor::FlowStats > flowStats =
        monitor -> GetFlowStats();

    auto dat = flowStats.begin() -> second;

    // Compute required statistics and display them.
 
    double rxBits = 8.0 * dat.rxBytes;

    // Total time is just delay between last packet received
    // and first packet sent
    double timeTaken = dat.timeLastRxPacket.GetSeconds() -
        dat.timeFirstTxPacket.GetSeconds();
    
    double throughput =  rxBits / timeTaken;

    // Compute Jains fairness index
    double fairness = FairnessIndex({throughput});
    
    std::cout << packetSize << ", " << throughput / 1000 << ", " << fairness << std::endl;

    // Serialize Flow Monitor output to an XML file and export it.
    
    std::string outputFilePath = tcpType + "_" + std::to_string(packetSize) + ".xml";
    monitor -> SerializeToXmlFile(outputFilePath, true, true);

    Simulator::Destroy();
}

// Calculate Raj Jain's Fairness Index on a vector of throughputs
double FairnessIndex(std::vector < double > xs) {
    double sum = 0.0, sumSq = 0.0;
    size_t n = xs.size();
    
    for (double x : xs) {
        sum += x; sumSq += x * x;
    }
    return (sum * sum) / (1.0 * n * sumSq);
}
