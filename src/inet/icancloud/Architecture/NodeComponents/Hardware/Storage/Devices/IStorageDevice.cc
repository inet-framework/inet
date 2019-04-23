#include "inet/icancloud/Architecture/NodeComponents/Hardware/Storage/Devices/IStorageDevice.h"

namespace inet {

namespace icancloud {


void IStorageDevice::initialize(int stage){

    HWEnergyInterface::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {

        storageMod = check_and_cast<StorageController*> (getParentModule()->getParentModule()->getSubmodule("eController"));
        storageMod->registerDevice(this, getIndex());
    }
}

void IStorageDevice::e_changeState (const string &energyState){
    storageMod->e_changeState(energyState, getIndex());
}

} // namespace icancloud
} // namespace inet
