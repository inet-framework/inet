// Module that implements a Generic Storage Scheduler
//
// nullptr Scheduler
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageSchedulers.NullStorageScheduler;

import inet.icancloud.Architecture.NodeComponents.OperatingSystems.Storage.VolumeManagers.StorageSchedulers.IStorageScheduler;

simple NullStorageScheduler like IStorageScheduler

{
    parameters:
    @class(icancloud::NullStorageScheduler);
    gates:
        input fromInput;		// Input gate from input
        input fromOutput;		// Input gate from output		

        output toInput;		// Output gate to input
        output toOutput;		// Output gate to output

}

