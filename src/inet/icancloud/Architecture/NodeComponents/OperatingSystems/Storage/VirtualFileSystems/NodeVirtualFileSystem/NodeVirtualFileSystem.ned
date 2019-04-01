// Module that implements an Generic I/O Redirector.
//
// This I/O Redirector contains information about the redirections to the 
// corresponding FS or to a corresponding APP module.
//
// This module contains a table with paths. Each one of these paths is associated
// to an index and a module type. This index correspond to a FS in the node's FS 
// array or to an APP in node's APP array.
//
// Local operations are sent to local File Systems: Parameter remoteOperation=false
// Remote operations are sent to corresponding client application: Parameter remoteOperation=true
//
// Operation list:
//
// - <b>Open a file</b><br>
//		- MessageName: inet.icancloud.App_IO_Message <br>
// 		- Operation: SM_OPEN_FILE<br>
//
// - <b>Close a file</b><br>
//		- MessageName: inet.icancloud.App_IO_Message <br>
// 		- Operation: SM_CLOSE_FILE<br>
//
// - <b>Read a file</b><br>
//		- MessageName: inet.icancloud.App_IO_Message <br>
// 		- Operation: SM_READ_FILE<br>
//
// - <b>Write a file</b><br>
//		- MessageName: inet.icancloud.App_IO_Message <br>
// 		- Operation: SM_WRITE_FILE<br>
//
// - <b>Creaate a file</b><br>
//		- MessageName: inet.icancloud.App_IO_Message <br>
// 		- Operation: SM_CREATE_FILE<br>
//
// - <b>Delete a file</b><br>
//		- MessageName: inet.icancloud.App_IO_Message <br>
// 		- Operation: SM_DELETE_FILE<br>
//
// NOTE: This module Supports NullCache and EnhancedFileCache caches.
//
// This module implements a virtual file system supporting management of tenants
//
// @author Gabriel González Castañé
// @date 2012-05-17

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VirtualFileSystems.NodeVirtualFileSystem;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VirtualFileSystems.IVirtualFileSystem;

simple NodeVirtualFileSystem like IVirtualFileSystem

{
    parameters:
        int numFS;						    // Number of File Systems
        @class(icancloud::NodeVirtualFileSystem);
        @display("i=device/fileSystem");
    gates:
        input fromMemory;			// Input gate from Memory
        input fromFS[];			// Input gates from File Systems

        output toMemory;			// Output gate to Memory
        output toFS[];			// Output gates to File Systems
}

