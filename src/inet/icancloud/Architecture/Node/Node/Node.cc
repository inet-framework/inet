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

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "inet/icancloud/Architecture/Node/Node/Node.h"

namespace inet {

namespace icancloud {


Define_Module(Node);

void Node::initialize(int stage){

    AbstractNode::initialize(stage);

    // Initialize the state
    if (stage == INITSTAGE_LOCAL)
        energyMeterPtr = nullptr;

}

void Node::finish(){
    AbstractNode::finish();
}

//----------------------------------------------------------------------------------

void Node::turnOn (){

    if (equalStates(getState(),MACHINE_STATE_OFF)){

            AbstractNode::changeState(MACHINE_STATE_IDLE);
            energyMeterPtr->switchOnPSU();
    }

}

void Node::turnOff (){

    if (!equalStates(getState(), MACHINE_STATE_OFF)){
        AbstractNode::changeState(MACHINE_STATE_OFF);
        energyMeterPtr->switchOffPSU();
    }
}

void Node::initNode (){

    //IRoutingTable* rTable;
    string ipNode;
    string state;
    // Init ..

    try{

        //get the ip of the Node
        auto node = getContainingNode(this);


        auto rTable = (L3AddressResolver().getIpv4RoutingTableOf(node));
        if (rTable == nullptr)
            throw cRuntimeError("BaseNode::initNode -> The node %s[%i] has no assigned ip. Check if everything is correctly configured..\n", getFullName(), getIndex());

        //auto rTable =  dynamic_cast <inet::IRoutingTable*> (getSubmodule("routingTable"))



        auto address = L3AddressResolver().addressOf(node);
        ipNode = address.toIpv4().str(false);

        if (address.isUnspecified())
            throw cRuntimeError("");


        ip = ipNode.c_str();

        par("ip").setStringValue(ip.c_str());

        // Init the parameters
        storageNode = par("storageNode").boolValue();
        state = par("initialState").stringValue();
        storageLocalPort = par("storage_local_port").intValue();

        cModule* mod = getSubmodule("energyMeter")->getSubmodule("meterController");

        energyMeterPtr = check_and_cast <EnergyMeterController*> (mod);
        energyMeterPtr->init();
        energyMeterPtr->registerMemorization(getParentModule()->getSubmodule("manager")->par("memorization").boolValue());
        energyMeterPtr->activateMeters();

        // initialize system apps
        int port;
         port = storageLocalPort;
        initialize_syscallManager(port);

         if (equalStates(getState(),MACHINE_STATE_OFF)) turnOff();
         else if (!equalStates(getState(),MACHINE_STATE_OFF)) turnOn();
         else throw cRuntimeError("Node::initNode ->initializing state unknown [%s]\n", state.c_str());

    }catch (exception& e){
        throw cRuntimeError("Node::initNode -> can not initialize the node module!...");
    }

}

void Node::notifyManager (Packet *pkt){
    AbstractDCManager* manager;

    auto sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError ("Packet doesn0t contain icancloud_Message header");

    manager = dynamic_cast <AbstractDCManager*> (managerPtr);
    if (manager == nullptr) throw cRuntimeError ("Node::notifyVMConnectionsClosed -> Manager can not be casted\n");

    manager->notifyManager(pkt);
};



void Node::setManager(icancloud_Base* manager){

    managerPtr = dynamic_cast <AbstractDCManager*> (manager);
    if (managerPtr == nullptr) throw cRuntimeError ("Node::notifyVMConnectionsClosed -> Manager can not be casted\n");
};





} // namespace icancloud
} // namespace inet
