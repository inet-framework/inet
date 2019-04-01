// Module that represents a Generic FS (File System)
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30
//
// @author Gabriel González Castañé
// @date 2013-10-6

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.FileSystems;

moduleinterface IFS

{
    @display("i=device/fileSystem");
    gates:
        input fromVM;		// Input gate from VM (Volume Manager)
        input fromIOR;	// Input gate from IOR (I/O Redirector)

        output toVM;		// Output gate to VM (Volume Manager)
        output toIOR;		// Output gate to IOR (I/O Redirector)

}

