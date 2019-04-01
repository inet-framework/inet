// Module that implements a Generic Main Memory
//
// Memory module that implements the read-ahead and write-back functionality.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2009-03-11
// @author (modified) Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute
// @date (modified) 2013-10-27

package inet.icancloud.Architecture.NodeComponents.Hardware.Memories.MainMemories.RAMMemory_BlockModel;

import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.MainMemories.IMainMemory;

simple RAMMemory_BlockModel like IMainMemory
{
	parameters:	
	    @class(icancloud::RAMMemory_BlockModel);
        double readLatencyTime_s;								// Latency time for read operations (in seconds)
        double writeLatencyTime_s;								// Latency time for write operations (in seconds)
        double searchLatencyTime_s;								// Latency time for write operations (in seconds)
		double flushTime_s;							// Timer to flush cache (in seconds)	
		int size_MB @unit(MiB);											// Memory size (in MB)	
		int	sizeCache_KB;			// Amount of memory dedicated for disk cache space
        int blockSize_KB @unit(KiB);									// Block Size (in KB)			
		int readAheadBlocks; 		// Window Size to read-ahead operations (in number of blocks)			

		int numDRAMChips;										// Number of DRAM chips in the same module
		int numModules;											// Number of connected slots in the node

    gates:
        input fromInput;		// Input gate from imputs
        input fromOutput;		// Input gate from output

        output toInput;			// Output gate to inputs
        output toOutput;		// Output gate to output

	
}

