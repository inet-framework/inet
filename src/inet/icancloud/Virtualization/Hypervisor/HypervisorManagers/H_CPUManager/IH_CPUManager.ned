//
// Module interface that defines the behavior of a CPU hypervisor controller..
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_CPUManager;


moduleinterface IH_CPUManager
{
    parameters:
        int numCPUs;				// Number of cores that the hypervisor is linked to.
        double computingOverhead;	// The percent of overhead that a cpu message requesting a computational block has to do
    gates:
        input fromVMCPU[];
        output toVMCPU[];

        input fromNodeCPU[numCPUs];
        output toNodeCPU[numCPUs];
}

