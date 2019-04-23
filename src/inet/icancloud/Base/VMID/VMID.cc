/*
 * VMID.cpp
 *
 *  Created on: Oct 18, 2012
 *      Author: gabriel
 */

#include "inet/icancloud/Base/VMID/VMID.h"

namespace inet {

namespace icancloud {


VMID::VMID(){
    user = -1;
    vmID = -1;
    vm = nullptr;
}

void VMID::initialize(int newUser, int vmid) {
	user = newUser;
	vmID = vmid;
	vm = nullptr;
}

} // namespace icancloud
} // namespace inet
