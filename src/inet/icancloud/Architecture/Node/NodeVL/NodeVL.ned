
// Module that implements a Node with its virtualization layer (hypervisor).
//
// This class models a node with an hypervisor. It is able to execute applications at node and virtual machines 
//	requested by tenants.
//
// @author Gabriel González Castañé
// @date 2014-12-12

package inet.icancloud.Architecture.Node.NodeVL;

import inet.common.lifecycle.NodeStatus;
import inet.mobility.contract.IMobility;
import inet.node.inet.INetworkNode;
import inet.power.contract.IEnergyGenerator;
import inet.power.contract.IEnergyManagement;
import inet.power.contract.IEnergyStorage;
import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IExternalInterface;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.contract.ITunnelInterface;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.contract.IVlanInterface;
import inet.networklayer.common.InterfaceTable;
import inet.transportlayer.contract.ITcp;
import inet.common.MessageDispatcher;
import inet.networklayer.contract.INetworkLayer;

import inet.icancloud.Architecture.Node.INode;
import inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.ICPU_Module;
import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.MainMemories.IMainMemory;
import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageSystems.StorageSystem;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.OperatingSystemModules.OperatingSystemModule;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeter;
import inet.icancloud.EnergySystem.PSU.IPSU;
import inet.icancloud.Virtualization.Hypervisor.IHypervisor;
import inet.linklayer.loopback.LoopbackInterface;

module NodeVL like INode

{
    parameters:

        @class("icancloud::NodeVL");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        @display("i=old/telnet");
        


        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");

        bool hasStatus = default(false);
        string osgModel = default(""); // 3D model for OSG visualization, no 3D model by default
        string osgModelColor = default(""); // tint color, no colorization by default
        string canvasImage = default(""); // image for canvas visualization, no image by default
        string canvasImageColor = default(""); // tint color, no colorization by default
        mobility.subjectModule = default("^");

        // Default parameters
        int numNetworkInterfaces = default(1);													// As default, 1 network interface
        //string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));   	// Type of TCP protocol     
        string ip = default("");															// Node's IP
        string routingFile = default("");														// Routing File
        int namid = default(-1);																// Nam ID
        bool storageNode = default(false);														// is this a atorage Node? If no, then its a compute node
        int storage_local_port = default(2049);													// Port for listening the incoming connections
        string hostName = default("");															// Node's hostname       

        bool activeEnergyMeter = default(true);
        
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        


