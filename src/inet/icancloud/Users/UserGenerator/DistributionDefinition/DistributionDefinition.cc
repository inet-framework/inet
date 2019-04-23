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

#include "inet/icancloud/Users/UserGenerator/DistributionDefinition/DistributionDefinition.h"

namespace inet {

namespace icancloud {


Define_Module(DistributionDefinition);

DistributionDefinition::~DistributionDefinition() {
    // TODO Auto-generated destructor stub
}

void DistributionDefinition::initialize(){

}

void DistributionDefinition::handleMessage(cMessage* msg){
    throw cRuntimeError ("DistributionDefinition::handleMessage->this module does not receive messages\n");
}

} // namespace icancloud
} // namespace inet
