# NS-3 WAN Security Exercise - Complete Implementation Guide

## Overview
This guide provides step-by-step instructions for implementing Exercise 3: WAN Security Integration and Attack Simulation in NS-3.

## Prerequisites
- NS-3 (version 3.35 or later recommended)
- C++ compiler (g++ or clang)
- Basic understanding of NS-3 simulation framework

## Installation Steps

### 1. Setup NS-3 Environment

```bash
# If you haven't installed NS-3 yet:
cd ~
wget https://www.nsnam.org/releases/ns-allinone-3.41.tar.bz2
tar xjf ns-allinone-3.41.tar.bz2
cd ns-allinone-3.41
./build.py --enable-examples --enable-tests

# Navigate to NS-3 directory
cd ns-3.41
```

### 2. Create the Simulation File

```bash
# Create the simulation file in the scratch directory
nano scratch/wan-security-exercise.cc

# Copy the complete code from the artifact into this file
# Save and exit (Ctrl+X, Y, Enter)
```

### 3. Build the Simulation

```bash
# Configure and build
./waf configure --enable-examples
./waf build

# Or just rebuild the scratch file
./waf --run wan-security-exercise
```

## Running Different Scenarios

### Scenario 1: Baseline (No Security, No Attacks)
Test basic network performance without any security or attacks:

```bash
./waf --run "wan-security-exercise --ipsec=0 --ddos=0 --eavesdrop=0 --ratelimit=0 --fairqueue=0"
```

**Expected Results:**
- High throughput (~5 Mbps)
- Low latency (20-25ms)
- 100% packet delivery ratio
- No security protection

---

### Scenario 2: IPsec Only (Question 1)
Test IPsec implementation and its performance impact:

```bash
./waf --run "wan-security-exercise --ipsec=1 --ddos=0 --eavesdrop=0 --ratelimit=0 --fairqueue=0"
```

**Analysis Points:**
- Compare throughput with Scenario 1 (expect 10-15% reduction)
- Measure latency increase (expect 2-5ms additional delay)
- Note IPsec overhead in packet sizes

**To Demonstrate:**
- Packet size increases by ~39 bytes (IPsec header)
- Processing delay simulated at 3ms per packet
- Shows encryption overhead

---

### Scenario 3: Eavesdropping Attack (Question 2)
Simulate eavesdropping on WAN link:

```bash
# Without IPsec (vulnerable)
./waf --run "wan-security-exercise --ipsec=0 --ddos=0 --eavesdrop=1 --ratelimit=0 --fairqueue=0"

# With IPsec (protected)
./waf --run "wan-security-exercise --ipsec=1 --ddos=0 --eavesdrop=1 --ratelimit=0 --fairqueue=0"
```

**Analysis Points:**
- Check "Packets sniffed by attacker" in output
- Without IPsec: Attacker can read packet contents
- With IPsec: Packets are encrypted (data unreadable)

**In the Logs:**
Look for lines showing:
```
ATTACKER: Sniffed packet of size XXX bytes at X.XXXs
```

---

### Scenario 4: DDoS Attack Without Defense (Question 3)
Simulate distributed denial of service attack:

```bash
./waf --run "wan-security-exercise --ipsec=0 --ddos=1 --eavesdrop=0 --ratelimit=0 --fairqueue=0 --attackers=20"
```

**Expected Results:**
- Legitimate packet delivery ratio drops significantly (< 50%)
- High packet loss
- Increased latency
- Server becomes overwhelmed

**Vary Attack Intensity:**
```bash
# Light attack (10 attackers)
./waf --run "wan-security-exercise --ddos=1 --attackers=10"

# Heavy attack (50 attackers)
./waf --run "wan-security-exercise --ddos=1 --attackers=50"
```

---

### Scenario 5: DDoS with Rate Limiting (Question 4a)
Test rate limiting defense mechanism:

```bash
./waf --run "wan-security-exercise --ipsec=0 --ddos=1 --eavesdrop=0 --ratelimit=1 --fairqueue=0 --attackers=20"
```

