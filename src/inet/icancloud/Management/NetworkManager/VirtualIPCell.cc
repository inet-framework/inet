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

#include "VirtualIPCell.h"

namespace inet {

namespace icancloud {


VirtualIPCell::VirtualIPCell() {
	// TODO Auto-generated constructor stub
	virtualIP = new VirtualIPs();
	ipNode = "";
	vmID = -1;
}

VirtualIPCell::~VirtualIPCell() {

    delete(virtualIP);
}


void VirtualIPCell::setVirtualIP(string newVirtualIP){
	virtualIP->setVirtualIP(newVirtualIP);
}

string VirtualIPCell::getVirtualIP(){
	return virtualIP->getVirtualIP().c_str();
}

void VirtualIPCell::setIPNode(string id){
	ipNode = id;
}

string VirtualIPCell::getIPNode(){
	return ipNode.c_str();
}

void VirtualIPCell::setVMID(int vm_id){
	vmID = vm_id;
}

int VirtualIPCell::getVMID(){
	return vmID;
}

string VirtualIPCell::getHole(string ipBasis){
	return virtualIP->generateHole(ipBasis);
}

} // namespace icancloud
} // namespace inet