        int numPcapRecorders = default(0);
        int numLoInterfaces = default(1);
        int numWlanInterfaces = default(0);
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces
        int numExtInterfaces = default(0);
        int numPppInterfaces = default(0);  // minimum number of PPP interfaces
        int numTunInterfaces = default(0);
        int numVlanInterfaces = default(0);

        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);

        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;

        *.mobilityModule = default(exists(mobility) ? absPath(".mobility") : "");
        *.energySourceModule = default(exists(energyStorage) ? absPath(".energyStorage") : "");
        *.energyGeneratorModule = default(exists(energyGenerator) ? absPath(".energyGenerator") : "");
        *.routingTableModule = default(absPath(".ipv4.routingTable"));

        *.interfaceTableModule = default(absPath(".interfaceTable"));


        // Main Hardware subsystems        
        string cpuModuleType = default("CPU_Module");				// CPUModule type as CPU type
        string memoryType = default("RAMmemory");					// RAMmemory as memory type  
        string psuModuleType = default("MainPSU");					// PSU module  

         // Main Software configuration
        string hypervisorType = default("HypervisorNoOverhead");			// Main hypervisor		

        // Node parameters
        int numCores;												// Number of CPU cores		
        int memorySize_MB @unit(MiB);										// Total amount of memory of this node (in MB)
        int memoryBlockSize_KB @unit(KiB) = default(128KiB);
        int numStorageSystems = default(1);							// Number of Storage systems. As default, 1 storage system
        int storageSize_GB  @unit(GiB);									// Total amount of space for each storage system
        string initialState;										// state or the node  NODE_STATE_OFF | NODE_STATE_IDLE | NODE_STATE_RUNNING
        int numFS = default(1);													// Number of File Systems

    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[numPppInterfaces] @labels(PppFrame-conn) @allowUnconnected;
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;

         //storage server
        input fromVMStorageServers[];
        output toVMStorageServers[];

        //cpu
        input fromVMCPU[];
        output toVMCPU[];

        //memory
        input fromVMMemoryI[];
        input fromVMMemoryO[];
        output toVMMemoryI[];
        output toVMMemoryO[];

        // Hypervisor net
        input fromVMNet[];
        output toVMNet[];


    submodules:

      status: NodeStatus if hasStatus {
            parameters:
                @display("p=53,301;is=s");
        }
        energyStorage: <default("")> like IEnergyStorage if typename != "" {
            parameters:
                @display("p=53,364;is=s");
        }
        energyManagement: <default("")> like IEnergyManagement if typename != "" {
            parameters:
                @display("p=53,431;is=s");
        }
        energyGenerator: <default("")> like IEnergyGenerator if typename != "" {
            parameters:
                @display("p=53,505;is=s");
        }
        // TODO: move mobility right after status to have it in the same order as in their positions, changes fingerprints 
        mobility: <default("")> like IMobility if typename != "" {
            parameters:
                @display("p=110,497;is=s");
        }



        at: MessageDispatcher {
            parameters:
                @display("p=462,144;b=627,5,,,,1");
        }

        tn: MessageDispatcher {
            parameters:
                @display("p=434,206;b=627,5,,,,1");
        }

        nl: MessageDispatcher {
            parameters:
                @display("p=453,310;b=664,5,,,,1");
        }


        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            parameters:
                @display("p=292,248;q=queue");
        }
        ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6 {
            parameters:
                @display("p=443,240;q=queue");
        }
        generic: <default("")> like INetworkLayer if hasGn {
            parameters:
                @display("p=564,239;q=queue");
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=45,532;is=s");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                @display("p=86,560;is=s");
        }

        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            parameters:
                @display("p=536,523,row,150");
        }
        // TODO: move wlan interfaces after eth interfaces, but it changes IP address assignment and breaks examples/inet/configurator/complex.ini
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            parameters:
                @display("p=355,504,row,150;q=queue");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=208,484,row,150;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=684,529,row,150;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalInterface {
            parameters:
                @display("p=1029,490,row,150;q=txQueue");
        }

        vlan[numVlanInterfaces]: <default("VlanInterface")> like IVlanInterface {
            parameters:
                @display("p=822,523,row,150;q=txQueue");
        }

        tun[numTunInterfaces]: <default("TunInterface")> like ITunnelInterface {
            parameters:
                @display("p=959,560,row,150;q=txQueue");
        }



        energyMeter: EnergyMeter {
            parameters:
                activeEnergyMeter = activeEnergyMeter;
                @display("p=45,459");
        }

        psu: <psuModuleType> like IPSU {
            @display("p=45,400");
        }

        tcp: <default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"))> like ITcp if hasTcp {
            parameters:
                @display("p=443,176;is=s");
        }


        osModule: OperatingSystemModule {
            parameters:
                numFS = numFS;
                numCPUs = numCores;
                numStorageSystems = numStorageSystems;
                @display("p=339,52");
            gates:
                fromApps[];
                toApps[];
                fromCPU[numCores];
                toCPU[numCores];
                fromStorageSystem[numStorageSystems];
                toStorageSystem[numStorageSystems];
                fromInputMemory;
                fromOutputMemory;
                toInputMemory;
                toOutputMemory;
                fromNet_TCP;
                toNet_TCP;
        }

        cpuModule: <cpuModuleType> like ICPU_Module {
            parameters:
                numCPUs = numCores;
                @display("p=230,93");
            gates:
                fromOS[numCores];
                toOS[numCores];

        }
        storageSystem: StorageSystem {
            parameters:
                numStorageSystems = numStorageSystems;
                @display("i=device/blockManager;p=115,128,row");
            gates:
                fromHypervisor[numStorageSystems];
                toDevice[numStorageSystems];

        }
        memory: <memoryType> like IMainMemory {
            parameters:
                blockSize_KB = memoryBlockSize_KB;
                size_MB = memorySize_MB;
                @display("p=65,180");
            gates:
                fromInput;
                toInput;
        }

        hypervisor: <hypervisorType> like IHypervisor {
            parameters:
                numStorageServers = numStorageSystems; 							// Number of Storage Servers	
                numCPUs = numCores; 	 										// Number of CPUs
                memorySize_MB = memorySize_MB;
                blockSize_KB = memoryBlockSize_KB;
                numNetworkInterfaces = numNetworkInterfaces;
                ip = ip;
                storageSize_GB = storageSize_GB;
                storageApp_ModuleIndex = 0;
                @display("p=55,52;i=energy/Hypervisor_40");
            gates:

                //Hypervisor to array of gates connections
                //                Storage server
                fromVMStorageServers[];
                toVMStorageServers[];

                //cpu
                fromVMCPU[];
                toVMCPU[];

                //memory
                fromVMMemoryI[];
                fromVMMemoryO[];
                toVMMemoryI[];
                toVMMemoryO[];

				//Hypervisor to HW connections                
                fromNodeCPU[numCores];
                toNodeCPU[numCores];

                fromNodeStorageServers[numStorageSystems];
                toNodeStorageServers[numStorageSystems];

                fromNodeMemoryI;
                fromNodeMemoryO;
                toNodeMemoryI;
                toNodeMemoryO;

                fromNodeNet;			// Input gate from Network (Ethernet TCP)
                toNodeNet;			// Output gate to Network (Ethernet TCP)

        }

    connections allowunconnected:

        // Connections between CPU and Hypervisor
        for i=0..numCores-1 {
            cpuModule.toOS[i] --> hypervisor.fromNodeCPU[i];
            cpuModule.fromOS[i] <-- hypervisor.toNodeCPU[i];

        }

        // Connections between Storage Servers and hypervisor
        for i=0..numStorageSystems-1 {
            storageSystem.toDevice[i] --> hypervisor.fromNodeStorageServers[i];
            storageSystem.fromHypervisor[i] <-- hypervisor.toNodeStorageServers[i];
        }

        // Connections between Memory and hypervisor
        memory.toInput --> hypervisor.fromNodeMemoryI;
        memory.fromInput <-- hypervisor.toNodeMemoryI;
        memory.fromOutput <-- hypervisor.toNodeMemoryO;
        memory.toOutput --> hypervisor.fromNodeMemoryO;

		// Connections between TCP and hypervisor

        at.out++ --> tcp.appIn;
        at.in++ <-- tcp.appOut;

        hypervisor.toNodeNet --> at.in++;
        hypervisor.fromNodeNet <-- at.out++;

        tcp.ipOut --> tn.in++;
        tcp.ipIn <-- tn.out++;

		// Connections between the BS and the Operative System        
        for i=0..numStorageSystems-1 {
            osModule.fromStorageSystem++ <-- hypervisor.toVMStorageServers++;
            osModule.toStorageSystem++ --> hypervisor.fromVMStorageServers++;
        }
        //Conections of the Memory and Operative System
        osModule.fromInputMemory <-- hypervisor.toVMMemoryI++;
        osModule.toInputMemory --> hypervisor.fromVMMemoryI++;
        osModule.fromOutputMemory <-- hypervisor.toVMMemoryO++;
        osModule.toOutputMemory --> hypervisor.fromVMMemoryO++;

		//Conections of the CPU and Operative System
        osModule.fromCPU[0] <-- hypervisor.toVMCPU++;
        osModule.toCPU[0] --> hypervisor.fromVMCPU++;

        // Connections between Operating System and Network
        osModule.toNet_TCP --> hypervisor.fromVMNet++;
        osModule.fromNet_TCP <-- hypervisor.toVMNet++;

        // INET internals...
        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        generic.ifIn <-- nl.out++ if exists(generic);
        generic.ifOut --> nl.in++ if exists(generic);

        tn.out++ --> ipv4.transportIn if hasIpv4;
        tn.in++ <-- ipv4.transportOut if hasIpv4;

        tn.out++ --> ipv6.transportIn if hasIpv6;
        tn.in++ <-- ipv6.transportOut if hasIpv6;

        tn.out++ --> generic.transportIn if hasGn;
        tn.in++ <-- generic.transportOut if hasGn;


        for i=0..numLoInterfaces-1 {
            nl.out++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> nl.in++;
        }

        for i=0..sizeof(radioIn)-1 {
            wlan[i].upperLayerOut --> nl.in++;
            wlan[i].upperLayerIn <-- nl.out++;
        }

        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerOut --> nl.in++;
            eth[i].upperLayerIn <-- nl.out++;
        }

        for i=0..sizeof(pppg)-1 {
            ppp[i].upperLayerOut --> nl.in++;
            ppp[i].upperLayerIn <-- nl.out++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> nl.in++;
            ext[i].upperLayerIn <-- nl.out++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> nl.in++;
            tun[i].upperLayerIn <-- nl.out++;
        }

        for i=0..numVlanInterfaces-1 {
            vlan[i].upperLayerOut --> nl.in++;
            vlan[i].upperLayerIn <-- nl.out++;
        }


        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
        }

        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
        }

}
