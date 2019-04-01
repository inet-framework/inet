// Module that implements a Generic Volume Manager Module.
//
// This module contains:
//
// - A GenericStorageManager.
// - A GenericStorageScheduler.
// - A GenericMemory.
//
// This module calculates the location of the Storages involved in an IO operation,
// split the request messages in N subRequest messages to Storage Servers, and wait to
// all these subRequest responses to send a response to the corresponding File System.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30


package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageManagers.IStorageManager;
import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageSchedulers.IStorageScheduler;
import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches.ICache;


module VolumeManagerModule

{
    parameters:
        string storageManagerType;		// Storage Manager type		
        string schedulerType;			// Storage Scheduler Type		
        string cacheType;				// Cache Type
        int numFS;					// Number of File Systems
        int numStorageSystems;		// Number of Storage Systems


        @display("i=device/StorageManager");
    gates:
        input fromFS[];			// Input gates from File Systems
        input fromStorageSystem[];	// Input gates from Storage Servers   						

        output toFS[];			// Output gates from File Systems
        output toStorageSystem[];	// Output gates from Storage Servers   


    submodules:

        storageManager: <storageManagerType> like IStorageManager {
            parameters:
                numStorageSystems = numStorageSystems;
                @display("i=icons/Redirector;p=100,100");

            gates:
                fromStorageSystem[numStorageSystems];
                toStorageSystem[numStorageSystems];
        }
        scheduler: <schedulerType> like IStorageScheduler {
            parameters:
                @display("i=icons/Redirector;p=100,200");

        }
        cache: <cacheType> like ICache {
            parameters:
                numInputs = numFS;
                @display("i=icons/chip;p=100,300");


            gates:
                fromInput[numFS];
                toInput[numFS];
        }
    connections allowunconnected:

        // Conections with FSs
        for i=0..numFS-1 {
            fromFS[i] --> cache.fromInput[i];
            toFS[i] <-- cache.toInput[i];
        }

        // Conections between Cache and Scheduler
        cache.fromOutput <-- scheduler.toInput;
        cache.toOutput --> scheduler.fromInput;

        // Conections between Scheduler and Storage Manager
        scheduler.fromOutput <-- storageManager.toScheduler;
        scheduler.toOutput --> storageManager.fromScheduler;

        // Connections with Storage Servers
        for i=0..numStorageSystems-1 {
            fromStorageSystem[i] --> storageManager.fromStorageSystem[i];
            toStorageSystem[i] <-- storageManager.toStorageSystem[i];
        }

}
