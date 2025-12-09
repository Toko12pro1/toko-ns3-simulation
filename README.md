
Step-by-Step Guide to Run the Simulation
1. Prerequisites
Make sure you have NS-3 installed. If not, install it:
bash# Clone NS-3 (version 3.36 or later recommended)
git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3
cd ns-3

# Build NS-3
./ns3 configure --enable-examples --enable-tests
./ns3 build
2. Copy the Simulation File
Copy the code I provided above into the NS-3 scratch directory:
bash# From the ns-3 root directory
nano scratch/multi-hop-wan-simulation.cc
# Paste the code and save (Ctrl+O, Enter, Ctrl+X)
3. Compile the Simulation
bash./ns3 build
4. Run the Simulation
Basic run with static routing:
bash./ns3 run scratch/multi-hop-wan-simulation
Run with command-line options:
bash# Static routing (default) - 20 second simulation
./ns3 run "scratch/multi-hop-wan-simulation --static=true --time=20"

# With verbose logging
./ns3 run "scratch/multi-hop-wan-simulation --static=true --time=20" --verbose

# Change packet size and data rate
./ns3 run "scratch/multi-hop-wan-simulation --size=512 --rate=1Mb/s"
```

### 5. **Expected Output**

You should see output like this:
```
Creating Topology...
Using STATIC ROUTING
Configuring Static Routes...

=== INITIAL ROUTING TABLES ===
[Routing table contents...]

Installing Applications...
Scheduling link failure at t=5.0s
Starting Simulation...

=== STATISTICS BEFORE FAILURE (t=4.5s) ===
Packets Transmitted: 45
Packets Received: 45
Packets Dropped: 0
Packet Loss Rate: 0%

=== DISABLING PRIMARY LINK at 5s ===

=== STATISTICS AFTER FAILURE (t=10.0s) ===
Packets Transmitted: 100
Packets Received: 45  <-- With static routing, no recovery!
Packets Dropped: 55
Packet Loss Rate: 55%

=== FLOW STATISTICS ===
[Detailed flow statistics...]
6. Analyze Results
The simulation generates an XML file with detailed statistics:
bash# View the FlowMonitor XML output
cat multi-hop-wan-flowmon.xml

# Or use a Python script to parse it
7. Generate PCAP Files (Optional)
To enable packet capture for Wireshark analysis, uncomment this line in the code:
cppp2p.EnablePcapAll("multi-hop-wan");
Then analyze with Wireshark:
bashwireshark multi-hop-wan-0-0.pcap &

Understanding the Results
With Static Routing (default):

✅ Before t=5s: All packets delivered via primary path (Network2)
❌ After t=5s: All packets dropped (no automatic failover)
This demonstrates why static routing fails for fault tolerance

Key Observations:

Before Failure (0-5s):

Packet loss: 0%
Packets flow: Branch-C → DC-A (Network1) → DR-B (Network2)


During Failure (5-20s with static routing):

Packet loss: ~100%
Static routes don't update automatically
Backup path (Network3) exists but is never used




Modifying the Code for Different Scenarios
A. To Implement Dynamic Routing (OSPF Alternative):
Replace the static routing section with:
cppif (!useStaticRouting)
{
  NS_LOG_INFO("Using GLOBAL ROUTING (simulates OSPF convergence)");
  // Global routing will automatically compute backup paths
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}
Run with:
bash./ns3 run "scratch/multi-hop-wan-simulation --static=false"
B. To Change Failure Time:
Modify line:
cppSimulator::Schedule(Seconds(5.0), &DisablePrimaryLink, ...);
// Change to:
Simulator::Schedule(Seconds(10.0), &DisablePrimaryLink, ...);
C. To Add More Traffic:
Modify the client application:
cppclient.SetAttribute("MaxPackets", UintegerValue(10000)); // More packets
client.SetAttribute("Interval", TimeValue(Seconds(0.01))); // Faster rate

Troubleshooting
Error: "No such file or directory"
bash# Make sure you're in the ns-3 root directory
cd /path/to/ns-3
Error: "Command not found: ./ns3"
bash# For older NS-3 versions, use:
./waf --run scratch/multi-hop-wan-simulation
No output appearing:
bash# Enable verbose mode
export NS_LOG=MultiHopWanSimulation=level_all
./ns3 run scratch/multi-hop-wan-simulation# toko-ns3-simulation
