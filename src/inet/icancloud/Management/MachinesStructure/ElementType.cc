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

#include "inet/icancloud/Management/MachinesStructure/ElementType.h"

namespace inet {

namespace icancloud {


elementType::elementType() {

}

void elementType::setTypeParameters (int numTotalCores, int memorySize, int storageSize, int numIfEth, string newNodeType, int numStorageDev){
	numCores = numTotalCores;
	memoryTotal = memorySize * 1024;
	storageTotal = storageSize * 1024 * 1024;
	numStorageDevices = numStorageDev;
	numNetIF = numIfEth;
	name = newNodeType;

}

elementType::~elementType() {

}


} // namespace icancloud
} // namespace inet
