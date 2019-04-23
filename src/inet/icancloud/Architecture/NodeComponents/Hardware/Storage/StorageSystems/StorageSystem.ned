// Module that implements a Generic storage system
//
// This module contains:
//
// - A Generic storage device.
// - A storage controller.
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
// @date 2012-23-11

package inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageSystems;

import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageSystems.StorageSystem.MainStorageDevice;
import inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageController.StorageController;

module StorageSystem

{
    parameters:
        int numStorageSystems;

        @display("i=device/blockManager;bgb=223,243");
    gates:
        input fromHypervisor[];		// Input gate from Operating System	
        output toDevice[];			// Output gate to Operating System	


    submodules:

        device[numStorageSystems]: MainStorageDevice {
            parameters:
                @display("i=device/disk;p=103,64");

        }

        eController: StorageController {
            parameters:
                @display("p=103,158");
                numStorageSystems = numStorageSystems;
                
        }
    connections allowunconnected:

        for i=0..numStorageSystems-1 {
            fromHypervisor[i] --> device[i].fromOS;
            toDevice[i] <-- device[i].toOS;
        }

}
