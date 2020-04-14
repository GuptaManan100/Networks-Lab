//2020 - Quarantine
//Group 8 - Manan Gupta, Ashish Agarwal, Mriganka Basu Roy Chaudhary
//Networks Lab Assignment 4 - Part 1

//Header Files
#include <iostream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/enum.h" 

#define pb push_back

using namespace ns3;

//Class for client application, Taken from seven.cc of the exmaple tutorial of ns-3.
class ClientApp : public Application
{
public:
  ClientApp (); //Constructor
  virtual ~ClientApp (); //Deconstructor

  //Initialize the object parameters
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;	//Socket
  Address         m_peer; //Address of receiver
  uint32_t        m_packetSize;	//Packet Size
  uint32_t        m_nPackets;	//Total number of packets to be sent
  DataRate        m_dataRate;	//Data rate
  EventId         m_sendEvent;
  bool            m_running;	//State of runnning
  uint32_t        m_packetsSent; //Number of packets sent
};

//Constructor provides initial value to all the variables.
ClientApp::ClientApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

ClientApp::~ClientApp ()
{
  m_socket = 0;
}

//Setup initializes all the parameters.
void
ClientApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

//Start the application
void
ClientApp::StartApplication (void)
{
  m_running = true; //Set the running state to true
  m_packetsSent = 0; //Set the number of packets sent to 0
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
    //Connect to the peer after binding
  m_socket->Connect (m_peer);
  //Send the packets
  SendPacket ();
}

//Stop Application function
void
ClientApp::StopApplication (void)
{
  m_running = false; //Set running state to false

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close (); //close the socket
    }
}

//Funciton to send the packet
void
ClientApp::SendPacket (void)
{
	//Create a new packet of given packet size and send it
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  //In case the last packet is also sent, schedule for closure.
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
ClientApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &ClientApp::SendPacket, this);
    }
}

