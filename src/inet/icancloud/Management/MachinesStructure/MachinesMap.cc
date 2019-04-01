//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/icancloud/Management/MachinesStructure/MachinesMap.h"

namespace inet {

namespace icancloud {


MachinesMap::MachinesMap() {
	machineMap.clear();
}

MachinesMap::~MachinesMap() {
	machineMap.clear();
}



void MachinesMap::setInstances (HeterogeneousSet* newmachineSet){

	machineMap.insert(machineMap.end(),newmachineSet);

}

int MachinesMap::isSet(string typeName){
    bool found = false;
    int position = -1;
    for (int i = 0; (i < (int)machineMap.size()) && (!found); i++){
        if (strcmp((getSetId(i)).c_str(), typeName.c_str()) == 0){
            found = true;
            position = i;
        }
    }

    return position;

}

void MachinesMap::increaseInstances (vector<Machine*> machines, string typeName){

    // Define ..

        HeterogeneousSet* set;
        elementType* elt;
        int position = isSet(typeName);
        Machine* m;
    // Initialize

        if (position == -1){
            elt = new elementType();
            m = (*(machines.begin()));
            elt->setTypeParameters(m->getNumCores(), m->getMemoryCapacity(), m->getStorageCapacity(), m->getNumNetworkIF(),
                               typeName.c_str(), m->getNumOfStorageDevices());

            set = new HeterogeneousSet();
            set->setType(elt);
            machineMap.push_back(set);
            position = machineMap.size() - 1;
        }

        for (int i = 0; i < (int)machines.size(); i++)
            (*(machineMap.begin() + position))->initMachine((*(machines.begin()+i)));

}

void MachinesMap::destroyInstance (string type, int id){

    // Define ..
    int count = 0;
    bool found = false;
    vector<HeterogeneousSet*>::iterator setIt;

    for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

        if (strcmp(getSetIdentifier(count).c_str(),type.c_str()) == 0){

            (*setIt)->deleteMachine(id) ;
            found = true;
        }
        else {
            count ++;
        }
    }
}

int MachinesMap::getMapQuantity (){
	return machineMap.size();
}

int MachinesMap::getSetIndex (string machineSetId){

	// Define ..

		int index;
		bool found;
		vector<HeterogeneousSet*>::iterator setIt;

	// Init ..
		found = false;
		index = -1;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found);){

			if ((*setIt)->getSetIdentifier() == machineSetId){
				found = true;
			}
			else{
				index ++;
				setIt++;
			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachineSetIndex->Error. machineSetID %s does not exists", machineSetId.c_str());

		return index;

}

string MachinesMap::getSetId (int index){

	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		string result;
		int machineMapSize;

	// Init ..
		result.clear();
		machineMapSize = machineMap.size();
	// Begin ..

		if ((index < machineMapSize) && (index >=0)){
			setIt = machineMap.begin()+index;
			result = (*setIt)->getSetIdentifier();
		}
		else {
			throw cRuntimeError("machinesMap::getmachineSetId->Error. Index (%i) - machineMapIndex(%i)", index, machineMap.size());
		}

	return result;

}

int MachinesMap::getSetQuantity (string machineSetId){


	// Define ..

		int quantity;
		bool found;
		vector<HeterogeneousSet*>::iterator setIt;

	// Init ..

		quantity = -1;
		found = false;
	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if ((*setIt)->getSetIdentifier() == machineSetId){

				quantity = (*setIt)->size();
				found = true;

			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachineSetQuantity->Error. machineSetID %s does not exists", machineSetId.c_str());

	return quantity;

}

int MachinesMap::getSetQuantity (int machineIndex){

	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		int result;
		int machineMapSize;
	// Init ..
		result = -1;
        machineMapSize = machineMap.size();
	// Begin ..

		if ((machineIndex < machineMapSize) && (machineIndex >= 0)){
			setIt = machineMap.begin()+machineIndex;
			result = (*setIt)->size();
		}
		else {
			throw cRuntimeError("machinesMap::getmachineSetId->Error. Index (%i) - machineMapIndex(%i)", machineIndex, machineMap.size());
		}

		return result;
}

int MachinesMap::getSetNumberOfCPUs(string machineSetId){

	// Define ..

		int quantity;
		vector<HeterogeneousSet*>::iterator setIt;
		bool found;

	// Init ..
		quantity = -1;
		found = false;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if ((*setIt)->getSetIdentifier() == machineSetId){

				quantity = (*setIt)->getNumberOfCPUs() ;
				found = true;

			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachineSetNumberOfCPUs->Error. machineSetID %s does not exists", machineSetId.c_str());


	return quantity;

}

int MachinesMap::getSetNumberOfCPUs(int machineIndex){

	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		int result;
		int machineMapSize;
	// Init ..
		result = -1;
        machineMapSize = machineMap.size();
	// Begin ..

		if ((machineIndex < machineMapSize) && (machineIndex >= 0)){
			setIt = machineMap.begin()+machineIndex;
			result = (*setIt)->getNumberOfCPUs();
		}
		else {
			throw cRuntimeError("machinesMap::getmachineSetNumberOfCPUs->Error. Index (%i) - machineMapIndex(%i)", machineIndex, machineMap.size());
		}

		return result;

}

int MachinesMap::getSetTotalMemory(string machineSetId){

	// Define ..

		int quantity;
		vector<HeterogeneousSet*>::iterator setIt;
		bool found;

	// Init ..
		quantity = -1;
		found = false;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if ((*setIt)->getSetIdentifier() == machineSetId){

				quantity = (*setIt)->getTotalMemory() ;
				found = true;

			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachineSetTotalMemory->Error. machineSetID %s does not exists", machineSetId.c_str());

	return quantity;
}

int MachinesMap::getSetTotalStorage(string machineSetId){

    // Define ..

        int quantity;
        vector<HeterogeneousSet*>::iterator setIt;
        bool found;

    // Init ..
        quantity = -1;
        found = false;

    // Begin ..

        for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

            if ((*setIt)->getSetIdentifier() == machineSetId){

                quantity = (*setIt)->getStorageSize() ;
                found = true;

            }

        }

        if (!found) throw cRuntimeError("machinesMap::getmachineSetStorageSize->Error. machineSetID %s does not exists", machineSetId.c_str());

    return quantity;
}

int MachinesMap::getSetTotalMemory(int machineIndex){
    // Define ..

        vector<HeterogeneousSet*>::iterator setIt;
        int result;
        int machineMapSize;
    // Init ..
        result = -1;
        machineMapSize = machineMap.size();
    // Begin ..

        if ((machineIndex < machineMapSize) && (machineIndex >= 0)){
            setIt = machineMap.begin()+machineIndex;
            result = (*setIt)->getTotalMemory();
        }
        else {
            throw cRuntimeError("machinesMap::getmachineSetTotalMemory->Error. Index (%i) - machineMapIndex(%i)", machineIndex, machineMap.size());
        }

        return result;
}


string MachinesMap::getSetIdentifier(int machineIndex){
	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		string result;
		int machineMapSize;
	// Init ..
		result.clear();
        machineMapSize = machineMap.size();
	// Begin ..
		if ((machineIndex < machineMapSize) && (machineIndex >=0)){
			setIt = machineMap.begin()+machineIndex;
			result = (*setIt)->getSetIdentifier();
		}
		else {
			throw cRuntimeError("machinesMap::getmachineSetIdentifier->Error. Index (%i) - machineMapIndex(%i)", machineIndex, machineMap.size());
		}

		return result;
}

bool MachinesMap::isMachineON (string machineSetId, int machineIndex){

	// Define ..

		bool state;
		vector<HeterogeneousSet*>::iterator setIt;

	// Init ..

		state = "";
		bool found = false;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if ((*setIt)->getSetIdentifier() == machineSetId){

				state = (*setIt)-> isON(machineIndex) ;
				found = true;

			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachineStateByIndex->Error. machineSetID %s does not exists", machineSetId.c_str());

	return state;

}

bool MachinesMap::isMachineON  (int machineSetIndex, int machineIndex){

	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		bool result;
		int machineMapSize;
	// Init ..
        machineMapSize = machineMap.size();
	// Begin ..

		if ((machineSetIndex < machineMapSize) && (machineSetIndex >= 0)){
			setIt = machineMap.begin()+machineSetIndex;
			result = (*setIt)->isON(machineIndex);
		}
		else {
			throw cRuntimeError("machinesMap::getmachineStateByIndex->Error. Index (%i) - machineMapIndex(%i)", machineSetIndex, machineMap.size());
		}

		return result;
}

int MachinesMap::countONMachines (string machineSetId){

	// Define ..

		int count;
		vector<HeterogeneousSet*>::iterator setIt;

	// Init ..

		count = -1;
		bool found = false;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if ((*setIt)->getSetIdentifier() == machineSetId){

				count = (*setIt)->countON() ;
				found = true;
			}

		}

		if (!found) throw cRuntimeError("machinesMap::countmachinesInStateByState->Error. machineSetID %s does not exists", machineSetId.c_str());


	return count;

}

int MachinesMap::countONMachines (int machineSetIndex){

	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		int result;
		int machineMapSize;
	// Init ..
		result = -1;
        machineMapSize = machineMap.size();
	// Begin ..

		if ((machineSetIndex < machineMapSize) && (machineSetIndex >= 0)){
			setIt = machineMap.begin()+machineSetIndex;
			result = (*setIt)->countON();
		}
		else {
			throw cRuntimeError("machinesMap::countmachinesInStateByState->Error. Index (%i) - machineMapIndex(%i)", machineSetIndex, machineMap.size());
		}

		return result;
}

int MachinesMap::countOFFMachines (string machineSetId){

    // Define ..

        int count;
        vector<HeterogeneousSet*>::iterator setIt;

    // Init ..

        count = -1;
        bool found = false;

    // Begin ..

        for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

            if ((*setIt)->getSetIdentifier() == machineSetId){

                count = (*setIt)->countOFF() ;
                found = true;
            }

        }

        if (!found) throw cRuntimeError("machinesMap::countmachinesInStateByState->Error. machineSetID %s does not exists", machineSetId.c_str());


    return count;

}

int MachinesMap::countOFFMachines (int machineSetIndex){

    // Define ..

        vector<HeterogeneousSet*>::iterator setIt;
        int result;
        int machineMapSize;
    // Init ..
        result = -1;
        machineMapSize = machineMap.size();
    // Begin ..

        if ((machineSetIndex < machineMapSize) && (machineSetIndex >= 0)){
            setIt = machineMap.begin()+machineSetIndex;
            result = (*setIt)->countOFF();
        }
        else {
            throw cRuntimeError("machinesMap::countmachinesInStateByState->Error. Index (%i) - machineMapIndex(%i)", machineSetIndex, machineMap.size());
        }

        return result;
}


Machine* MachinesMap::getMachineByIndex (string machineSetId, int machineIndex){

	// Define ..
		Machine* machine;
		vector<HeterogeneousSet*>::iterator setIt;
		bool found;

	// Init ..
		machine = nullptr;
		found = false;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if (strcmp ((*setIt)->getSetIdentifier().c_str(), machineSetId.c_str()) == 0){

				machine = (*setIt)->getMachineByIndex(machineIndex) ;
				found = true;

			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachineByIndex->Error. machineSetID %s does not exists", machineSetId.c_str());


	return machine;

}

Machine* MachinesMap::getMachineByIndex (int machineSetIndex, int machineIndex){

	// Define ..
		Machine* result;
		int machineMapSize;
	// Init ..
		result = nullptr;
        machineMapSize = machineMap.size();
	// Begin ..

		if ((machineSetIndex < machineMapSize) && (machineSetIndex >= 0)){
			result = (*(machineMap.begin()+machineSetIndex))->getMachineByIndex(machineIndex);
		}
		else {
			throw cRuntimeError("machinesMap::getmachineByIndex->Error. Index (%i) - machineMapIndex(%i)", machineSetIndex, machineMap.size());
		}
		return result;

}

Machine* MachinesMap::getMachineById(int id){

    // Define ..
        Machine* machine;
        bool found;

    // Init ..
        machine = nullptr;
        found = false;

    // Begin ..

        for (int i = 0; (i < getMapQuantity()) && (!found); i++){
            for (int j = 0; (j < getSetQuantity(i)) && (!found); j++){

                machine = getMachineByIndex(i,j);

                if (machine->getId() == id)
                     found = true;
            }
        }

        if (!found) throw cRuntimeError("machinesMap::getMachineById->Error. machine does not exists");

    return machine;

}

vector<Machine*> MachinesMap::getOFFMachines (string machineSetId){
	// Define ..

		vector<Machine*> machines;
		vector<HeterogeneousSet*>::iterator setIt;

	// Init ..

		machines.clear();
		bool found = false;

	// Begin ..

		for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

			if ((*setIt)->getSetIdentifier() == machineSetId){

				machines = (*setIt)->getOFFMachines();
				found = true;

			}

		}

		if (!found) throw cRuntimeError("machinesMap::getmachinesByState->Error. machineSetID %s does not exists", machineSetId.c_str());

		return machines;

}

vector<Machine*> MachinesMap::getOFFMachines (int machineSetIndex){
	// Define ..

		vector<HeterogeneousSet*>::iterator setIt;
		vector<Machine*> result;
		int machineMapSize;
	// Init ..
		result.clear();
        machineMapSize = machineMap.size();
	// Begin ..

		if ((machineSetIndex < machineMapSize) && (machineSetIndex >= 0)){
			setIt = machineMap.begin()+machineSetIndex;
			result = (*setIt)-> getOFFMachines();
		}
		else {
			throw cRuntimeError("machinesMap::getmachinesByState->Error. Index (%i) - machineMapIndex(%i)", machineSetIndex, machineMap.size());
		}

		return result;
}

vector<Machine*> MachinesMap::getONMachines (string machineSetId){
    // Define ..

        vector<Machine*> machines;
        vector<HeterogeneousSet*>::iterator setIt;

    // Init ..

        machines.clear();
        bool found = false;

    // Begin ..

        for (setIt = machineMap.begin(); (setIt < machineMap.end()) && (!found); setIt++){

            if ((*setIt)->getSetIdentifier() == machineSetId){

                machines = (*setIt)->getONMachines();
                found = true;

            }

        }

        if (!found) throw cRuntimeError("machinesMap::getmachinesByState->Error. machineSetID %s does not exists", machineSetId.c_str());

        return machines;

}

vector<Machine*> MachinesMap::getONMachines (int machineSetIndex){
    // Define ..

        vector<HeterogeneousSet*>::iterator setIt;
        vector<Machine*> result;
        int machineMapSize;
    // Init ..
        result.clear();
        machineMapSize = machineMap.size();
    // Begin ..

        if ((machineSetIndex < machineMapSize) && (machineSetIndex >= 0)){
            setIt = machineMap.begin()+machineSetIndex;
            result = (*setIt)-> getONMachines();
        }
        else {
            throw cRuntimeError("machinesMap::getmachinesByState->Error. Index (%i) - machineMapIndex(%i)", machineSetIndex, machineMap.size());
        }

        return result;
}


unsigned int MachinesMap::size (){

	return machineMap.size();

}

Machine* MachinesMap::getMachineByIP (string ip){
    int i,j;
    int machineMapSize;
    bool found;
    Machine* machine;

    found = false;
    machineMapSize = getMapQuantity();

    for (i = 0; (i < machineMapSize) && (!found); i++){

        machineMapSize = getSetQuantity(i);

        for (j = 0; (j < machineMapSize) && (!found); j++){

            machine = getMachineByIndex(i,j);

            if (strcmp (machine -> getIP().c_str(), ip.c_str()) == 0){
                found = true;
            }
        }

    }

    if (!found) machine = nullptr;

    return machine;
}

void MachinesMap::setElementType (int index, elementType* element){
    machineMap[index]->setElementType(element);
};


} // namespace icancloud
} // namespace inet
