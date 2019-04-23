//
// Module that defines the behavior of hypervisor memory controller.
//
// The scheduling policy of this module is first input first output.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_MemoryManager.Schedulers.H_MEM_SCHED_FIFO;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_MemoryManager.IH_MemoryManager;

simple H_MEM_SCHED_FIFO like IH_MemoryManager
{
	parameters:
	@class(icancloud::H_MEM_SCHED_FIFO);
		int blockSize_KB @unit(KiB);
		int memorySize_MB @unit(MiB);
		
		int memoryOverhead_MB;
    gates:
		input   fromVMMemoryI[];
        input   fromVMMemoryO[];
        output  toVMMemoryI[];
        output  toVMMemoryO[];
        
        input   fromNodeMemoryI;
        input   fromNodeMemoryO;
        output  toNodeMemoryI;
        output  toNodeMemoryO;
}

