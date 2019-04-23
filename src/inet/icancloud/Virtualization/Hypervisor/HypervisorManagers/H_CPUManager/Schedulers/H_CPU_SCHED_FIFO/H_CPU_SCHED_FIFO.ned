//
// Module that defines the behavior of hypervisor cpu controller.
//
// The scheduling policy of this module is first input first output.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_CPUManager.Schedulers.H_CPU_SCHED_FIFO;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_CPUManager.IH_CPUManager;

simple H_CPU_SCHED_FIFO like IH_CPUManager
{
    parameters:
    @class(icancloud::H_CPU_SCHED_FIFO);
        int numCPUs;
        double computingOverhead = default(0);
    gates:
        input fromVMCPU[];
        output toVMCPU[];

        input fromNodeCPU[numCPUs];
        output toNodeCPU[numCPUs];
}

