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

#include "inet/icancloud/Management/DataCenterManagement/Base/DataCenterAPI.h"

namespace inet {

namespace icancloud {



DataCenterAPI::~DataCenterAPI() {
    // TODO Auto-generated destructor stub
}


// ---------------------- Scheduling interaction with nodeMap  ------------------------------------

int DataCenterAPI::getDifferentTypesSize(bool storage){

    int result;

    if (storage){
        result = storage_nodesMap->getMapQuantity();
    }else{
        result = nodesMap->getMapQuantity();
    }

    return result;

}


int DataCenterAPI::getSetSize(bool storage, string setId){

    int result;

    if (storage){
        result = storage_nodesMap->getSetQuantity(setId);
    }else{
        result = nodesMap->getSetQuantity(setId);
    }

    return result;

}

int DataCenterAPI::getSetSize(bool storage, int setIndex){

    int result;

    if (storage){
        result = storage_nodesMap->getSetQuantity(setIndex);
    }else{
        result = nodesMap->getSetQuantity(setIndex);
    }

    return result;

}

int DataCenterAPI::getSetPosition (string setId, bool storage){


    int result;

    if (storage)
        result = storage_nodesMap->getSetIndex(setId);
    else
        result = nodesMap->getSetIndex(setId);

    return result;
}

string DataCenterAPI::getSetName (int index, bool storage){

    string result;

    if (storage)
        result = storage_nodesMap->getSetIdentifier(index);
    else
        result = nodesMap->getSetIdentifier(index);

    return result;

}

int DataCenterAPI::getSetSize (string setId, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetQuantity(setId);
    else
        result = nodesMap->getSetQuantity(setId);

    return result;
}

int DataCenterAPI::getSetSize (int nodeIndex, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetQuantity(nodeIndex);
    else
        result = nodesMap->getSetQuantity(nodeIndex);

    return result;
}

int DataCenterAPI::getSetNumCores(string setId, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetNumberOfCPUs(setId);
    else
        result = nodesMap->getSetNumberOfCPUs(setId);

    return result;
}

int DataCenterAPI::getSetNumCores(int nodeIndex, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetNumberOfCPUs(nodeIndex);
    else
        result = nodesMap->getSetNumberOfCPUs(nodeIndex);

    return result;
}

int DataCenterAPI::getSetMemorySize(string setId, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetTotalMemory(setId);
    else
        result = nodesMap->getSetTotalMemory(setId);

    return result;
}

int DataCenterAPI::getSetMemorySize(int nodeIndex, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetTotalMemory(nodeIndex);
    else
        result = nodesMap->getSetTotalMemory(nodeIndex);

    return result;
}

int DataCenterAPI::getSetStorageSize(string setId, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetTotalStorage(setId);
    else
        result = nodesMap->getSetTotalStorage(setId);

    return result;
}

int DataCenterAPI::getSetStorageSize(int nodeIndex, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->getSetTotalMemory(nodeIndex);
    else
        result = nodesMap->getSetTotalMemory(nodeIndex);

    return result;
}


string DataCenterAPI::getNodeState(string setId, int nodeIndex, bool storage){
    string result;

    if (storage)
        result = storage_nodesMap->isMachineON(setId.c_str(), nodeIndex);
    else
        result = nodesMap->isMachineON(setId.c_str(), nodeIndex);

    return result;
}

string DataCenterAPI::getNodeState  (int setIndex, int nodeIndex, bool storage){
    string result;

    if (storage)
        result = storage_nodesMap->isMachineON(setIndex, nodeIndex);
    else
        result = nodesMap->isMachineON(setIndex, nodeIndex);

    return result;
}

int DataCenterAPI::countONNodes (string setId, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->countONMachines(setId);
    else
        result = nodesMap->countONMachines(setId);

    return result;
}

int DataCenterAPI::countONNodes (int setIndex, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->countONMachines(setIndex);
    else
        result = nodesMap->countONMachines(setIndex);

    return result;
}

int DataCenterAPI::countOFFNodes (string setId, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->countOFFMachines(setId);
    else
        result = nodesMap->countOFFMachines(setId);

    return result;
}

int DataCenterAPI::countOFFNodes (int setIndex, bool storage){
    int result;

    if (storage)
        result = storage_nodesMap->countOFFMachines(setIndex);
    else
        result = nodesMap->countOFFMachines(setIndex);

    return result;
}

// ------------------ To obtain nodes from the set -------------------------

AbstractNode* DataCenterAPI::getNodeByIndex (string setId, int nodeIndex, bool storage){
    Machine* result;
    AbstractNode* node;

    if (storage)
        result = storage_nodesMap->getMachineByIndex(setId, nodeIndex);
    else
        result = nodesMap->getMachineByIndex(setId, nodeIndex);

    node = dynamic_cast<AbstractNode*>(result);

    return node;
}

AbstractNode* DataCenterAPI::getNodeByIndex (int setIndex, int nodeIndex, bool storage){
    Machine* result;
    AbstractNode* node;

    if (storage)
        result = storage_nodesMap->getMachineByIndex(setIndex, nodeIndex);
    else
        result = nodesMap->getMachineByIndex(setIndex, nodeIndex);

    node = dynamic_cast<AbstractNode*>(result);

    return node;
}

string DataCenterAPI::getNodeIp (string setIndex, int nodeIndex, bool storage){
    string result;

    if (storage)
        result = storage_nodesMap->getMachineByIndex(setIndex, nodeIndex)->getIP().c_str();
    else
        result = nodesMap->getMachineByIndex(setIndex, nodeIndex)->getIP().c_str();

    return result;
}

vector<AbstractNode*> DataCenterAPI::getONNodes (string setId, bool storage){
    vector<AbstractNode*> nodes;
    AbstractNode* node;
    vector<Machine*> result;

    if (storage)
        result = storage_nodesMap->getONMachines(setId.c_str());
    else
        result = nodesMap->getONMachines(setId.c_str());

    nodes.clear();
    for (int i =0; i < (int)result.size();i++){
        node = dynamic_cast<AbstractNode*>(*(result.begin() + i));
        nodes.push_back(node);
    }

    return nodes;
}

vector<AbstractNode*> DataCenterAPI::getONNodes (int setIndex, bool storage){
    vector<AbstractNode*> nodes;
    AbstractNode* node;
    vector<Machine*> result;

    if (storage)
        result = storage_nodesMap->getONMachines(setIndex);
    else
        result = nodesMap->getONMachines(setIndex);

    nodes.clear();
    for (int i =0; i < (int)result.size();i++){
        node = dynamic_cast<AbstractNode*>(*(result.begin() + i));
        nodes.push_back(node);
    }

    return nodes;
}

vector<AbstractNode*> DataCenterAPI::getOFFNodes (string setId, bool storage){
    vector<AbstractNode*> nodes;
    AbstractNode* node;
    vector<Machine*> result;

    if (storage)
        result = storage_nodesMap->getOFFMachines(setId.c_str());
    else
        result = nodesMap->getOFFMachines(setId.c_str());

    nodes.clear();
    for (int i =0; i < (int)result.size();i++){
        node = dynamic_cast<AbstractNode*>(*(result.begin() + i));
        nodes.push_back(node);
    }

    return nodes;
}

vector<AbstractNode*> DataCenterAPI::getOFFNodes (int setIndex, bool storage){
    vector<AbstractNode*> nodes;
    AbstractNode* node;
    vector<Machine*> result;

    if (storage)
        result = storage_nodesMap->getOFFMachines(setIndex);
    else
        result = nodesMap->getOFFMachines(setIndex);

    nodes.clear();
    for (int i =0; i < (int)result.size();i++){
        node = dynamic_cast<AbstractNode*>(*(result.begin() + i));
        nodes.push_back(node);
    }

    return nodes;
}

AbstractNode* DataCenterAPI::getNodeByIP (string ip){
    Machine* result;
    AbstractNode* node;

    result = storage_nodesMap->getMachineByIP(ip);
    if (result == nullptr) result = nodesMap->getMachineByIP(ip);

    node = dynamic_cast<AbstractNode*>(result);

    return node;
}

elementType* DataCenterAPI::getElementType (int index, bool storageNodes){
    elementType* result;

    if (storageNodes)
       result = storage_nodesMap->getElementType(index);
    else
       result = nodesMap->getElementType(index);

    return result;
}

void DataCenterAPI::setElementType(elementType* element, int index, bool storageNodes){

    if (storageNodes)
        storage_nodesMap->setElementType(index, element);
    else
        nodesMap->setElementType(index, element);

}

int DataCenterAPI::getNumberOfProcesses(int nodeSet, int nodeID, bool storageNodes){
    return nodesMap->getMachineByIndex(nodeSet,nodeID)->getNumProcessesRunning();
}

void DataCenterAPI::printNodeMapInfo(){
    printf ("------------computeNodes ---------------\n");
    for (int i = 0; i < getMapSize(); i++){
        for (int j = 0; j < getSetSize(i,false); j++){
            printf("NodeIP:%s\n", getNodeByIndex(i,j, false)->getIP().c_str());
        }
    }

    printf ("------------storageNodes ---------------\n");
    for (int i = 0; i < getMapSize(); i++){
        for (int j = 0; j < getSetSize(i,true); j++){
            printf("NodeIP:%s\n", getNodeByIndex(i,j, true)->getIP().c_str());
        }
    }
    printf ("--------------------------------------------");
}

} // namespace icancloud
} // namespace inet
