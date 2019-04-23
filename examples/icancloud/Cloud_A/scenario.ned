
package inet.examples.icancloud.Cloud_A; 

import ned.DatarateChannel; 
import inet.node.ethernet.EtherSwitch;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.icancloud.Architecture.Node.NodeVL.NodeVL; 
import inet.icancloud.Management.CloudManagement.ResourcesProvisioning.**.*; 
import inet.icancloud.Management.NetworkManager.NetworkManager; 
import inet.icancloud.Management.Topology.Topology; 
import inet.icancloud.Management.NetworkManager.NetworkManager; 
import inet.icancloud.Users.UserGenerator.UserGenerator; 
import inet.icancloud.Virtualization.VirtualMachines.SetDefinition.VmSet; 

// ---------------------------------------------------------------
//   Definition of channels
// ---------------------------------------------------------------
channel Channel_0_Cloud_A extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_1_Cloud_A extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_0_Cloud_A extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

// ---------------------------------------------------------------
//   Definition of the cloud
// ---------------------------------------------------------------
network Cloud_A{

   // -------------------------------------------------------
   //   Definition of main modules
   // -------------------------------------------------------
   submodules:

	// ---------------------------------------------------------------
	//    Network Manager
	// ---------------------------------------------------------------
	networkManager: NetworkManager{
	}

	// ---------------------------------------------------------------
	//   Definition of cloud scheduler
	// ---------------------------------------------------------------
	 manager: CloudSchedulerFCFS{
	   parameters:
	     numberOfPFSRemoteServers = 2;
	     remote_storage_VMs_per_node = 2;
	     minimum_percent_storageNode_free = 0.15;
	     virtualIPsBasis = "10.0.0.1";
	}

	// ---------------------------------------------------------------
	//   Definition of user generator
	// ---------------------------------------------------------------
	userGenerator: UserGenerator {
	   parameters:
	     numCells = 1;
	}

	// ---------------------------------------------------------------
	//   Definition of switches
	// ---------------------------------------------------------------
	switch_0:EtherSwitch{
	}

	rackSwitch_0:EtherSwitch{
	}

	// ---------------------------------------------------------------
	//   Definition of Computing Nodes
	// ---------------------------------------------------------------
	// ---------------------------------------------------------------
	//   Definition of Storage Nodes
	// ---------------------------------------------------------------
	ns_0_NodeStorage_C:NodeVL;

	//------------------------------------------------------
	// Modules that defines the topology and the set of VMs 
	//------------------------------------------------------
	  vmSet: VmSet {} 
	  topology: Topology {} 


	// ---------------------------------------------------------------
	//   Definition of Racks
	// ---------------------------------------------------------------
	rc_0_Rack_A_16[16]:NodeVL;

	// ---------------------------------------------------------------
	//   Connections section
	// ---------------------------------------------------------------
	
	// ---------------------------------------------------------------
	//   Network configurator
	// ---------------------------------------------------------------
	configurator: Ipv4NetworkConfigurator{
	   parameters:
	     config = xml("<config><interface hosts='**' address='192.168.x.x' netmask='255.x.x.x'/></config>");
	}
	
	connections allowunconnected:

	// Connections for rack rc_0_Rack_A_16
	for i=0..15{
	   rc_0_Rack_A_16[i].ethg++ <--> RackChannel_0_Cloud_A <--> rackSwitch_0.ethg++;
	}

	   rackSwitch_0.ethg++ <--> Channel_0_Cloud_A <--> switch_0.ethg++;
	   ns_0_NodeStorage_C.ethg++ <--> Channel_1_Cloud_A <--> switch_0.ethg++;
}