**Analysis Points:**
- Compare delivery ratio with Scenario 4
- Note throughput cap at configured limit (40 Mbps)
- Some legitimate traffic preserved

**Limitations to Discuss:**
- Rate limiting affects all traffic equally
- Legitimate bursts may be dropped
- Doesn't distinguish between good/bad traffic

---

### Scenario 6: DDoS with ACL Blocking (Question 4b)
Test access control list defense:

```bash
./waf --run "wan-security-exercise --ipsec=0 --ddos=1 --eavesdrop=0 --ratelimit=0 --acl=1 --fairqueue=0 --attackers=20"
```

**Analysis Points:**
- First 5 attackers are blocked (check logs)
- Remaining attackers still cause impact
- Shows ACL effectiveness and limitations

**Limitations to Discuss:**
- Requires knowing attacker IPs in advance
- Attackers can use spoofed IPs
- IP rotation bypasses ACLs

---

### Scenario 7: DDoS with Fair Queuing (Question 4c)
Test fair queuing defense:

```bash
./waf --run "wan-security-exercise --ipsec=0 --ddos=1 --eavesdrop=0 --ratelimit=0 --fairqueue=1 --attackers=20"
```

**Analysis Points:**
- FqCoDel ensures fair bandwidth allocation
- Legitimate traffic gets proportional share
- Better delivery ratio than no defense

**Advantages:**
- Automatically fair without configuration
- Adapts to varying traffic patterns
- Combines well with rate limiting

---

### Scenario 8: Full Protection Stack (Question 5)
Test combined defenses for optimal security/performance balance:

```bash
./waf --run "wan-security-exercise --ipsec=1 --ddos=1 --eavesdrop=1 --ratelimit=1 --fairqueue=1 --attackers=20 --simtime=60"
```

**Analysis Points:**
- Best delivery ratio with multiple defenses
- Security benefits from IPsec
- Performance cost of layered security
- Balanced approach

---

## Understanding the Output

### Console Output Sections

1. **Network Setup Information**
```
Creating nodes...
Installing Rate Limiting: 40Mbps
Installing Fair Queuing
```

2. **Attack Statistics**
```
--- Eavesdropping Attack Results ---
Packets sniffed by attacker: 450
IPsec protection: ENABLED (data encrypted)
```

3. **Performance Metrics**
```
--- Legitimate Traffic Statistics ---
Packets sent: 1000
Packets received: 853
Delivery ratio: 85.3%
```

4. **Flow Analysis**
```
LEGITIMATE Flow 1
  10.1.1.1 -> 10.1.2.2
  Throughput: 4.2 Mbps
  Avg Delay: 24.5 ms
```

5. **Trade-off Analysis**
```
=== SECURITY vs PERFORMANCE TRADE-OFF ===
Total Throughput: 15.4 Mbps
Legitimate Traffic Throughput: 4.2 Mbps
Legitimate Delivery Ratio: 85.3%
```

### XML Output Analysis

The simulation generates `wan-security-results.xml` with detailed flow statistics:

```bash
# View the XML file
cat wan-security-results.xml

# Or use a tool to parse it
python3 << EOF
import xml.etree.ElementTree as ET
tree = ET.parse('wan-security-results.xml')
root = tree.getroot()
for flow in root.findall('.//Flow'):
    print(f"Flow {flow.get('flowId')}: {flow.get('timeFirstTxPacket')} -> {flow.get('timeLastRxPacket')}")
EOF
```

## Answering the Exercise Questions

### Question 1: IPsec VPN Implementation Design

**Run the comparison:**
```bash
# Without IPsec
./waf --run "wan-security-exercise --ipsec=0 --ddos=0" > no-ipsec.txt

# With IPsec
./waf --run "wan-security-exercise --ipsec=1 --ddos=0" > with-ipsec.txt

# Compare results
diff no-ipsec.txt with-ipsec.txt
```

