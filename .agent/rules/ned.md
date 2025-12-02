---
trigger: glob
glob: **/*.ned
description: Rules and best practices for editing and creating NED files in the INET Framework
---
# NED File Rules for INET Framework

## 1. File Structure and Header

Every NED file must follow this structure:

```ned
//
// Copyright (C) <year> <copyright holder>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package <fully.qualified.package.name>;

import <imports>;

//
// Documentation comment for the module
//
<module definition>
```

- **License header**: Use SPDX-License-Identifier format (LGPL-3.0-or-later for INET)
- **Package declaration**: Must match directory structure under `src/inet/`
- **Blank line** between license and package, and between package and imports
- **Imports**: List all required imports, one per line

## 2. Module Types

### Simple Modules
Implemented in C++ with a corresponding `.cc` file:
```ned
simple ModuleName extends SimpleModule like IInterface
{
    parameters:
        @class(ClassName);  // Required: links to C++ implementation
        // parameters...
    gates:
        // gates...
}
```

### Compound Modules
Composed of submodules:
```ned
module ModuleName extends BaseModule like IInterface
{
    parameters:
        // parameters...
    submodules:
        // submodules...
    connections:
        // connections...
}
```

### Networks
Top-level simulation networks:
```ned
network NetworkName extends NetworkBase
{
    parameters:
        @display("bgb=800,600");  // Background size
    submodules:
        // nodes and infrastructure...
    connections:
        // links between nodes...
}
```

### Module Interfaces
Define contracts for interchangeable modules:
```ned
moduleinterface IModuleName
{
    parameters:
        @display("i=block/icon");
    gates:
        input in;
        output out;
}
```

### Channels
Define link characteristics:
```ned
channel ChannelName extends DatarateChannel
{
    parameters:
        double length @unit(m) = default(10m);
        delay = default(replaceUnit(length / 2e8, "s"));
        datarate = default(100Mbps);
}
```

## 3. Base Classes to Extend

### For Simple Modules
- **`inet.common.SimpleModule`**: Base for all INET simple modules
- **`inet.common.Module`**: Base for all INET compound modules

### For Networks
- **`inet.networks.base.NetworkBase`**: Basic network with visualizer and configurator
- **`inet.networks.base.WirelessNetworkBase`**: Adds radioMedium and physicalEnvironment
- **`inet.networks.base.WiredNetworkBase`**: Adds MAC forwarding table configurator

### For Nodes
- **`inet.node.base.NodeBase`**: Basic node with mobility, clock, energy
- **`inet.node.base.ApplicationLayerNodeBase`**: Full network stack with apps
- **`inet.node.inet.StandardHost`**: Complete IPv4/IPv6 host

### For Network Interfaces
- **`inet.networklayer.common.NetworkInterface`**: Base for all interfaces

## 4. Common Interfaces (like keyword)

Use these interfaces for pluggable module types:

| Interface | Purpose | Location |
|-----------|---------|----------|
| `IApp` | Applications | `inet.applications.contract` |
| `INetworkInterface` | Network interfaces | `inet.linklayer.contract` |
| `INetworkLayer` | Network layer protocols | `inet.networklayer.contract` |
| `IRadio` | Radio modules | `inet.physicallayer.wireless.common.contract.packetlevel` |
| `IRadioMedium` | Wireless medium | `inet.physicallayer.wireless.common.contract.packetlevel` |
| `IMobility` | Mobility models | `inet.mobility.contract` |
| `IClock` | Clock modules | `inet.clock.contract` |
| `IPacketQueue` | Queuing modules | `inet.queueing.contract` |

## 5. Parameters

### Parameter Types and Syntax
```ned
parameters:
    // Basic types
    int count = default(10);
    double rate = default(1.5);
    bool enabled = default(true);
    string name = default("default");
    
    // With units (ALWAYS use @unit for physical quantities)
    double delay @unit(s) = default(1ms);
    double datarate @unit(bps) = default(100Mbps);
    int packetLength @unit(B) = default(1000B);
    double distance @unit(m) = default(100m);
    
    // Volatile (re-evaluated each access - for random values)
    volatile double sendInterval @unit(s) = default(exponential(1s));
    volatile int messageLength @unit(B) = default(intuniform(100B, 1000B));
    
    // Mutable (can be changed at runtime)
    string address @mutable = default("auto");
    
    // Enum constraint
    string mode @enum("a","b","g") = default("g");
    
    // Object type (for JSON arrays/objects)
    object durations @unit(s) = default([]);
    object mapping = default({});
    
    // Module path references
    string interfaceTableModule;
    string routingTableModule = default("^.ipv4.routingTable");
```

### Parameter Patterns (for setting submodule parameters)
```ned
parameters:
    // Set parameter on all matching submodules
    *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    **.clockModule = default(exists(clock) ? absPath(".clock") : "");
    
    // Conditional defaults
    mac.typename = default(duplexMode ? "EthernetMac" : "EthernetCsmaMac");
```

## 6. Gates

### Gate Syntax
```ned
gates:
    // Simple gates
    input in;
    output out;
    inout io;
    
    // Gate vectors
    input in[];
    output out[];
    inout ethg[];
    
    // With labels (for connection type checking)
    input upperLayerIn @labels(INetworkHeader/down);
    output upperLayerOut @labels(INetworkHeader/up);
    input radioIn @labels(IWirelessSignal);
    
    // Loose gates (optional connections)
    input cutthroughIn @loose;
    output cutthroughOut @loose;
```

## 7. Submodules

### Basic Submodule Declaration
```ned
submodules:
    // Fixed type
    queue: PacketQueue {
        parameters:
            packetCapacity = 1000;
            @display("p=100,100");
    }
    
    // Parametric type with interface
    radio: <default("Ieee80211Radio")> like IRadio {
        @display("p=200,100");
    }
    
    // Conditional submodule
    agent: <default("Ieee80211AgentSta")> like IIeee80211Agent if typename != "" {
        @display("p=300,100");
    }
    
    // Submodule vector
    app[numApps]: <> like IApp {
        @display("p=100,100,row,150");
    }
    
    // Conditional on parameter
    visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if typename != "" {
        @display("p=100,200;is=s");
    }
```

### Checking Submodule Existence
```ned
connections:
    mgmt.agentOut --> agent.mgmtIn if exists(agent);
```

## 8. Connections

### Connection Syntax
```ned
connections:
    // Simple connections
    app.out --> queue.in;
    queue.out --> mac.upperLayerIn;
    
    // Bidirectional
    mac.phys <--> phys;
    
    // With channel
    client.ethg++ <--> Eth100M <--> switch.ethg++;
    
    // With inline channel parameters
    node1.out --> { delay = 10ms; datarate = 1Gbps; } --> node2.in;
    
    // With display string
    radioIn --> { @display("m=s"); } --> radio.radioIn;
    
    // Gate vector connections in loops
    for i=0..numApps-1 {
        app[i].socketOut --> at.in++;
        app[i].socketIn <-- at.out++;
    }
    
    // Conditional connections
    at.out++ --> tcp.appIn if hasTcp;
```

### Allow Unconnected Gates
```ned
connections allowunconnected:
    // Some gates may remain unconnected
```

## 9. Properties (Annotations)

### Module Properties
```ned
@class(ClassName);           // C++ class name
@namespace(inet::tcp);       // C++ namespace
@display("i=block/app");     // Icon and display
@networkNode;                // Marks as network node
@networkInterface;           // Marks as network interface
@lifecycleSupport;           // Supports start/stop operations
@labels(node,wireless-node); // Classification labels
```

### Signal and Statistic Properties
```ned
// Signal declaration
@signal[packetSent](type=inet::Packet);
@signal[packetReceived](type=inet::Packet);
@signal[throughputChanged](type=double);

// Statistics
@statistic[packetSent](title="packets sent"; source=packetSent; record=count,vector; interpolationmode=none);
@statistic[throughput](title="throughput"; unit=bps; source=throughput(packetReceived); record=vector);
@statistic[queueLength](title="queue length"; source=count(packetPushed)-count(packetPulled); record=max,timeavg,vector; interpolationmode=sample-hold);

// Default statistic for IDE
@defaultStatistic(queueLength:vector);
```

### Figure Properties (for network visualization)
```ned
@figure[applicationLayer](type=rectangle; pos=250,5; size=1000,130; fillColor=#ffff00; fillOpacity=0.1);
@figure[title](type=text; pos=500,10; text="My Network");
@statistic[counter](source=count(signal); record=figure; targetFigure=myCounter);
@figure[myCounter](type=counter; pos=50,50; label="Packets");
```

## 10. Display Strings

### Common Display String Tags
```ned
@display("i=device/pc2");           // Icon
@display("p=100,200");              // Position
@display("p=100,200,row,150");      // Position with layout (row/column/matrix)
@display("is=s");                   // Icon size (vs=very small, s=small, n=normal, l=large, vl=very large)
@display("b=100,50,rect,red");      // Box shape
@display("bgb=800,600");            // Background bounds (for networks)
@display("bgi=background,s");       // Background image
@display("m=s");                    // Connection anchor (n/s/e/w)
```

## 11. Documentation Comments

Use NED documentation format:
```ned
//
// Brief description of the module.
//
// Detailed description with multiple paragraphs.
// Can include <b>HTML formatting</b>.
//
// <b>Parameters:</b>
// - `paramName`: Description of parameter
//
// @see ~OtherModule, ~AnotherModule
//
```

## 12. Best Practices

### DO:
- **Extend base classes**: Use `SimpleModule`, `Module`, `NetworkBase`, etc.
- **Use interfaces**: Declare modules with `like IInterface` for flexibility
- **Provide defaults**: Always provide sensible `default()` values
- **Use units**: Always use `@unit()` for physical quantities
- **Document**: Add documentation comments for all public modules
- **Use conditional submodules**: `if typename != ""` pattern for optional components
- **Reference modules by path**: Use `absPath()` for module references

### DON'T:
- **Don't hardcode paths**: Use parameter references like `^.interfaceTable`
- **Don't skip units**: Never use bare numbers for time, length, rate, etc.
- **Don't duplicate**: Extend existing modules instead of copying
- **Don't use non-existent parameters**: Always verify parameter exists in module definition

### Path References
```ned
// Relative paths
string tableModule = default("^.interfaceTable");     // Parent's interfaceTable
string clockModule = default("^.^.clock");            // Grandparent's clock

// Absolute path helper
*.module = default(absPath(".submodule"));            // Full path to submodule
```

## 13. Common Patterns in INET

### Application Module Pattern
```ned
simple MyApp extends SimpleModule like IApp
{
    parameters:
        @class(MyApp);
        string interfaceTableModule;
        int localPort = default(-1);
        double startTime @unit(s) = default(1s);
        double stopTime @unit(s) = default(-1s);
        volatile double sendInterval @unit(s);
        @display("i=block/app");
        @lifecycleSupport;
        @signal[packetSent](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,vector);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
```

### Network Interface Pattern
```ned
module MyInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IWirelessSignal);
    submodules:
        mac: <default("MyMac")> like IMacProtocol { }
        radio: <default("MyRadio")> like IRadio { }
    connections:
        upperLayerIn --> mac.upperLayerIn;
        mac.upperLayerOut --> upperLayerOut;
        mac.lowerLayerOut --> radio.upperLayerIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
        radioIn --> radio.radioIn;
}
```

### Simple Network Pattern
```ned
network MyNetwork extends WirelessNetworkBase
{
    parameters:
        int numHosts = default(10);
        @display("bgb=600,400");
    submodules:
        host[numHosts]: StandardHost {
            @display("p=100,200,row,100");
        }
}
```

### Wired Network with Connections
```ned
network MyWiredNetwork extends WiredNetworkBase
{
    submodules:
        client: StandardHost { @display("p=100,100"); }
        server: StandardHost { @display("p=300,100"); }
        switch: EthernetSwitch { @display("p=200,100"); }
    connections:
        client.ethg++ <--> Eth100M <--> switch.ethg++;
        server.ethg++ <--> Eth100M <--> switch.ethg++;
}
```
