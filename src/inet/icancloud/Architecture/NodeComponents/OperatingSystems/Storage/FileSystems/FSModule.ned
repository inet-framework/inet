// Module that implements a GenericBasicFSModule.
//
// This module contains:
//
// - A GenericFS.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author Gabriel González Castañé
// @date 2013-10-6

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems;

module FSModule

{
    parameters:
        string fsType = default("Basic_FileSystem");		// File System Type

        @display("i=device/fileSystem");
    gates:
        input fromVM;			// Input gate from VM (Volume Manager)
        input fromIOR;			// Input gate from IOR (I/O Redirector)

        output toVM;			// Output gate to VM (Volume Manager)
        output toIOR;			// Output gate to IOR (I/O Redirector)


    submodules:
        fs: <fsType> like IFS {
            parameters:
                @display("i=device/fileSystem;p=60,100,row");

        }
    connections allowunconnected:

        // Conections with IOR (I/O Redirector)
        fromIOR --> fs.fromIOR;
        toIOR <-- fs.toIOR;

        // Conections with VM (Volume Manager)
        fromVM --> fs.fromVM;
        toVM <-- fs.toVM;

}