**In Your Report:**
- NS-3 modules used: Custom IpsecSimulator class, point-to-point module
- Performance overhead: Extract from output (throughput reduction, latency increase)
- Packet size overhead: 39 bytes added per packet
- Processing delay: 3ms per packet for encryption/decryption

---

### Question 2: Eavesdropping Attack Simulation

**Run the demonstration:**
```bash
./waf --run "wan-security-exercise --eavesdrop=1 --ipsec=0" > eavesdrop-vulnerable.txt
./waf --run "wan-security-exercise --eavesdrop=1 --ipsec=1" > eavesdrop-protected.txt
```

**In Your Report:**
- Tracing mechanism: SetPromiscuousReceiveCallback on attacker's device
- Packet capture: PacketSnifferCallback function logs all packets
- Effectiveness: Compare "Packets sniffed" with/without IPsec
- Include code snippets from the simulation showing callback setup

---

### Question 3: DDoS Attack Simulation

**Run progressive attack scenarios:**
```bash
# Baseline
./waf --run "wan-security-exercise --ddos=0" > baseline.txt

# Light attack
./waf --run "wan-security-exercise --ddos=1 --attackers=10" > light-attack.txt

# Medium attack
./waf --run "wan-security-exercise --ddos=1 --attackers=20" > medium-attack.txt

# Heavy attack
./waf --run "wan-security-exercise --ddos=1 --attackers=50" > heavy-attack.txt
```

**In Your Report:**
- Multiple malicious nodes: DDoSAttacker class creates UDP/SYN flood
- Traffic patterns: 5 Mbps per attacker, staggered start
- Impact measurement: Delivery ratio drops from ~100% to < 50%
- Create a graph showing delivery ratio vs. number of attackers

---

### Question 4: Defense Mechanisms

**Test each defense individually:**

```bash
# Rate limiting only
./waf --run "wan-security-exercise --ddos=1 --ratelimit=1 --fairqueue=0 --acl=0 --attackers=20" > rate-limit.txt

# ACL only
./waf --run "wan-security-exercise --ddos=1 --ratelimit=0 --fairqueue=0 --acl=1 --attackers=20" > acl.txt

# Fair queuing only
./waf --run "wan-security-exercise --ddos=1 --ratelimit=0 --fairqueue=1 --acl=0 --attackers=20" > fair-queue.txt
```

**In Your Report:**
- Implementation: Show code for InstallRateLimiting, InstallAclBlocking, InstallFairQueuing
- Effectiveness: Compare delivery ratios
- Limitations: Discuss from simulation observations
- NS-3 specifics: TrafficControlHelper, QueueDisc usage

---

### Question 5: Security vs. Performance Trade-off

**Run comprehensive comparison:**
```bash
# No security, no attack
./waf --run "wan-security-exercise --ipsec=0 --ddos=0 --ratelimit=0 --fairqueue=0" > baseline.txt

# Security only
./waf --run "wan-security-exercise --ipsec=1 --ddos=0 --ratelimit=1 --fairqueue=1" > security-only.txt

# Attack without defense
./waf --run "wan-security-exercise --ipsec=0 --ddos=1 --ratelimit=0 --fairqueue=0 --attackers=20" > attack-no-defense.txt

# Full protection
./waf --run "wan-security-exercise --ipsec=1 --ddos=1 --ratelimit=1 --fairqueue=1 --attackers=20" > full-protection.txt
```

**Create Comparison Table:**

| Scenario | Throughput | Latency | Delivery Ratio | Security Level |
|----------|-----------|---------|----------------|----------------|
| Baseline | Extract from baseline.txt | | | Low |
| Security Only | Extract from security-only.txt | | | High |
| Attack, No Defense | Extract from attack-no-defense.txt | | | Low |
| Full Protection | Extract from full-protection.txt | | | High |

---

## Creating Graphs and Visualizations

### Python Script for Visualization

