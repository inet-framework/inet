//
// Module that defines the behavior of a network hypervisor manager.
//
// It is composed by two main parts:
//	- Local net manager: Manager responsible for translate virtual ports and virtual ip address to real physical ports and ip address
//	- network manager: Manager that manages the order of arrival messages from VMs to access to physical resources
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.Managers;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.LocalNetManager.LocalNetManager;
import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.IH_NETManager;
import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.Schedulers.**.*;

module H_NETManager like IH_NETManager
{
    parameters:
        string schedulerType;
        int storageApp_ModuleIndex;
        int connectionTimeOut;        
		double networkOverhead;
		string routingTableModule = default("..^.ipv4.routingTable");
        @display("bgb=144,247");
    gates:

        input fromVMNet[];
        output toVMNet[];
        input fromNodeNet;			// Input gate from Network (Ethernet TCP)
        output toNodeNet;			// Output gate to Network (Ethernet TCP)

        input fromHStorageManager;		// Input gate from Hypervisor Storage Manager to NFS systems
        output toHStorageManager;		// Input gate from Hypervisor Storage Server Manager to NFS systems

    submodules:

        localNetManager: LocalNetManager {
            @display("p=77,179");
        }

        netManager: <schedulerType> like IH_NETManager{
            parameters:
        		networkOverhead = networkOverhead;        		
                storageApp_ModuleIndex = storageApp_ModuleIndex;
                connectionTimeOut = connectionTimeOut;
                routingTableModule = routingTableModule;
                @display("i=icons/Redirector;p=77,73");
            gates:

                fromVMNet[];
                toVMNet[];
                fromNodeNet;
                toNodeNet;
                fromHStorageManager;
                toHStorageManager;
        }

    connections allowunconnected:

        // Connections between Network Manager and Block Server Manager
        netManager.fromHStorageManager <-- fromHStorageManager;
        netManager.toHStorageManager --> toHStorageManager;

        // Connections between VM Network and Network Service
        netManager.fromNodeNet <-- fromNodeNet;
        netManager.toNodeNet --> toNodeNet;

        netManager.fromVMNet++ <-- fromVMNet++;
        netManager.toVMNet++ --> toVMNet++;
}

