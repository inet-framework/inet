// Module that implements a Generic Block Server Module.
//
// This module contains:
//
// - A GenericDisk.
// - A GenericMemory.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageSystems.StorageSystem;

import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches.ICache;
import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.Devices.IStorageDevice;


module MainStorageDevice

{
    parameters:
        string deviceType;		// Device Type
        string cacheType;		// Cache Type

        @display("i=device/blockManager");
    gates:
        input fromOS;		// Input gate from Operating System	
        output toOS;		// Output gate to Operating System	


    submodules:

        device: <deviceType> like IStorageDevice {
            parameters:
                @display("i=device/disk;p=100,100");

        }
        cache: <cacheType> like ICache {
            parameters:
                numInputs = 1;
                @display("i=block/table2;p=100,300");


            gates:
                fromInput[1];
                toInput[1];
        }

    connections allowunconnected:

        // Conections with Operating System
        fromOS --> cache.fromInput[0];
        toOS <-- cache.toInput[0];

        // Conections between Cache and Storage Device
        cache.fromOutput <-- device.out;
        cache.toOutput --> device.in;

}
