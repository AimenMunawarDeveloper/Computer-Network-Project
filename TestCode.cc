#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/point-to-point-module.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include <iostream>
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket.h"
#include "ns3/netanim-module.h"
#include <vector>
#include <map>
#include <numeric>
#include <unistd.h>
#include "ns3/packet.h"
#include "ns3/udp-socket-factory.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("hadia");

double SetRandomBandwidth(PointToPointHelper& p2p, uint32_t node1, uint32_t node2) {
    Ptr<UniformRandomVariable> randBandwidth = CreateObject<UniformRandomVariable>();
    double bandwidth = randBandwidth->GetValue(50, 250);
    p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(bandwidth * 1e6))); // Convert to bps
    NS_LOG_INFO("Link between nodes " << node1 << " and " << node2 << " configured with bandwidth: " << bandwidth << " Mbps");
    return bandwidth;
}

int minDistance(std::vector<double>& dist, std::vector<bool>& sptSet) {
    double min = std::numeric_limits<double>::max();
    int minIndex;

    for (uint32_t v = 0; v < dist.size(); ++v) {
        if (!sptSet[v] && dist[v] <= min) {
            min = dist[v];
            minIndex = v;
        }
    }

    return minIndex;
}

void printSolution(std::vector<double>& dist) {
    for (uint32_t i = 0; i < dist.size(); ++i) {
        if (dist[i] == std::numeric_limits<double>::max()) {
            NS_LOG_INFO(i << "      No path");
        } else {
            NS_LOG_INFO(i << "      " << dist[i]);
        }
    }
}

