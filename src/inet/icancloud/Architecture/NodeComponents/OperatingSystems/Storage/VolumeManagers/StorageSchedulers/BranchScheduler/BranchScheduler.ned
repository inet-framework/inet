// Module that implements a Generic Block Scheduler
//
// This module schedules I/O requests.
// Basically, split all request on branch subRequests and inserts them on a queue.
//
// Supported operations:
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

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageSchedulers.BranchScheduler;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageSchedulers.IStorageScheduler;

simple BranchScheduler like IStorageScheduler

{
    parameters:
    @class(icancloud::BranchScheduler);
    gates:
        input fromInput;		// Input gate from input
        input fromOutput;		// Input gate from output		

        output toInput;		// Output gate to input
        output toOutput;		// Output gate to output

}

