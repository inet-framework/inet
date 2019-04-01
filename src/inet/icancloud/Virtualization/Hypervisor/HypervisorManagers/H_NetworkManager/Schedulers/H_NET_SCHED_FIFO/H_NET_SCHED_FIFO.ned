//
// Module that defines the behavior of hypervisor network controller.
//
// The scheduling policy of this module is first input first output.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.Schedulers.H_NET_SCHED_FIFO;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.IH_NETManager;

simple H_NET_SCHED_FIFO like IH_NETManager
{
    parameters:
        @class(icancloud::H_NET_SCHED_FIFO);
        int storageApp_ModuleIndex;
        int connectionTimeOut;
        string routingTableModule = default(absPath("ipv4.routingTable"));
		double networkOverhead;
		
    gates:

        input fromVMNet[];
        output toVMNet[];
        input fromNodeNet;			// Input gate from Network (Ethernet TCP)
        output toNodeNet;			// Output gate to Network (Ethernet TCP)

        input fromHStorageManager;		// Input gate from Hypervisor Block Server Manager to NFS systems
        output toHStorageManager;		// Input gate from Hypervisor Block Server Manager to NFS systems

}

