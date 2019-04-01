package inet.examples.icancloud.Cloud_C; 

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
channel Channel_0_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_1_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_2_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_3_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_4_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_5_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_6_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_7_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_8_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 1.25E-4;
   per = 1.25E-4;
}

channel Channel_9_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_10_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel Channel_11_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_0_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_1_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_2_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Gbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_3_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Mbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_4_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Mbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

channel RackChannel_5_Cloud_C extends DatarateChannel{
   delay = 1.25E-4us;
   datarate = 10Mbps;// RackChannelBandwidth
   ber = 0.0;
   per = 0.0;
}

// ---------------------------------------------------------------
//   Definition of the cloud
// ---------------------------------------------------------------
network Cloud_C{

   // -------------------------------------------------------
   //   Definition of main modules
   // -------------------------------------------------------
   submodules:


	// ---------------------------------------------------------------
	//   Network configurator
	// ---------------------------------------------------------------

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

	switch_1:EtherSwitch{
	}

	switch_2:EtherSwitch{
	}

	switch_3:EtherSwitch{
	}

	rackSwitch_0:EtherSwitch{
	}

	rackSwitch_1:EtherSwitch{
	}

	rackSwitch_2:EtherSwitch{
	}

	rackSwitch_3:EtherSwitch{
	}

	rackSwitch_4:EtherSwitch{
	}

	rackSwitch_5:EtherSwitch{
	}

	// ---------------------------------------------------------------
	//   Definition of Computing Nodes
	// ---------------------------------------------------------------
	// ---------------------------------------------------------------
	//   Definition of Storage Nodes
	// ---------------------------------------------------------------
	ns_0_NodeStorage_C:NodeVL;

	ns_1_NodeStorage_C:NodeVL;

	ns_2_NodeStorage_C:NodeVL;

	//------------------------------------------------------
	// Modules that defines the topology and the set of VMs 
	//------------------------------------------------------
	  vmSet: VmSet {} 
	  topology: Topology {} 


	// ---------------------------------------------------------------
	//   Definition of Racks
	// ---------------------------------------------------------------
	rc_0_Rack_B_32[32]:NodeVL;

	rc_1_Rack_B_32[32]:NodeVL;

	rc_2_Rack_B_32[32]:NodeVL;

	rc_3_Rack_C_48[48]:NodeVL;

	rc_4_Rack_C_48[48]:NodeVL;

	rc_5_Rack_C_48[48]:NodeVL;

	// ---------------------------------------------------------------
	//   Connections section
	// ---------------------------------------------------------------
	configurator: Ipv4NetworkConfigurator{
	   parameters:
	     config = xml("<config><interface hosts='**' address='192.168.x.x' netmask='255.x.x.x'/></config>");
	}
	
	connections allowunconnected:

	// Connections for rack rc_0_Rack_B_32
	for i=0..31{
	   rc_0_Rack_B_32[i].ethg++ <--> RackChannel_0_Cloud_C <--> rackSwitch_0.ethg++;
	}

	// Connections for rack rc_1_Rack_B_32
	for i=0..31{
	   rc_1_Rack_B_32[i].ethg++ <--> RackChannel_0_Cloud_C <--> rackSwitch_1.ethg++;
	}

	// Connections for rack rc_2_Rack_B_32
	for i=0..31{
	   rc_2_Rack_B_32[i].ethg++ <--> RackChannel_0_Cloud_C <--> rackSwitch_2.ethg++;
	}

	// Connections for rack rc_3_Rack_C_48
	for i=0..47{
	   rc_3_Rack_C_48[i].ethg++ <--> RackChannel_0_Cloud_C <--> rackSwitch_3.ethg++;
	}

	// Connections for rack rc_4_Rack_C_48
	for i=0..47{
	   rc_4_Rack_C_48[i].ethg++ <--> RackChannel_0_Cloud_C <--> rackSwitch_4.ethg++;
	}

	// Connections for rack rc_5_Rack_C_48
	for i=0..47{
	   rc_5_Rack_C_48[i].ethg++ <--> RackChannel_0_Cloud_C <--> rackSwitch_5.ethg++;
	}

	   rackSwitch_1.ethg++ <--> Channel_0_Cloud_C <--> switch_0.ethg++;
	   rackSwitch_0.ethg++ <--> Channel_1_Cloud_C <--> switch_0.ethg++;
	   switch_0.ethg++ <--> Channel_2_Cloud_C <--> rackSwitch_2.ethg++;
	   ns_2_NodeStorage_C.ethg++ <--> Channel_3_Cloud_C <--> switch_1.ethg++;
	   ns_1_NodeStorage_C.ethg++ <--> Channel_4_Cloud_C <--> switch_1.ethg++;
	   ns_0_NodeStorage_C.ethg++ <--> Channel_5_Cloud_C <--> switch_1.ethg++;
	   switch_0.ethg++ <--> Channel_6_Cloud_C <--> switch_3.ethg++;
	   switch_3.ethg++ <--> Channel_7_Cloud_C <--> switch_1.ethg++;
	   switch_2.ethg++ <--> Channel_8_Cloud_C <--> switch_3.ethg++;
	   rackSwitch_3.ethg++ <--> Channel_9_Cloud_C <--> switch_2.ethg++;
	   rackSwitch_4.ethg++ <--> Channel_10_Cloud_C <--> switch_2.ethg++;
	   rackSwitch_5.ethg++ <--> Channel_11_Cloud_C <--> switch_2.ethg++;
}
