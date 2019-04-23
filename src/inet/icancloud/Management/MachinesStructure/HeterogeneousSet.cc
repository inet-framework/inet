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

#include "inet/icancloud/Management/MachinesStructure/HeterogeneousSet.h"

namespace inet {

namespace icancloud {


HeterogeneousSet::HeterogeneousSet() {
	machinesSet.clear();
	type = nullptr;
}


HeterogeneousSet::~HeterogeneousSet() {

    while(machinesSet.size() != 0){
        machinesSet.erase(machinesSet.begin());
    }
    machinesSet.clear();
    delete(type);
}


void HeterogeneousSet::initMachine (Machine* machine){
       machinesSet.push_back(machine);
}

Machine* HeterogeneousSet::getMachineByIndex (int index){

		return (*(machinesSet.begin()+index));
}

vector<Machine*> HeterogeneousSet::getOFFMachines (){

	// Define ..

		vector<Machine*> machines;
		vector<Machine*>::iterator setIt;

	// Init ..

		machines.clear();

	// Begin ..

		for (setIt = machinesSet.begin(); setIt < machinesSet.end(); setIt++){
		   if (!((*setIt)->isON())) machines.insert(machines.end(), (*setIt));
		}

	return machines;
}

vector<Machine*> HeterogeneousSet::getONMachines (){

    // Define ..

        vector<Machine*> machines;
        vector<Machine*>::iterator setIt;

    // Init ..

        machines.clear();

    // Begin ..

        for (setIt = machinesSet.begin(); setIt < machinesSet.end(); setIt++){
           if ((*setIt)->isON()) machines.insert(machines.end(), (*setIt));
        }

    return machines;

}


bool HeterogeneousSet::isON (int index){

		return (*(machinesSet.begin()+index))->isON();

}

int HeterogeneousSet::countOFF (){

    // Define ..

        vector<Machine*> machines;
        vector<Machine*>::iterator setIt;

    // Init ..

        machines.clear();

    // Begin ..

        for (setIt = machinesSet.begin(); setIt < machinesSet.end(); setIt++){
           if (!((*setIt)->isON())) machines.insert(machines.end(), (*setIt));
        }

    return machines.size();
}

unsigned int HeterogeneousSet::size (){
    return machinesSet.size();
}

int HeterogeneousSet::countON (){

    // Define ..

        vector<Machine*> machines;
        vector<Machine*>::iterator setIt;

    // Init ..

        machines.clear();

    // Begin ..

        for (setIt = machinesSet.begin(); setIt < machinesSet.end(); setIt++){
           if ((*setIt)->isON()) machines.insert(machines.end(), (*setIt));
        }

    return machines.size();

}

void HeterogeneousSet::deleteMachine(int pId){

    // Define ..

        vector<Machine*>::iterator setIt;
        bool found = false;
    // Begin ..

        for (setIt = machinesSet.begin(); (setIt < machinesSet.end()) && (!found); setIt++){
           if ((*setIt)->getId() == pId){
               machinesSet.erase(setIt);
               found = true;
           }
        }

        if (!found) throw cRuntimeError("HeterogeneousSet::deleteMachine-> Error deleting machine pId = %i",pId);

};

} // namespace icancloud
} // namespace inet
