//
// Module interface that defines the behavior of a network hypervisor controller.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager;

moduleinterface IH_NETManager
{
    parameters:
        int storageApp_ModuleIndex;			// System storage application index
        int connectionTimeOut;				// Time limit for connecting applications        
		double networkOverhead;				// Percent of time that network increases as overhead per each operation.
        //string routingTableModule = default(absPath(".ipv4.routingTable"));
        string routingTableModule;
        
    gates:

        input fromVMNet[];
        output toVMNet[];
        input fromNodeNet;					// Input gate from Network (Ethernet TCP)
        output toNodeNet;					// Output gate to Network (Ethernet TCP)

        input fromHStorageManager;			// Input gate from Hypervisor Block Server Manager to NFS systems
        output toHStorageManager;			// Input gate from Hypervisor Block Server Manager to NFS systems

}

