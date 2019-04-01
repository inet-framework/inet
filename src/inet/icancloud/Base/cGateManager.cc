/*
 * gateData.cc
 *
 *  Created on: 22/10/2014
 *      Author: gabriel
 */

#include "inet/icancloud/Base/cGateManager.h"

namespace inet {

namespace icancloud {


cGateManager::cGateManager(cModule* module) {
    mod = module;
    holes.clear();
    gates.clear();
}

cGateManager::~cGateManager() {
    holes.clear();
}

int cGateManager::isGateEmpty(int index){
       return (!(*(gates.begin() + index))->gateHit);
}

void cGateManager::linkGate(string gateName, int index){


        // If the index requested to link is 2 positions less than size
        if ((int)gates.size() < index)
            throw cRuntimeError("gateData::linkGate->gate position %i is greater than the gates size\n", index);

        // The link is a new Gate. The vector has to be size increased
        else if ((int)gates.size() == index){

            gateStructure* gateNew;
            gateNew = new gateStructure();
            gateNew->gate = mod->gate(gateName.c_str(), index);
            gateNew->gateHit = true;
            gates.push_back(gateNew);
        }

        // Error. There is a gate at requested position
        else if  ((*(gates.begin() + index))->gateHit){

            throw cRuntimeError("gateData::linkGate->gate position %i has a linked gate\n", index);

        // The link is for a hole
        } else {
            (*(gates.begin() + index))->gate  = mod->gate(gateName.c_str(), index);
            (*(gates.begin() + index))->gateHit = true;
        }

}

int cGateManager::newGate(string gateName){

    int gatePosition;

    // If there is no gate
    if (gates.size() == 0){
        mod->setGateSize(gateName.c_str(), 1);
        gatePosition = 0;

    } else {
        // There is a free gate
        if (holes.size()>0){
            gatePosition = (*(holes.begin()));
            holes.erase(holes.begin());
        }
        // Last position
        else {
            gatePosition = gates.size();
            mod->setGateSize(gateName.c_str(), gates.size() + 1);
        }
    }

    linkGate(gateName.c_str(), gatePosition);

    return  gatePosition;
}

cGate* cGateManager::getGate(int index){

    cGate* gate = nullptr;
    if (!(*(gates.begin() + index))->gateHit){
        cRuntimeError("Error at position [%i] ->gateData::getGate\n", index);
    } else {
        gate = (*(gates.begin() + index))->gate;
    }
    return gate;
}

cGate* cGateManager::freeGate(int index){
    cGate* gate = nullptr;
    gateStructure* gateSt;

    gateSt = (*(gates.begin() + index));

    if (!gateSt->gateHit){
        throw cRuntimeError("Error at position [%i] ->gateData::freeGate\n", index);
    } else {
        gateSt->gateHit = false;
        gate = gateSt->gate;
        gate -> disconnect();
        holes.push_back(index);

    }
    return gate;
}

void cGateManager::connectIn(cGate* gate, int index){
    cGate* internalGate;
    internalGate = (*(gates.begin()+index))->gate;
    gate->connectTo(internalGate);
}

void cGateManager::connectOut(cGate* gate, int index){
    cGate* internalGate;
    internalGate = (*(gates.begin()+index))->gate;
    internalGate->connectTo(gate);
}

int cGateManager::searchGate(int gateId){
    bool found = false;
    int position = -1;

    for (int i = 0; (i < (int)gates.size()) && (!found); i++){
        if ((*(gates.begin() + i))->gate->getId() == gateId){
            found = true;
            position = i;
        }
    }

    if (position == -1) throw cRuntimeError("cGateManager::searchGate->gate index not found\n");

    return position;
}


} // namespace icancloud
} // namespace inet