//Main function
int main(int argc, char *argv[])
{
	std::cout << std::fixed;
	std::string tcpAgent = "TCPveno";

	//Take a command line argument for TCPAgent, default to veno
	CommandLine cmd;
	cmd.AddValue ("tcpAgent", "TCP Agent to use", tcpAgent);
	cmd.Parse (argc, argv);

	if(tcpAgent == "TCPvegas")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
	}
	else if(tcpAgent == "TCPveno")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
	}
	else if(tcpAgent == "TCPwestwood")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpWestwood"));
	}
	else
	{
		//In case of incorrect parameters exit the application after printing an error message
		std::cout<<"Incorrect Parameters"<<std::endl;
		exit(1);
	}

	//Vector to hold the size of the packets
	std::vector<int> sizPack;
	//Push back all the different sizes.
	sizPack.pb(40);
	sizPack.pb(44);
	sizPack.pb(48);
	sizPack.pb(52);
	sizPack.pb(60);
	sizPack.pb(250);
	sizPack.pb(300);
	sizPack.pb(552);
	sizPack.pb(576);
	sizPack.pb(628);
	sizPack.pb(1420);
	sizPack.pb(1500);

	//Loop through all the packet sizes.
	for(auto siz : sizPack)
	{
		//Create a node container with 4 nodes
		NodeContainer nodes;
		nodes.Create (4);

		//Create point to point link of given specification
		PointToPointHelper p2pOuter;
		p2pOuter.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
		p2pOuter.SetChannelAttribute ("Delay", StringValue ("20ms"));
		int x = 100*20*1000/(8*siz);  //x is the number of packets it can receive in the queue buffer.
		std::string sp;
		sp = std::to_string(x)+"p";
		//std::cout<<sp<<std::endl;
		p2pOuter.SetQueue ("ns3::DropTailQueue","MaxSize", StringValue(sp));
		
		//Create Device containers and attach the p2p connection.
		NetDeviceContainer devicesL,devicesM,devicesR;
		devicesL = p2pOuter.Install (nodes.Get(0),nodes.Get(1));
		devicesR = p2pOuter.Install (nodes.Get(2),nodes.Get(3));

		//Repeat the above steps for the second connection
		PointToPointHelper p2pInner;
		p2pInner.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
		p2pInner.SetChannelAttribute ("Delay", StringValue ("50ms"));
		x = 10*50*1000/(8*siz);
		sp = std::to_string(x)+"p";
		//std::cout<<sp<<std::endl;
		p2pInner.SetQueue ("ns3::DropTailQueue","MaxSize", StringValue(sp));
		devicesM = p2pInner.Install(nodes.Get(1),nodes.Get(2));

		//Add the internet stack to all the nodes.
		InternetStackHelper stack;
		stack.Install (nodes);

		//Random server port number
		uint16_t serverPort = 4290;
		Address serverAddress;
		Address anyAddress;

		//Set Ipv4 addresses for the connections.
		Ipv4AddressHelper addressL;
		addressL.SetBase("10.10.10.0","255.255.255.0");
		Ipv4InterfaceContainer interfacesL = addressL.Assign (devicesL);

		Ipv4AddressHelper addressM;
		addressM.SetBase("10.10.11.0","255.255.255.0");
		Ipv4InterfaceContainer interfacesM = addressM.Assign (devicesM);

		Ipv4AddressHelper addressR;
		addressR.SetBase("10.10.12.0","255.255.255.0");
		Ipv4InterfaceContainer interfacesR = addressR.Assign (devicesR);

		//Get the server address and any address for the client
		serverAddress = InetSocketAddress (interfacesR.GetAddress(1),serverPort);
		anyAddress = InetSocketAddress(Ipv4Address::GetAny(),serverPort);

		//Set the global routing table.
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		//Attach a packet collector to the server and specify the start and stop times.
		PacketSinkHelper packetServerHelper ("ns3::TcpSocketFactory", anyAddress);
		ApplicationContainer serverApp = packetServerHelper.Install (nodes.Get (3));
		serverApp.Start (Seconds (0.));
		serverApp.Stop (Seconds (20.));

		//Create a client socket
		Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
		//Create an application
		Ptr<ClientApp> clientApp = CreateObject<ClientApp> ();
		//Setup the application
		clientApp->Setup (ns3TcpSocket, serverAddress, siz, 10000, DataRate ("20Mbps"));
		//Add the application onto the client
		nodes.Get (0)->AddApplication (clientApp);
		//Set the start and stop times for the client
		clientApp->SetStartTime (Seconds (1.));
		clientApp->SetStopTime (Seconds (20.));

		//Create a flow monitor
		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll();
		
		//Start the simulation
		Simulator::Stop (Seconds (20));
		Simulator::Run ();

		//Get the stats from the flow monitor
		std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

		//initialize a few variables
		double totTime, totBits, throughput, sumThr, sumsqThr;
		sumThr = 0;
		sumsqThr = 0;
		int coun = 0;

		//Update the total bits sent, and the total time.
		auto iter = stats.begin();
		totBits = 8.0 * iter->second.rxBytes;
		totTime = iter->second.timeLastRxPacket.GetSeconds();
		totTime -= iter->second.timeFirstTxPacket.GetSeconds();

		//std::cout<<"totBits: "<<totBits<<" time: "<<totTime<<std::endl;
		//calculate throughput and find the fairness index.
		throughput = totBits/totTime;
		sumThr+= throughput;
		sumsqThr += throughput*throughput;
		coun++;

		//Output thr information
		double fairness = (sumThr*sumThr)/(sumsqThr*(coun+0.0));
		std::cout<<"Packet Size: "<<siz<<" ";
		std::cout<<"Total Bits: "<<totBits<<" Time:"<<totTime<<" ";
		std::cout<<"Throughput: "<<throughput<<" Fairness: "<<fairness<<std::endl;

		//Also output the serialized data in xml format
		monitor->SerializeToXmlFile(tcpAgent+"_"+std::to_string(siz)+".xml", true, true);

		//end simulation
		Simulator::Destroy ();
	}

	return 0;
}