```python
import matplotlib.pyplot as plt
import numpy as np

# Example: Parse simulation results and create graphs

scenarios = ['Baseline', 'IPsec Only', 'DDoS Only', 'Full Protection']
throughput = [5.0, 4.5, 1.2, 3.8]  # Replace with actual values
delivery_ratio = [100, 98, 25, 85]  # Replace with actual values
latency = [22, 27, 120, 35]  # Replace with actual values

# Throughput comparison
plt.figure(figsize=(10, 6))
plt.subplot(1, 3, 1)
plt.bar(scenarios, throughput, color=['green', 'blue', 'red', 'orange'])
plt.ylabel('Throughput (Mbps)')
plt.title('Throughput Comparison')
plt.xticks(rotation=45)

# Delivery ratio
plt.subplot(1, 3, 2)
plt.bar(scenarios, delivery_ratio, color=['green', 'blue', 'red', 'orange'])
plt.ylabel('Delivery Ratio (%)')
plt.title('Packet Delivery')
plt.xticks(rotation=45)

# Latency
plt.subplot(1, 3, 3)
plt.bar(scenarios, latency, color=['green', 'blue', 'red', 'orange'])
plt.ylabel('Latency (ms)')
plt.title('Average Latency')
plt.xticks(rotation=45)

plt.tight_layout()
plt.savefig('wan-security-analysis.png', dpi=300)
plt.show()
```

## Troubleshooting

### Common Issues

1. **Build Errors**
```bash
# Clean and rebuild
./waf clean
./waf configure --enable-examples
./waf build
```

2. **Module Not Found**
```bash
# Check NS-3 version
./waf --version

# Ensure all modules are enabled
./waf configure --enable-modules=core,network,internet,point-to-point,applications,flow-monitor,traffic-control
```

3. **Low Throughput**
- Check link bandwidth settings in code
- Verify simulation time is sufficient
- Ensure routing tables are populated

4. **Segmentation Fault**
- Check pointer validity in callbacks
- Verify node/device indices
- Use NS_LOG to debug

### Debugging Tips

```bash
# Enable detailed logging
export NS_LOG=WanSecurityExercise=level_all

# Or enable specific components
export NS_LOG=WanSecurityExercise=level_info:UdpEchoClientApplication=level_all

# Run with logging
./waf --run wan-security-exercise
```

## Advanced Modifications

### 1. Add More Attack Types

Implement HTTP flood:
```cpp
class HttpFloodAttacker : public Application {
    // Generate HTTP GET requests at high rate
    // Simulate legitimate-looking traffic
};
```

### 2. Implement Dynamic ACL

Add traffic analysis:
```cpp
void AnalyzeTrafficAndUpdateAcl(Ptr<Node> router) {
    // Monitor packet rates per source
    // Automatically blacklist high-rate sources
}
```

### 3. Add Intrusion Detection

```cpp
class SimpleIDS {
    // Pattern matching
    // Anomaly detection
    // Alert generation
};
```

## Report Structure Recommendation

1. **Introduction**
   - Objective of each question
   - NS-3 simulation approach

2. **Methodology**
   - Network topology description
   - NS-3 implementation details
   - Simulation parameters

3. **Results** (for each question)
   - Console output excerpts
   - Tables with metrics
   - Graphs and visualizations

4. **Analysis**
   - Performance trade-offs
   - Defense effectiveness
   - Limitations observed

5. **Conclusion**
   - Recommended security posture
   - Lessons learned from simulation

## Additional Resources

- NS-3 Documentation: https://www.nsnam.org/documentation/
- NS-3 Tutorial: https://www.nsnam.org/docs/tutorial/html/
- Flow Monitor: https://www.nsnam.org/docs/models/html/flow-monitor.html
- Traffic Control: https://www.nsnam.org/docs/models/html/traffic-control.html

## Summary

This implementation provides a complete, working NS-3 simulation for Exercise 3. Run the different scenarios, collect the metrics, and analyze the trade-offs between security and performance to answer all five questions comprehensively.# toko-ns3-simulation
