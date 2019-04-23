// Module that implements a Generic Main Memory
//
// Memory module that implements the basic functionality of memory system
// This module does not use cache blocks.
//
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute
// @date  2013-10-27

package inet.icancloud.Architecture.NodeComponents.Hardware.Memories.MainMemories.RAMmemory;

import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.MainMemories.IMainMemory;

simple RAMmemory like IMainMemory

{
    parameters:
    @class(icancloud::RAMmemory);
        double readLatencyTime_s;								// Latency time for read operations (in seconds)
        double writeLatencyTime_s;								// Latency time for write operations (in seconds)
        double searchLatencyTime_s;								// Latency time for search operations (in seconds)
        int size_MB @unit(MiB);											// Memory size (in MB)	
        int blockSize_KB @unit(KiB);									// Block Size (in KB)			
		int numDRAMChips;										// Number of DRAM chips in the same module
		int numModules;											// Number of connected slots in the node
        @display("i=icons/DDR");
        
    gates:
        input fromInput;		// Input gate from imputs
        input fromOutput;		// Input gate from output

        output toInput;			// Output gate to inputs
        output toOutput;		// Output gate to output

}

