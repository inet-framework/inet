//
// @class Node_FileSystem Node_FileSystem.h "Node_FileSystem.h"
//
// This module translates an I/O file request expressed as (fileName, offset, requestSize) to a list of disk blocks.
// It is able to distinguish between different tenant accounts at file system.
// It's based on Basic_FileSystem
//
// @author Gabriel González Castañé
// @date 2013-10-6

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems.Node_FileSystem;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems.IFS;

simple Node_FileSystem like IFS

{
    parameters:
        @class(icancloud::Node_FileSystem);
        @display("i=device/fileSystem");
    gates:
        input fromVM;		// Input gate from VM (Volume Manager)
        input fromIOR;	// Input gate from IOR (I/O Redirector)

        output toVM;		// Output gate to VM (Volume Manager)
        output toIOR;		// Output gate to IOR (I/O Redirector)

}

