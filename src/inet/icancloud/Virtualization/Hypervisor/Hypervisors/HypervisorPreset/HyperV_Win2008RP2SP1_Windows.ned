//
// This module defines the overheads parameters for a HyperV v.6.0 on a node with OS Windows 2008-RP2-SP1
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-12
//

package inet.icancloud.Virtualization.Hypervisor.Hypervisors.HypervisorPreset;

import inet.icancloud.Virtualization.Hypervisor.Hypervisors.MainHypervisor;

module HyperV_Win2008RP2SP1_Windows extends MainHypervisor
{
    parameters:
        @class("icancloud::Hypervisor");
        @display("i=energy/Hypervisor_40;p=208,236");
    
    	// Scheduler
            string cpuScheduler = default("H_CPU_SCHED_FIFO");
    		string netScheduler = default("H_NET_SCHED_FIFO");
    		string memScheduler = default("H_MEM_SCHED_FIFO");
    		string storageScheduler = default("H_STORAGE_SCHED_FIFO");
		
		// Overhead
			// The memory overhead is the amount of memory that the hypervisor needs at memory (MB)
			// The computing overhead is the average percent of overload that cpu operations need to me performed
			// The io overhead is the average percent of overload that io operations need to me performed
			// The netowrk overhead is the average percent of overload that network operations need to me performed
			 int memoryOverhead_MB = default (512);
			 double computingOverhead = default(3.10);
			 double ioOverhead = default(20.96);
			 double networkOverhead = default(24.34);
}