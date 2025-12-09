# QoS Mixed Traffic Simulation - Setup & Run Guide

## Installation Steps

### 1. Save the Code

Save the C++ code as a file in your NS-3 scratch directory:

```bash
cd ~/ns-3-dev/scratch/
nano qos-mixed-traffic.cc
# Paste the code and save (Ctrl+X, Y, Enter)
```

### 2. Build the Simulation

```bash
cd ~/ns-3-dev/
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

Or if using older NS-3 versions (pre-3.36):
```bash
./waf configure --enable-examples --enable-tests
./waf build
```

## Running the Simulation

### Basic Run (with QoS enabled)

```bash
./ns3 run qos-mixed-traffic
```

### Run WITHOUT QoS (for comparison)

```bash
./ns3 run "qos-mixed-traffic --enableQoS=false"
```

### Custom Parameters

```bash
# More VoIP flows
./ns3 run "qos-mixed-traffic --voipFlows=10"

# More FTP flows (heavier congestion)
./ns3 run "qos-mixed-traffic --ftpFlows=5"

# Longer simulation
./ns3 run "qos-mixed-traffic --simTime=60"

# Combined parameters
./ns3 run "qos-mixed-traffic --voipFlows=8 --ftpFlows=4 --simTime=45"
```

## Expected Output

The simulation will display:

1. **Configuration Summary**: Shows QoS status, flow counts, and network parameters
2. **Simulation Timeline**: Key events during the simulation
3. **Detailed Flow Statistics**: Per-flow metrics (delay, jitter, loss, throughput)
4. **Summary Statistics**: Aggregated metrics for VoIP and FTP traffic classes
5. **XML Output File**: `qos-enabled.xml` or `qos-disabled.xml`

## Analyzing Results

### Key Metrics to Compare

**With QoS Enabled:**
- VoIP delay: Should be < 150ms even under congestion
- VoIP jitter: Should be < 30ms
- VoIP packet loss: Should be < 1%
- FTP gets remaining bandwidth

**Without QoS:**
- VoIP delay: Will increase to 200-400ms during congestion
- VoIP jitter: Will be high (50-100ms+)
- Both traffic types suffer equally

### Visualization (Optional)

You can visualize the FlowMonitor XML output:

```bash
# Install required tools
sudo apt-get install python3-matplotlib

# Use NS-3's FlowMonitor helper scripts (if available)
cd ~/ns-3-dev/
python3 utils/flowmon-parse-results.py qos-enabled.xml
```

## Comparison Experiment

Run both scenarios and compare:

```bash
# Scenario 1: WITH QoS
./ns3 run "qos-mixed-traffic --enableQoS=true" > results-qos-enabled.txt

# Scenario 2: WITHOUT QoS  
./ns3 run "qos-mixed-traffic --enableQoS=false" > results-qos-disabled.txt

# Compare the outputs
diff results-qos-enabled.txt results-qos-disabled.txt
```

## Expected Results Summary Table

| Metric              | Class 1 (VoIP) QoS ON | Class 2 (FTP) QoS ON | Class 1 (VoIP) QoS OFF | Class 2 (FTP) QoS OFF |
|---------------------|----------------------|---------------------|----------------------|---------------------|
| Avg Latency         | 30-50ms              | 200-400ms           | 200-400ms            | 200-400ms           |
| Avg Jitter          | 5-15ms               | 50-100ms            | 50-100ms             | 50-100ms            |
| Packet Loss Rate    | < 1%                 | 1-3%                | 1-3%                 | 1-3%                |
| Throughput          | ~64 kbps per flow    | Shares remaining BW | ~64 kbps per flow    | Shares available BW |

## Troubleshooting

### Build Errors

**Error: "module not found"**
```bash
# Ensure all modules are enabled
./ns3 configure --enable-examples
```

**Error: "command not found: ./ns3"**
```bash
# Use waf for older NS-3 versions
./waf --run qos-mixed-traffic
```

### Runtime Issues

**No output or crash:**
- Check NS-3 version compatibility (tested on NS-3.35+)
- Verify all modules are built: `./ns3 build`

**Unrealistic results:**
- Increase simulation time: `--simTime=60`
- Adjust flow counts to avoid overload
- Check bottleneck capacity matches traffic load

## Advanced Usage

### Enable Packet Capture (PCAP)

Add this before `Simulator::Run()` in the code:

```cpp
bottleneckLink.EnablePcapAll("qos-mixed");
```

Then analyze with Wireshark:
```bash
wireshark qos-mixed-*.pcap
```

### Enable Animation (NetAnim)

Add to the code:
```cpp
#include "ns3/netanim-module.h"

// Before Simulator::Run()
AnimationInterface anim("qos-animation.xml");
```

Then view:
```bash
./NetAnim qos-animation.xml
```

## Questions Answered by This Simulation

1. ✅ **Traffic Differentiation**: Uses DSCP tagging (EF for VoIP, BE for FTP)
2. ✅ **Queue Management**: Implements PrioQueueDisc with 2 priority bands
3. ✅ **Performance Measurement**: FlowMonitor tracks latency, jitter, loss, throughput
4. ✅ **Congestion Testing**: Staggered FTP flows create progressive congestion from 2s-20s

## Next Steps

1. Run both QoS enabled and disabled scenarios
2. Collect and compare the statistics
3. Create a comparison table for your report
4. Optional: Generate graphs from the XML output
5. Document your observations about how QoS protects VoIP traffic# toko-ns3-simulation
