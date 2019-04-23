//
// @class Basic_FileSystem Basic_FileSystem.h "Basic_FileSystem.h"
//
// This module translates an I/O file request expressed as (fileName, offset, requestSize) to a list of disk blocks.
// This module only is useful with one tenant at system. It is not able to distinguish between more ones.
//
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2009-03-11
//
// @author updated to iCanCloud by Gabriel González Castañé
// @date 2012-05-17



package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems.Basic_FileSystem;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems.IFS;

simple Basic_FileSystem like IFS

{
    parameters:
        @class(icancloud::Basic_FileSystem);
        @display("i=device/fileSystem");
    gates:
        input fromVM;		// Input gate from VM (Volume Manager)
        input fromIOR;	// Input gate from IOR (I/O Redirector)

        output toVM;		// Output gate to VM (Volume Manager)
        output toIOR;		// Output gate to IOR (I/O Redirector)

}

