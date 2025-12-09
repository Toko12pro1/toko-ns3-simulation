# NS-3 Policy-Based Routing - Build and Run Guide

## Step 1: Save the Code

Save the complete C++ code as a file in your NS-3 scratch directory:

```bash
cd ns-3.xx/scratch/
nano policy-based-routing.cc
# Paste the code and save (Ctrl+O, Ctrl+X)
```

## Step 2: Configure NS-3 (if not already done)

```bash
cd ~/ns-3.xx/
./ns3 configure --enable-examples --enable-tests
```

## Step 3: Build the Simulation

```bash
./ns3 build
```

Or build just your file:

```bash
./ns3 build policy-based-routing
```

## Step 4: Run the Simulation

```bash
./ns3 run scratch/policy-based-routing
```

With custom simulation time:

```bash
./ns3 run "scratch/policy-based-routing --simTime=120"
```

## Step 5: Expected Output

You should see output like:

```
=== Policy-Based Routing Simulation ===
Creating network topology...
Installing applications...
Installing FlowMonitor...
Creating SD-WAN Controller...
Running simulation for 60 seconds...

=== SD-WAN Controller Update at 1s ===
  Video Flow Latency: 12.5 ms
  Data Flow Latency: 7.3 ms

=== SD-WAN Controller Update at 2s ===
  Video Flow Latency: 12.8 ms
  Data Flow Latency: 7.1 ms

...

!!! SIMULATING PRIMARY LINK DEGRADATION at t=30s !!!

=== SD-WAN Controller Update at 31s ===
  Video Flow Latency: 52.3 ms
  Data Flow Latency: 7.2 ms
*** PRIMARY LINK DEGRADED! Switching video traffic to secondary link ***
  Updated routing: Video traffic now uses interface 2

...

!!! RESTORING PRIMARY LINK at t=45s !!!

=== SD-WAN Controller Update at 46s ===
  Video Flow Latency: 13.1 ms
  Data Flow Latency: 7.4 ms
*** Primary link recovered. Switching video traffic back ***
  Updated routing: Video traffic now uses interface 1

=== Final Statistics ===
Flow 1 (10.1.1.1:49153 -> 10.1.1.2:5004)
  Tx Packets: 3000
  Rx Packets: 2998
  Lost Packets: 2
  Avg Delay: 15.6 ms
  Throughput: 0.24 Mbps

Flow 2 (10.1.1.1:49154 -> 10.1.1.2:5005)
  Tx Packets: 300
  Rx Packets: 299
  Lost Packets: 1
  Avg Delay: 8.2 ms
  Throughput: 1.68 Mbps

Simulation complete!
```

## Code Structure Overview

### 1. **Traffic Generation Classes**
- `VideoTrafficApp`: Generates small, periodic packets (200 bytes every 20ms)
- `DataTrafficApp`: Generates large, bursty packets (1400 bytes every 100ms)

### 2. **SD-WAN Controller**
- `SdWanController`: Monitors link metrics every 1 second
- Uses FlowMonitor to measure real-time latency
- Applies policy: if video latency > 30ms, switch to secondary link
- Updates global routing variable `g_videoFlowInterface`

### 3. **Network Topology**
```
Studio (n0) ----[Primary: 10Mbps, 10ms]---- Cloud (n1)
     |                                          |
     +----[Secondary: 5Mbps, 5ms]--- n2 -------+
```

### 4. **Policy-Based Routing Logic**
- Classification by UDP destination port (VIDEO_PORT vs DATA_PORT)
- Dynamic path selection based on controller decisions
- Simulated link degradation at t=30s to trigger failover

## Key NS-3 Features Used

1. **Custom Applications**: Creating traffic generators with specific patterns
2. **FlowMonitor**: Real-time metrics collection (latency, throughput, packet loss)
3. **Dynamic Channel Modification**: Simulating link degradation
4. **Event Scheduling**: Periodic controller updates using `Simulator::Schedule()`
5. **Multiple Paths**: Using intermediate nodes for redundant connectivity

## Troubleshooting

### Build Errors

If you get compilation errors:

```bash
# Make sure you're using C++17 or later
./ns3 configure --enable-examples CXXFLAGS="-std=c++17"
./ns3 build
```

### Missing Modules

If modules are missing:

```bash
# Ensure all required modules are enabled
./ns3 configure --enable-modules=core,network,internet,point-to-point,applications,flow-monitor
./ns3 build
```

### No Output

If you don't see log output:

```bash
# Run with environment variable
NS_LOG=PolicyBasedRouting=info ./ns3 run scratch/policy-based-routing
```

## Customization Options

### Change Policy Threshold

In the code, modify:
```cpp
m_latencyThreshold = 30.0; // Change to desired value in ms
```

### Change Traffic Patterns

Modify in main():
```cpp
// Video: smaller packets, higher frequency
videoApp->Setup(InetSocketAddress(cloudAddr, videoPort), 
                150,              // packet size
                MilliSeconds(10), // interval
                maxPackets);

// Data: larger packets, lower frequency  
dataApp->Setup(InetSocketAddress(cloudAddr, dataPort), 
               1500,               // packet size
               MilliSeconds(200),  // interval
               maxPackets/10);
```

### Change Link Characteristics

```cpp
p2pPrimary.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
p2pPrimary.SetChannelAttribute("Delay", StringValue("20ms"));
```

## Advanced: Visualizing with NetAnim

To visualize the simulation:

```cpp
// Add at the end of main(), before Simulator::Run()
#include "ns3/netanim-module.h"

AnimationInterface anim("policy-based-routing.xml");
anim.SetConstantPosition(studioNode, 10, 10);
anim.SetConstantPosition(cloudNode, 100, 10);
anim.SetConstantPosition(intermediateNode, 55, 50);
```

Then run NetAnim:
```bash
./NetAnim
# Load policy-based-routing.xml
```

## Exercise Questions Answered

### Q1: Traffic Classification Logic
✅ Implemented in `VideoTrafficApp` and `DataTrafficApp` classes
- Different packet sizes (200 vs 1400 bytes)
- Different intervals (20ms vs 100ms)
- Different port numbers (5004 vs 5005)

### Q2: Implementing PBR in NS-3
✅ Implemented through:
- Port-based classification in controller
- Global variable `g_videoFlowInterface` for routing decisions
- Dynamic updates via `SdWanController::ApplyPolicyRules()`

### Q3: Path Characterization
✅ Implemented using:
- `FlowMonitor` for latency and throughput measurement
- `FetchLinkMetrics()` method extracts per-flow statistics
- Metrics stored in `g_linkLatency` map accessible to PBR function

### Q4: Dynamic Policy Engine
✅ Fully implemented in `SdWanController` class:
- Periodic updates every 1 second
- Fetches metrics from FlowMonitor
- Applies policy rules (30ms latency threshold)
- Updates routing decisions dynamically
- Handles link degradation and recovery# toko-ns3-simulation
