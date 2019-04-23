//
// This module defines the overheads parameters for a CitrixXenServer v.6.0 on a node with OS PV Linux
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-12
//


package inet.icancloud.Virtualization.Hypervisor.Hypervisors.HypervisorPreset;

import inet.icancloud.Virtualization.Hypervisor.Hypervisors.MainHypervisor;

module CitrixXenServer6_PV_Linux extends MainHypervisor
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
			 double memoryOverhead_MB = default (400.0);		
			 double computingOverhead = default(8.49);
			 double ioOverhead = default(0.0);
			 double networkOverhead = default(23.96);
}