// Module that implements an Operating System Module for a node with a virtualization layer.
//
// This module is responsible for redirect all requests to  the corresponding subsytem
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author updated to iCanCloud by Gabriel González Castañé
// @date 2012-12-17

package inet.icancloud.Architecture.NodeComponents.VirtualOS.OperatingSystemModules;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.SyscallManager.ISyscallManager;
import inet.icancloud.Architecture.NodeComponents.VirtualOS.SyscallManager.VMSyscallManager;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.IOperatingSystem;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.CPU_Schedulers.ICPU_Scheduler;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VirtualFileSystems.IVirtualFileSystem;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems.FSModule;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.VolumeManagerModule;
import inet.icancloud.Virtualization.VirtualMachines.VMManagement.VmMsgController.VmMsgController;

module VmOperatingSystemModule like IOperatingSystem
{
    parameters:

        int numCPUs;													// Number of CPUs
        int numFS = default(1);													// Number of File Systems
        int numStorageSystems;											// Number of Storage Servers		

        string vfsType = default("VirtualFileSystem");		// Virtual File System type		
        string fsModuleType = default("Basic_FileSystem");				// File System module type
        string vmModuleType = default("VolumeManagerModule");		// Volume Manager module type
        string syscallManagerType = default("VMSyscallManager");
        string cpuSchedulerType = default("CPU_Scheduler_FIFO");		// CPU Service type   

        @display("i=icons/OOSS;bgb=306,405");
    gates:
        input fromApps[];			// Input gates from Applications
        input fromStorageSystem[];	// Input gates from Storage Servers
        input fromNet_TCP;			// Input gate from Network (Ethernet TCP)
        input fromCPU[];			// Input gates from CPU
        //***/
        input fromInputMemory;    // Input gates from imputs
        input fromOutputMemory;		// Input gate from output
        output toInputMemory;		// Output gates to inputs
        output toOutputMemory;		// Output gate to output

        output toApps[];			// Output gates to Applications
        output toStorageSystem[];	// Output gates to Storage Servers
        output toNet_TCP;			// Output gate to Network (Ethernet TCP)
        output toCPU[];				// Output gates to CPU


    submodules:

        syscallManager: <syscallManagerType> like ISyscallManager {
            parameters:
                @display("p=74,147");
            gates:
                fromApps[];
                toApps[];
        }

        cpuScheduler: <cpuSchedulerType> like ICPU_Scheduler {
            parameters:
                numCPUs = numCPUs;
                @display("p=74,62");
            gates:
                fromCPU[numCPUs];
                toCPU[numCPUs];
        }

        vfs: <vfsType> like IVirtualFileSystem {
            parameters:
                numFS = numFS;
                @display("p=63,256");
            gates:
                fromFS[numFS];
                toFS[numFS];
        }

        fsModule[numFS]: FSModule {
            @display("p=153,339");
        }

        vmModule: VolumeManagerModule {
            parameters:
                numStorageSystems = numStorageSystems;
                numFS = numFS;
                storageManagerType = default("NullStorageManager");		// Storage Manager type		
                schedulerType = default("NullStorageScheduler");			// Storage Scheduler Type		
                cacheType = default("NullCache");					// Cache Type
                @display("p=252,262");
            gates:
                fromFS[numFS];
                toFS[numFS];
                fromStorageSystem[numStorageSystems];
                toStorageSystem[numStorageSystems];
        }

        vmMsgController: VmMsgController {
            parameters:
                @display("p=192,147");
            gates:
                fromOSApps[];
                toOSApps[];
                fromApps[];
                toApps[];
        }

    connections allowunconnected:

        // Connections between Service Redirector and Network
        syscallManager.fromNet <-- fromNet_TCP;//networkService.toinet.icancloud.PI;
        syscallManager.toNet --> toNet_TCP;//networkService.frominet.icancloud.PI;

        // Connections between Service Redirector and CPU Service
        syscallManager.fromCPU <-- cpuScheduler.toOsGate;
        syscallManager.toCPU --> cpuScheduler.fromOsGate;

        // Connections between Service Redirector and Memory	
        syscallManager.fromMemory <-- fromInputMemory;
        syscallManager.toMemory --> toInputMemory;

        // Connections between Memory and I/O Redirector		
        toOutputMemory <-- vfs.toMemory;
        fromOutputMemory --> vfs.fromMemory;

        // Connections between Virtual File System and File Systems
        for i=0..numFS-1 {
            vfs.fromFS[i] <-- fsModule[i].toIOR;
            vfs.toFS[i] --> fsModule[i].fromIOR;
        }

        // Connections between File Systems and Volume Manager
        for i=0..numFS-1 {
            vmModule.fromFS[i] <-- fsModule[i].toVM;
            vmModule.toFS[i] --> fsModule[i].fromVM;
        }

        // Connections between Volume Manager and Storage Server
        for i=0..numStorageSystems-1 {
            vmModule.fromStorageSystem[i] <-- fromStorageSystem[i];
            vmModule.toStorageSystem[i] --> toStorageSystem[i];
        }

        // Connections between CPU Scheduler and CPU
        for i=0..numCPUs-1 {
            cpuScheduler.fromCPU[i] <-- fromCPU[i];
            cpuScheduler.toCPU[i] --> toCPU[i];
        }


}
