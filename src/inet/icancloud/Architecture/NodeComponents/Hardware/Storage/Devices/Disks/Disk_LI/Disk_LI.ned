// Module that implements a Generic Disk
//
// This module simulates a disk using Lineal Interpolation.
//
// This module receives messages with a branch list. To calculate the service time, 
// this model uses calculates each block position on disk.
//
// This module has been created using the tool HD_benchmark with sequential operations.
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


package inet.icancloud.Architecture.NodeComponents.Hardware.Storage.Devices.Disks.Disk_LI;

import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.Devices.IStorageDevice;

simple Disk_LI like IStorageDevice

{
    parameters:
    @class(icancloud::Disk_LI);
	string componentName = default ("generic_LI_Disk");
    @display("i=device/disk");
    gates:
        input in;		// Input gate
        output out;	// Output gate

}

