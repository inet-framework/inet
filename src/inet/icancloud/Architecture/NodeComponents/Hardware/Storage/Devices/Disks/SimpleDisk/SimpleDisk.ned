// Module that implements a Generic Disk
//
// This module simulates a disk with a user defined bandwidth (for reads and writes)
//
// This module receives messages with a branch list. To calculate the service time, 
// this model only uses the total number of requested blocks, not each block position on disk.
//
// Operation list:
//
// - <b>Read a file</b><br>
//		- MessageName: inet.icancloud.BlockList_Message <br>
// 		- Operation: SM_READ_FILE<br>
//
// - <b>Write a file</b><br>
//		- MessageName: inet.icancloud.BlockList_Message <br>
// 		- Operation: SM_WRITE_FILE<br>
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
// @date 2012-23-11

package inet.icancloud.Architecture.NodeComponents.Hardware.Storage.Devices.Disks.SimpleDisk;

import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.Devices.IStorageDevice;

simple SimpleDisk like IStorageDevice

{
    parameters:
        @class(icancloud::SimpleDisk);
        int readBandwidth;		// Bandwidth (in MB/s) for reads
        int writeBandwidth;		// Bandwidth (in MB/s) for writes		
		string componentName = default ("genericDisk");
        @display("i=device/disk");
    gates:
        input in;		// Input gate
        output out;		// Output gate

}
