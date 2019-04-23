//
// Module interface that implements a Node.
//
// @author Gabriel González Castañé
// @date 2014-12-12

package inet.icancloud.Architecture.Node;


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




import inet.icancloud.Architecture.NodeComponents.Hardware.CPUs.ICPU_Module;
import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.MainMemories.IMainMemory;
import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageSystems.StorageSystem;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.OperatingSystemModules.OperatingSystemModule;
import inet.icancloud.EnergySystem.EnergyMeter.EnergyMeter;
import inet.icancloud.EnergySystem.PSU.IPSU;
import inet.icancloud.Virtualization.Hypervisor.IHypervisor;
import inet.linklayer.loopback.LoopbackInterface;
import inet.networklayer.common.InterfaceTable;


moduleinterface INode

{
    parameters:
        @display("i=old/telnet");
        
        int numPcapRecorders;
        int numLoInterfaces;
        int numWlanInterfaces;
        int numEthInterfaces;  // minimum number of ethernet interfaces
        int numExtInterfaces;
        int numPppInterfaces;  // minimum number of PPP interfaces
        int numTunInterfaces;
        int numVlanInterfaces;
        
        // Main Hardware subsystems
        // CPU        
        
        string cpuModuleType;													// CPUModule type as CPU type
        int numCores;															// Number of CPU cores		
        // MEMORY
        string memoryType;														// RAMmemory as memory type  
        int memorySize_MB @unit(MiB);													// Total amount of memory of this node (in MB)
        // Storage
        bool storageNode;														// is this a atorage Node? If no, then its a compute node
        int storage_local_port;													// Port for listening the incoming connections
        string hostName;														// Node's hostname       
        int storageSize_GB  @unit(GiB);												// Total amount of space for each storage system
        int numStorageSystems;													// Number of Storage systems. As default, 1 storage system
        // Network
        int numNetworkInterfaces;												// As default, 1 network interface
       // string tcpType;   														// Type of TCP protocol          
        string ip;															// Node's IP
        string routingFile;														// Routing File
        int namid;																// Nam ID
        bool forwarding;
        bool multicastForwarding;


		//Energy
        bool activeEnergyMeter;
        string psuModuleType;													// PSU module
        string initialState;													// state or the node  NODE_STATE_OFF | NODE_STATE_IDLE | NODE_STATE_RUNNING
     gates:        
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[numPppInterfaces] @labels(PppFrame-conn) @allowUnconnected;
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;

}
