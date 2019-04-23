// Module that represents a Generic Block Manager.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageManagers;

moduleinterface IStorageManager

{
    parameters:
        int numStorageSystems;		// Number of Block Servers		

        @display("i=icons/Redirector");
    gates:
        input fromScheduler;		// Input gate from Scheduler
        input fromStorageSystem[];	// Input gates from Block Servers		

        output toScheduler;			// Output gate to Scheduler
        output toStorageSystem[];	// Output gates to Block Servers	

}
