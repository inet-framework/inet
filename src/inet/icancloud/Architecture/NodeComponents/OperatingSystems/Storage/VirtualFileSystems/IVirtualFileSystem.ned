// Module that represents a Generic I/O Redirector
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VirtualFileSystems;

moduleinterface IVirtualFileSystem
{
    parameters:
        int numFS;						// Number of File Systems
        @display("i=device/fileSystem");
    gates:
        input fromMemory;				// Input gate from Memory
        input fromFS[];					// Input gates from File Systems

        output toMemory;				// Output gate to Memory
        output toFS[];					// Output gates to File Systems

}