std::vector<double> dijkstraIntraAS(std::vector<std::vector<double>>& graph, uint32_t src, uint32_t gateway) {
    uint32_t V = graph.size();
    std::vector<double> dist(V, std::numeric_limits<double>::max());
    std::vector<bool> sptSet(V, false);

    dist[src] = 0;

    for (uint32_t count = 0; count < V - 1; ++count) {
        int u = minDistance(dist, sptSet);
        sptSet[u] = true;

        for (uint32_t v = 0; v < V; ++v) {
            if (!sptSet[v] && graph[u][v] != 0 && dist[u] != std::numeric_limits<double>::max() &&
                dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    return dist;
}
// Declare totalDelay and packetsReceived as global variables
double totalDelay = 0.0;
uint32_t packetsReceived = 0;
void SendPacket(Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destinationAddress, uint16_t port) {
    socket->SendTo(packet, 0, InetSocketAddress(destinationAddress, port));
    // Update totalDelay and packetsReceived during packet transmission
    totalDelay += Simulator::Now().GetSeconds();
    packetsReceived++;
}

int main() {
LogComponentEnable("hadia", LOG_LEVEL_INFO);

PacketMetadata::Enable();
// Set seed
RngSeedManager::SetSeed (Time (0).GetSeconds () + getpid ());
Ptr<UniformRandomVariable> randBandwidth = CreateObject<UniformRandomVariable>();
// Create 50 nodes
NodeContainer nodes;
nodes.Create(50);

// Install Internet Stack on the nodes
InternetStackHelper internet;
internet.Install(nodes);

// Create interfaces and devices
NetDeviceContainer devices;
Ipv4InterfaceContainer interfaces;
NetDeviceContainer device;
Ipv4InterfaceContainer interface;
Ipv4AddressHelper address;
PointToPointHelper p2p;
address.SetBase("10.0.0.0", "255.0.0.0");


std::vector<std::vector<double>> graph(50, std::vector<double>(50, 0.0));

// 6 and 27
PointToPointHelper p2p1;
p2p1.SetChannelAttribute("Delay", StringValue("2ms"));
SetRandomBandwidth(p2p1, 6, 27);
device = p2p1.Install(nodes.Get(6), nodes.Get(27));
// Assign IP addresses
interface = address.Assign(device);
devices.Add(device);
interfaces.Add(interface);
// Retrieve link data rate
DataRateValue rate1;
devices.Get(devices.GetN() - 1)->GetAttribute("DataRate", rate1);
double linkBandwidth1 = rate1.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
graph[6][27] = linkBandwidth1;

// 11 and 24
PointToPointHelper p2p2;
p2p2.SetChannelAttribute("Delay", StringValue("2ms"));
SetRandomBandwidth(p2p2, 11, 24);
device = p2p2.Install(nodes.Get(11), nodes.Get(24));
// Assign IP addresses
interface = address.Assign(device);
devices.Add(device);
interfaces.Add(interface);
// Retrieve link data rate
DataRateValue rate2;
devices.Get(devices.GetN() - 1)->GetAttribute("DataRate", rate2);
double linkBandwidth2 = rate2.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
graph[11][24] = linkBandwidth2;

// 49 and 23
PointToPointHelper p2p3;
p2p3.SetChannelAttribute("Delay", StringValue("2ms"));
SetRandomBandwidth(p2p3, 23, 49);
device = p2p3.Install(nodes.Get(23), nodes.Get(49));
// Assign IP addresses
interface = address.Assign(device);
devices.Add(device);
interfaces.Add(interface);
// Retrieve link data rate
DataRateValue rate3;
devices.Get(devices.GetN() - 1)->GetAttribute("DataRate", rate3);
double linkBandwidth3 = rate3.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
graph[23][49] = linkBandwidth3;

// 26 and 39
PointToPointHelper p2p4;
p2p4.SetChannelAttribute("Delay", StringValue("2ms"));
SetRandomBandwidth(p2p4, 26, 39);
device = p2p4.Install(nodes.Get(26), nodes.Get(39));
// Assign IP addresses
interface = address.Assign(device);
devices.Add(device);
interfaces.Add(interface);
// Retrieve link data rate
DataRateValue rate4;
devices.Get(devices.GetN() - 1)->GetAttribute("DataRate", rate4);
double linkBandwidth4 = rate4.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
graph[26][39] = linkBandwidth4;

for (uint32_t i = 0; i < 10; ++i) {
        PointToPointHelper p2pToGateway;
        SetRandomBandwidth(p2pToGateway, i, 6);
        p2pToGateway.SetChannelAttribute("Delay", StringValue("2ms"));
        device = p2pToGateway.Install(nodes.Get(i), nodes.Get(6));
        interface = address.Assign(device);
        devices.Add(device);
        interfaces.Add(interface);
        NetDeviceContainer devicesWithinAS;

        for (uint32_t j = 0; j < 10; ++j) {
            if (i != j) {
                PointToPointHelper p2pWithinAS;
                SetRandomBandwidth(p2pWithinAS, i, j);
                p2pWithinAS.SetChannelAttribute("Delay", StringValue("2ms"));
                device = p2pWithinAS.Install(nodes.Get(i), nodes.Get(j));
                interface = address.Assign(device);
                devicesWithinAS.Add(device);
                interfaces.Add(interface);

                // Retrieve link data rate
                DataRateValue rate;
                devicesWithinAS.Get(j)->GetAttribute("DataRate", rate);
                double linkBandwidth = rate.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
                graph[i][j] = linkBandwidth;
            }
        }
    }

    // Connect nodes between AS 1 and AS 2 (9-10)
    PointToPointHelper p2pAS1toAS2;
    SetRandomBandwidth(p2pAS1toAS2, 9, 10);
    p2pAS1toAS2.SetChannelAttribute("Delay", StringValue("2ms"));
    device = p2pAS1toAS2.Install(nodes.Get(9), nodes.Get(10));
    interface = address.Assign(device);
    devices.Add(device);
    interfaces.Add(interface);

    // Connect nodes within AS 2 (10-19)
    for (uint32_t i = 10; i < 20; ++i) {
        PointToPointHelper p2pToGateway;
        SetRandomBandwidth(p2pToGateway, i, 11);
        p2pToGateway.SetChannelAttribute("Delay", StringValue("2ms"));
        device = p2pToGateway.Install(nodes.Get(i), nodes.Get(11));
        interface = address.Assign(device);
        devices.Add(device);
        interfaces.Add(interface);

        NetDeviceContainer devicesWithinAS;

        for (uint32_t j = 10; j < 20; ++j) {
            if (i != j) {
                PointToPointHelper p2pWithinAS;
                SetRandomBandwidth(p2pWithinAS, i, j);
                p2pWithinAS.SetChannelAttribute("Delay", StringValue("2ms"));
                device = p2pWithinAS.Install(nodes.Get(i), nodes.Get(j));
                interface = address.Assign(device);
                devicesWithinAS.Add(device);
                interfaces.Add(interface);

                // Retrieve link data rate
                DataRateValue rate;
                devicesWithinAS.Get(j - 10)->GetAttribute("DataRate", rate);
                double linkBandwidth = rate.Get().GetBitRate() / 1e6;
                graph[i][j] = linkBandwidth;
            }
        }
    }

    // Connect nodes between AS 2 and AS 3 (19-20)
    PointToPointHelper p2pAS2toAS3;
    SetRandomBandwidth(p2pAS2toAS3, 19, 20);
    p2pAS2toAS3.SetChannelAttribute("Delay", StringValue("2ms"));
    device = p2pAS2toAS3.Install(nodes.Get(19), nodes.Get(20));
    interface = address.Assign(device);
    devices.Add(device);
    interfaces.Add(interface);
    // Connect nodes within the third AS (20-29)
    // Connect nodes within AS 3 (20-29)
for (uint32_t i = 20; i < 30; ++i) {
    PointToPointHelper p2pToGateway;
    SetRandomBandwidth(p2pToGateway, i, 23);
    p2pToGateway.SetChannelAttribute("Delay", StringValue("2ms"));
    device = p2pToGateway.Install(nodes.Get(i), nodes.Get(23));
    interface = address.Assign(device);
    devices.Add(device);
    interfaces.Add(interface);

    NetDeviceContainer devicesWithinAS;

    for (uint32_t j = 20; j < 30; ++j) {
        if (i != j) {
            PointToPointHelper p2pWithinAS;
            SetRandomBandwidth(p2pWithinAS, i, j);
            p2pWithinAS.SetChannelAttribute("Delay", StringValue("2ms"));
            device = p2pWithinAS.Install(nodes.Get(i), nodes.Get(j));
            interface = address.Assign(device);
            devicesWithinAS.Add(device);
            interfaces.Add(interface);

            // Retrieve link data rate
            DataRateValue rate;
            devicesWithinAS.Get(j - 20)->GetAttribute("DataRate", rate);
            double linkBandwidth = rate.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
            graph[i][j] = linkBandwidth;
        }
    }
}

// Connect nodes between AS 3 and AS 4 (29-30)
PointToPointHelper p2pAS3toAS4;
SetRandomBandwidth(p2pAS3toAS4, 29, 30);
p2pAS3toAS4.SetChannelAttribute("Delay", StringValue("2ms"));
device = p2pAS3toAS4.Install(nodes.Get(29), nodes.Get(30));
interface = address.Assign(device);
devices.Add(device);
interfaces.Add(interface);

// Connect nodes within AS 4 (30-39)
for (uint32_t i = 30; i < 40; ++i) {
    PointToPointHelper p2pToGateway;
    SetRandomBandwidth(p2pToGateway, i, 39);
    p2pToGateway.SetChannelAttribute("Delay", StringValue("2ms"));
    device = p2pToGateway.Install(nodes.Get(i), nodes.Get(39));
    interface = address.Assign(device);
    devices.Add(device);
    interfaces.Add(interface);

    NetDeviceContainer devicesWithinAS;

    for (uint32_t j = 30; j < 40; ++j) {
        if (i != j) {
            PointToPointHelper p2pWithinAS;
            SetRandomBandwidth(p2pWithinAS, i, j);
            p2pWithinAS.SetChannelAttribute("Delay", StringValue("2ms"));
            device = p2pWithinAS.Install(nodes.Get(i), nodes.Get(j));
            interface = address.Assign(device);
            devicesWithinAS.Add(device);
            interfaces.Add(interface);

            // Retrieve link data rate
            DataRateValue rate;
            devicesWithinAS.Get(j - 30)->GetAttribute("DataRate", rate);
            double linkBandwidth = rate.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
            graph[i][j] = linkBandwidth;
        }
    }
}

// Connect nodes between AS 4 and AS 5 (39-40)
PointToPointHelper p2pAS4toAS5;
SetRandomBandwidth(p2pAS4toAS5, 39, 40);
p2pAS4toAS5.SetChannelAttribute("Delay", StringValue("2ms"));
device = p2pAS4toAS5.Install(nodes.Get(39), nodes.Get(40));
interface = address.Assign(device);
devices.Add(device);
interfaces.Add(interface);

for (uint32_t i = 40; i < 50; ++i) {
        PointToPointHelper p2pToGateway;
        SetRandomBandwidth(p2pToGateway, i, 49);
        p2pToGateway.SetChannelAttribute("Delay", StringValue("2ms"));
        device = p2pToGateway.Install(nodes.Get(i), nodes.Get(49));
        interface = address.Assign(device);
        devices.Add(device);
        interfaces.Add(interface);

        NetDeviceContainer devicesWithinAS;

        for (uint32_t j = 40; j < 50; ++j) {
            if (i != j) {
                PointToPointHelper p2pWithinAS;
                SetRandomBandwidth(p2pWithinAS, i, j);
                p2pWithinAS.SetChannelAttribute("Delay", StringValue("2ms"));
                device = p2pWithinAS.Install(nodes.Get(i), nodes.Get(j));
                interface = address.Assign(device);
                devicesWithinAS.Add(device);
                interfaces.Add(interface);

                // Retrieve link data rate
                DataRateValue rate;
                devicesWithinAS.Get(j - 40)->GetAttribute("DataRate", rate);
                double linkBandwidth = rate.Get().GetBitRate() / 1e6; // Get link bandwidth in Mbps
                graph[i][j] = linkBandwidth;
            }
        }
    }
 
Simulator::Stop(Seconds(10));
// Create an animation interface
AnimationInterface anim("hadia.xml");

// Create a uniform random variable for Link Bandwidth
Ptr<UniformRandomVariable> randPos = CreateObject<UniformRandomVariable> ();
randPos->SetAttribute("Min", DoubleValue(0));
randPos->SetAttribute("Max", DoubleValue(10));

// First AS
for (uint32_t i = 0; i < 10; ++i) {
double x = randPos->GetInteger();
double y = randPos->GetInteger() + 40;
anim.SetConstantPosition(nodes.Get(i), x, y);
}

// Second AS
randPos->SetAttribute("Min", DoubleValue(40));
randPos->SetAttribute("Max", DoubleValue(50));

for (uint32_t i = 10; i < 20; ++i) {
double x = randPos->GetInteger();
double y = randPos->GetInteger() - 40;
anim.SetConstantPosition(nodes.Get(i), x, y);
}

// Third AS
randPos->SetAttribute("Min", DoubleValue(40));
randPos->SetAttribute("Max", DoubleValue(50));

for (uint32_t i = 20; i < 30; ++i) {
double x = randPos->GetInteger();
double y = randPos->GetInteger();
anim.SetConstantPosition(nodes.Get(i), x, y);
}

// Fourth AS
randPos->SetAttribute("Min", DoubleValue(40));
randPos->SetAttribute("Max", DoubleValue(50));

for (uint32_t i = 30; i < 40; ++i) {
double x = randPos->GetInteger();
double y = randPos->GetInteger() + 40;
anim.SetConstantPosition(nodes.Get(i), x, y);
}


// Fifth AS
randPos->SetAttribute("Min", DoubleValue(70));
randPos->SetAttribute("Max", DoubleValue(80));

for (uint32_t i = 40; i < 50; ++i) {
double x = randPos->GetInteger();
double y = randPos->GetInteger() - 30;
anim.SetConstantPosition(nodes.Get(i), x, y);
}


std::vector<double> overallDistances;

NS_LOG_INFO("Overall Shortest path from node 0 to 48:");

    // Step 1: Calculate the shortest path from node 0 to gateway 6 (AS 1)
    std::vector<double> distancesAS1;
    distancesAS1 = dijkstraIntraAS(graph, 0, 6);
    NS_LOG_INFO("Shortest paths from node 0 to gateway 6: " << distancesAS1[6]);
    overallDistances.push_back(distancesAS1[6]);

    // Step 2: Take the randomly assigned cost on the link from gateway 6 to gateway 27
    NS_LOG_INFO("Link cost for Gateway 6 to gateway 27: " << graph[6][27]);
    overallDistances.push_back(graph[6][27]);

    // Step 3: Calculate the shortest path from node 27 to node 23 (AS 3)
    std::vector<double> distancesAS2;
    distancesAS2 = dijkstraIntraAS(graph, 27, 23);
    NS_LOG_INFO("Shortest paths from node 27 to gateway 23: " << distancesAS2[23]);
    overallDistances.push_back(distancesAS2[23]);

    // Step 4: Take the randomly assigned cost on the link from gateway 23 to gateway 49
    NS_LOG_INFO("Link cost for Gateway 23 to gateway 49: " << graph[23][49]);
    overallDistances.push_back(graph[23][49]);

    // Step 5: Calculate the shortest path from node 49 to node 47 (AS 5)
    std::vector<double> distancesAS3;
    distancesAS3 = dijkstraIntraAS(graph, 49, 48);
    NS_LOG_INFO("Shortest paths from node 49 to node 48: " << distancesAS3[48]);
    overallDistances.push_back(distancesAS3[48]);

    // Calculate and print the total Shortest Path
    double totalShortestPath = std::accumulate(overallDistances.begin(), overallDistances.end(), 0.0);
    NS_LOG_INFO("Total Shortest Path from node 0 to 48: " << totalShortestPath);
   
    // Step 2: Transmit five packets from source to destination
    uint32_t sourceNode = 0;
    uint32_t destinationNode = 48;
    uint32_t numPackets = 5;
    double interval = 1.0;  // 1 second interval between packets

    for (uint32_t packetNum = 0; packetNum < numPackets; ++packetNum) {
   Ptr<Socket> sourceSocket = Socket::CreateSocket(nodes.Get(sourceNode), UdpSocketFactory::GetTypeId());
   sourceSocket->Bind();

   Ptr<Socket> destSocket = Socket::CreateSocket(nodes.Get(destinationNode), UdpSocketFactory::GetTypeId());
   destSocket->Bind(InetSocketAddress(interfaces.GetAddress(destinationNode), 9));

   Ptr<Packet> packet = Create<Packet>(1024); // Assuming packet size is 1024 bytes

   // Schedule packet transmission with a fixed interval
   Simulator::Schedule(Seconds(packetNum * interval), &SendPacket, sourceSocket, packet, interfaces.GetAddress(destinationNode), 9);
    }
    // Step 3: Run the simulation
    Simulator::Stop(Seconds(5));
    Simulator::Run();

    // Step 4: Calculate average delay and PDR
    double averageDelay = totalDelay / packetsReceived;
    double packetDeliveryRatio = static_cast<double>(packetsReceived) / numPackets;

    // Step 5: Print results for the current iteration
    NS_LOG_INFO("Average Delay: " << averageDelay << " seconds");
    NS_LOG_INFO("Packet Delivery Ratio: " << packetDeliveryRatio);


anim.EnablePacketMetadata(true);
anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10));

Simulator::Run();
Simulator::Destroy();

}

