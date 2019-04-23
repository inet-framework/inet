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

#include "inet/icancloud/Architecture/Node/AbstractNode.h"

namespace inet {

namespace icancloud {


AbstractNode::~AbstractNode() {
}

void AbstractNode::initialize(int stage){

    Machine::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        os = dynamic_cast<SyscallManager*>(os);
        storageLocalPort = -1;
    }
}

void AbstractNode::finish(){

    Machine::finish();
}

void AbstractNode::initialize_syscallManager(int localPort){
    SyscallManager* sc;
    storageLocalPort = localPort;
    sc = check_and_cast<SyscallManager*>(os);
    sc -> initializeSystemApps(storageLocalPort, INITIAL_STATE);
    sc -> setManager(this);
}

void AbstractNode::closeConnections (int uId, int pId){
    SyscallManager* sc;
    sc = check_and_cast<SyscallManager*>(os);
    sc ->closeConnection(uId, pId);
}

} // namespace icancloud
} // namespace inet
