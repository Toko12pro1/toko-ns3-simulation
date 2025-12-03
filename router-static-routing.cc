/* Triangular WAN Topology with Static Routing
 *
 * Modified from the original two-node + router example.
 *
 * Topology:
 *
 *   HQ (n0) ----- Branch (n1) ----- DC (n2)
 *      \_____________________________/
 *
 * Links:
 *   n0 <-> n1 : 10.1.1.0/24  (5Mbps, 2ms)
 *   n1 <-> n2 : 10.1.2.0/24  (5Mbps, 2ms)
 *   n0 <-> n2 : 10.1.3.0/24  (5Mbps, 2ms)  <-- primary HQ-DC
 *
 * Static routes configured so HQ -> DC primary is direct (n0->n2),
 * backup is via Branch (n0->n1->n2), and symmetric return routes exist.
 * The HQ-DC link is disabled at t = 4.0s to demonstrate failover.
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TriangularStaticRouting");

int
main(int argc, char* argv[])
{
    // Optional: enable logging for echo applications
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create three nodes: n0 = HQ (client), n1 = Branch (router), n2 = DC (server)
    NodeContainer nodes;
    nodes.Create(3);

    Ptr<Node> n0 = nodes.Get(0);
    Ptr<Node> n1 = nodes.Get(1);
    Ptr<Node> n2 = nodes.Get(2);

    // Point-to-point helper (applies to all three links)
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // Create links
    NodeContainer n0n1(n0, n1);
    NetDeviceContainer d0d1 = p2p.Install(n0n1);

    NodeContainer n1n2(n1, n2);
    NetDeviceContainer d1d2 = p2p.Install(n1n2);

    NodeContainer n0n2(n0, n2);
    NetDeviceContainer d0d2 = p2p.Install(n0n2);

    // Mobility (fixed positions) for NetAnim visualization
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Set positions
    Ptr<MobilityModel> mob0 = n0->GetObject<MobilityModel>();
    Ptr<MobilityModel> mob1 = n1->GetObject<MobilityModel>();
    Ptr<MobilityModel> mob2 = n2->GetObject<MobilityModel>();

    mob0->SetPosition(Vector(5.0, 15.0, 0.0));   // HQ (left)
    mob1->SetPosition(Vector(10.0, 5.0, 0.0));   // Branch (center)
    mob2->SetPosition(Vector(15.0, 15.0, 0.0));  // DC (right)

    // Install Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;

    // Network 10.1.1.0/24 : n0 <-> n1
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if0_1 = address.Assign(d0d1);
    // if0_1.GetAddress(0) => n0 (10.1.1.1)
    // if0_1.GetAddress(1) => n1 (10.1.1.2)

    // Network 10.1.2.0/24 : n1 <-> n2
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer if1_2 = address.Assign(d1d2);
    // if1_2.GetAddress(0) => n1 (10.1.2.1)
    // if1_2.GetAddress(1) => n2 (10.1.2.2)

    // Network 10.1.3.0/24 : n0 <-> n2 (HQ-DC direct)
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer if0_2 = address.Assign(d0d2);
    // if0_2.GetAddress(0) => n0 (10.1.3.1)
    // if0_2.GetAddress(1) => n2 (10.1.3.2)

    // Enable IP forwarding on the Branch (n1)
    Ptr<Ipv4> ipv4Router = n1->GetObject<Ipv4>();
    ipv4Router->SetAttribute("IpForward", BooleanValue(true));

    // Static routing helper
    Ipv4StaticRoutingHelper staticRoutingHelper;

    // -- Configure static routes on HQ (n0)
    Ptr<Ipv4StaticRouting> staticN0 = staticRoutingHelper.GetStaticRouting(n0->GetObject<Ipv4>());

    // To reach DC networks:
    // Primary: direct via 10.1.3.2 (n2 on n0-n2 link) -> prefer direct (AddNetworkRouteTo with nextHop)
    // Add a network route to 10.1.2.0/24 (DC's connected network) with primary nextHop = n2's address on n0-n2
    // Note: We add routes to the remote network 10.1.2.0/24 (n1-n2 link) so traffic is forwarded correctly.
    // Primary route to DC network via direct link to n2
    staticN0->AddNetworkRouteTo(Ipv4Address("10.1.2.0"), Ipv4Mask("255.255.255.0"), if0_2.GetAddress(1));
    // Backup route to DC network via Branch (n1)
    staticN0->AddNetworkRouteTo(Ipv4Address("10.1.2.0"), Ipv4Mask("255.255.255.0"), if0_1.GetAddress(1));
    // Also add route to Branch's own network (10.1.1.0) via the n0-n1 interface
    staticN0->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("0.0.0.0"), 1);

    // -- Configure static routes on Branch (n1)
    Ptr<Ipv4StaticRouting> staticN1 = staticRoutingHelper.GetStaticRouting(n1->GetObject<Ipv4>());

    // Branch is directly connected to both 10.1.1.0 and 10.1.2.0; routing table will reflect that automatically.
    // Add route for 10.1.3.0 (HQ-DC link network) via n0 (for packets destined to n2's 10.1.3.2 address)
    staticN1->AddNetworkRouteTo(Ipv4Address("10.1.3.0"), Ipv4Mask("255.255.255.0"), if0_1.GetAddress(0));

    // -- Configure static routes on DC (n2)
    Ptr<Ipv4StaticRouting> staticN2 = staticRoutingHelper.GetStaticRouting(n2->GetObject<Ipv4>());

    // Return path to HQ networks:
    // Primary return path (direct) via n0 on n0-n2 link
    staticN2->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), if0_2.GetAddress(0));
    // Backup return path via Branch (n1)
    staticN2->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), if1_2.GetAddress(0));
    // Also ensure DC knows how to reach its directly-connected 10.1.2.0 network (done automatically by stack)

    // Print routing tables to file for verification
    Ptr<OutputStreamWrapper> routingStream =
        Create<OutputStreamWrapper>("scratch/triangular-static-routing.routes", std::ios::out);
    staticRoutingHelper.PrintRoutingTableAllAt(Seconds(1.0), routingStream);

    // Display IPs to stdout
    std::cout << "\n=== Network Configuration ===\n";
    std::cout << "HQ (n0) addresses: " << if0_1.GetAddress(0) << " (n0-n1), " << if0_2.GetAddress(0) << " (n0-n2)\n";
    std::cout << "Branch (n1) addresses: " << if0_1.GetAddress(1) << " (n0-n1), " << if1_2.GetAddress(0) << " (n1-n2)\n";
    std::cout << "DC (n2) addresses: " << if1_2.GetAddress(1) << " (n1-n2), " << if0_2.GetAddress(1) << " (n0-n2)\n";
    std::cout << "=============================\n\n";

    // UDP Echo server on DC (n2) - listen on port 9
    uint16_t port = 9;
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps = echoServer.Install(n2);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));

    // UDP Echo client on HQ (n0) targeting DC's address on the n1-n2 network (10.1.2.2)
    // We'll target the DC's address on its n1-n2 interface so initial traffic uses the primary path decision.
    UdpEchoClientHelper echoClient(if1_2.GetAddress(1), port);
    echoClient.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps = echoClient.Install(n0);
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(20.0));

    // Enable PCAP and NetAnim as before
    p2p.EnablePcapAll("scratch/triangular-static-routing");
    AnimationInterface anim("scratch/triangular-static-routing.xml");
    anim.UpdateNodeDescription(n0, "HQ\n10.1.1.1 | 10.1.3.1");
    anim.UpdateNodeDescription(n1, "Branch\n10.1.1.2 | 10.1.2.1");
    anim.UpdateNodeDescription(n2, "DC\n10.1.2.2 | 10.1.3.2");

    anim.UpdateNodeColor(n0, 0, 255, 0);
    anim.UpdateNodeColor(n1, 255, 255, 0);
    anim.UpdateNodeColor(n2, 0, 0, 255);

    // --- Optional: FlowMonitor to compare primary vs backup latency (small example)
    FlowMonitorHelper fmHelper;
    Ptr<FlowMonitor> flowMon = fmHelper.InstallAll();

    // --- Simulate link failure: disable HQ-DC link (n0<->n2) at t = 4s
    // We simulate "failure" by setting MTU to 0 on both devices of that link.
    // (This is a lightweight way to simulate the link going down.)
    Simulator::Schedule(Seconds(4.0), &NetDevice::SetMtu, d0d2.Get(0), 0u);
    Simulator::Schedule(Seconds(4.0), &NetDevice::SetMtu, d0d2.Get(1), 0u);

    // Stop and run
    Simulator::Stop(Seconds(21.0));
    Simulator::Run();

    // FlowMonitor stats (optional quick summary printed to stdout)
    flowMon->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(fmHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMon->GetFlowStats();
    for (auto const& entry : stats)
    {
        FlowId fid = entry.first;
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(fid);
        std::cout << "Flow " << fid << " : " << t.sourceAddress << " -> " << t.destinationAddress
                  << " TxPackets=" << entry.second.txPackets
                  << " RxPackets=" << entry.second.rxPackets
                  << " LostPackets=" << entry.second.lostPackets
                  << " MeanDelay=" << (entry.second.delaySum.GetSeconds() / std::max(1u, entry.second.rxPackets))
                  << "s\n";
    }

    Simulator::Destroy();

    std::cout << "\n=== Simulation Complete ===\n";
    std::cout << "Animation trace saved to: scratch/triangular-static-routing.xml\n";
    std::cout << "Routing tables saved to: scratch/triangular-static-routing.routes\n";
    std::cout << "PCAP traces saved to: scratch/triangular-static-routing-*.pcap\n";
    std::cout << "NetAnim + FlowMonitor outputs are available in the scratch/ directory.\n";

    return 0;
}
