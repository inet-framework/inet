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

#include "User_VirtualPort_Cell.h"

namespace inet {

namespace icancloud {


User_VirtualPort_Cell::User_VirtualPort_Cell() {
	user = -1;
	vm_ports.clear();

}

User_VirtualPort_Cell::~User_VirtualPort_Cell() {
	// TODO Auto-generated destructor stub
}

void User_VirtualPort_Cell::setUserID (int userID){
	user = userID;
}

int User_VirtualPort_Cell::getUserID (){
	return user;
}

Vm_VirtualPort_Cell* User_VirtualPort_Cell::searchVM(int vmID){

	// Define ..
		bool found;
		unsigned int i;
		Vm_VirtualPort_Cell* vm_cell;

	// Init ..
		found = false;
		vm_cell = nullptr;

	for (i = 0; (i < vm_ports.size()) && (!found);){

		if (((*(vm_ports.begin()+i))->getVMID()) ==  vmID){

			found = true;
			vm_cell = (*(vm_ports.begin()+i));
		}else {
			i++;
		}
	}

	return vm_cell;

}

Vm_VirtualPort_Cell* User_VirtualPort_Cell::newVM(int vmID){

	Vm_VirtualPort_Cell* vm;

	vm = new Vm_VirtualPort_Cell();

	vm->setVMID(vmID);

	vm_ports.push_back(vm);

	return vm;

}

void User_VirtualPort_Cell::eraseVM(int vmID){

	// Define ..
		bool found;
		unsigned int i;
		vector<int> rPorts;

	// Init ..
		found = false;

	for (i = 0; (i < vm_ports.size()) && (!found);){

		if (((*(vm_ports.begin()+i))->getVMID()) == vmID){
			found = true;
			vm_ports.erase(vm_ports.begin()+i);
		}else {
			i++;
		}
	}

}

int User_VirtualPort_Cell::getVM_Size(){
	return vm_ports.size();
}

} // namespace icancloud
} // namespace inet
