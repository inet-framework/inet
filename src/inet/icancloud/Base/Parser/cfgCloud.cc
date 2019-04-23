/*
 * cfgCloud.cpp
 *
 *  Created on: May 15, 2013
 *      Author: gabriel
 */

#include "inet/icancloud/Base/Parser/cfgCloud.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;

CfgCloud::CfgCloud() {
	// TODO Auto-generated constructor stub
	nodes.clear();
	vms.clear();

}

CfgCloud::~CfgCloud() {
	// TODO Auto-generated destructor stub
}

string CfgCloud::getVMType (int index){

	vector <vmStructure*>::iterator vmIt;

	vmIt = vms.begin() + index;

	return (*vmIt)->vmtype.c_str();

}

int CfgCloud::getIndexForVM (string vmName){

    bool found = false;
    int position = -1;

    for (int i = 0; (i < (int)vms.size()) && (!found); i++){
        if ( strcmp((*(vms.begin() + i))->vmtype.c_str(), vmName.c_str()) == 0){
            found = true;
            position = i;
        }
    }

    if (position == -1)  throw cRuntimeError("CfgCloud::getIndexForVM-> vm name %s not found \n", vmName.c_str());

    return position;

}

int CfgCloud::getNumCores (int index){

	vector <vmStructure*>::iterator vmIt;

	vmIt = vms.begin() + index;

	return (*vmIt)->numCores;

}

uint64_t CfgCloud::getMemorySize (int index){

    vector <vmStructure*>::iterator vmIt;

    vmIt = vms.begin() + index;

    return (*vmIt)->memorySize_MB;

}

uint64_t CfgCloud::getStorageSize (int index){

    vector <vmStructure*>::iterator vmIt;

    vmIt = vms.begin() + index;

    return (*vmIt)->storageSize_GB;

}


void CfgCloud::printVms(){

	vector<vmStructure*>::iterator vmIt;
	printf ("------------------- SET OF VMs ---------------------\n");

	for (vmIt = vms.begin(); vmIt < vms.end(); vmIt++){
		printf ("Virtual Machine Type:\t %s\n", (*vmIt) ->vmtype.c_str());
		printf ("Num. cores: %i, memory Size(MB):%lu, storageSize(GB):%lu \n", (*vmIt)->numCores,(*vmIt)->memorySize_MB, (*vmIt)->storageSize_GB);
	}
}

void CfgCloud::setVMType (string vmName, int numCores, uint64_t memorySize_MB, uint64_t storageSize_GB){

	// Define ..
		 vmStructure *vm;

	// Init ..
		 vm = new vmStructure();

	// Insert the new node into the vector of nodes
		 vm->vmtype = vmName;
		 vm->numCores = numCores;
         vm->memorySize_MB = memorySize_MB;
         vm->storageSize_GB = storageSize_GB;

		 vms.push_back(vm);
}

} // namespace icancloud
} // namespace inet
