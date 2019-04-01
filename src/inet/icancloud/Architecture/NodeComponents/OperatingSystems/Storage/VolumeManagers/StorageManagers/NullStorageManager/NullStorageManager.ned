// Module that implements a Generic Storage Manager
//
// This module calculates the location of the Storages involved in an I/O operation,
// and send the corresponding requests to the corresponding Storage Server.
//
// If parameter <b>numStorageServers</b> is greater than 1, this volume manager
// will act like a RAID controller. Else, if <b>numStorageServers</b> is equal to
// 1, then this volume manager will act like a single disk controller.
//
//
// Operation list:
//
// - <b>Read a file</b><br>
//		- MessageName: inet.icancloud.StorageList_Message <br>
// 		- Operation: SM_READ_FILE<br>
//
// - <b>Write a file</b><br>
//		- MessageName: inet.icancloud.StorageList_Message <br>
// 		- Operation: SM_WRITE_FILE<br>
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageManagers.NullStorageManager;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageManagers.IStorageManager;

simple NullStorageManager like IStorageManager

{
    parameters:
        @class(icancloud::NullStorageManager);
        int numStorageSystems;		// Number of Storage Servers


    gates:
        input fromScheduler;		// Input gate from Scheduler
        input fromStorageSystem[];	// Input gates from Storage Servers

        output toScheduler;		// Output gate to Scheduler
        output toStorageSystem[];	// Output gates to Storage Servers

}

