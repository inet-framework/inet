//
// Module that represents a storage controller 
//
// This module is responsible for controlling all storage devices of a machine 
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
// @date 2012-23-11

package inet.icancloud.Architecture.NodeComponents.Hardware.Storage.StorageController;

simple StorageController

{
    parameters:
    @class(icancloud::StorageController);
          int numStorageSystems;
}
