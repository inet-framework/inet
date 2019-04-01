//
// This application is an application system managed by syscall manager in order to create connections
// between compute nodes and remote storage servers. In addition it is also in charge of load tenant
// files from preloadfiles (files that is going to be loaded before an user application execution) and the
// file system structure.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute;
// @date 2013-03-04

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.SystemApps.RemoteStorageApp;

simple RemoteStorageApp
{
    parameters:
       @class(icancloud::RemoteStorageApp);
        string application_netType = default("INET"); 		// Network type (INET or BASIC)
     
    gates:
        input fromOS;							// Input gate from OS (Operating System)
        output toOS;							// Output gate to OS (Operating System)

}
