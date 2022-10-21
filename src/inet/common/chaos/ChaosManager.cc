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

#include "ChaosManager.h"
#include <list>
#include <vector>
#include <algorithm>
#include <time.h>
#include <string>
#include <cstring>
#include <map>
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/chaos/ChaosEvent_m.h"
using namespace std;

namespace inet {

struct eventObject {
    int command;
    int time;
    int nodeNum;
};

vector<eventObject> eventList;

Define_Module(ChaosManager);

void ChaosManager::initialize()
{
    numNodes = par("numNodes");
    faultPerc = par("faultPerc");
    generateChaos(faultPerc, numNodes);


    // iterate through list and generate self messages
    for (auto &itr : eventList) {
        simtime_t t = itr.time;
        auto msg = new ChaosEvent("chaos-event");
        msg->setNodeNum(itr.nodeNum);
        msg->setCommand(itr.command);
        scheduleAt(t, msg);
    }
}

void ChaosManager::handleMessage(cMessage *msg)
{
    int retrievedCommand = check_and_cast<ChaosEvent *>(msg)->getCommand();
    int retrievedNodeNum = check_and_cast<ChaosEvent *>(msg)->getNodeNum();
    delete msg;

    // processCommands as they come in
    processCommand(retrievedCommand, retrievedNodeNum);

}

void ChaosManager::generateChaos(int faultPerc, int numNodes) {
    srand(time(0));
    int faultyNodes = numNodes * faultPerc / 100;
    for (int counter = 0; counter < faultyNodes; counter++) {
        eventObject event;
        event.nodeNum = getUniqueNodeId(numNodes);
        event.time = rand()%getResolvedSimTimeLimit();; // random value 50 assigned
        event.command = 1 + (rand()%2);
        eventList.push_back(event);
    }
}

bool ChaosManager::eventExist(int nodeId) {
    for (auto& it : eventList) {
        if (it.nodeNum == nodeId) {
            return true;
        }
    }
    return false;
}

int ChaosManager::getUniqueNodeId(int numNodes) {
    int uniqueNodeId;
    for (int counter = 0; counter <= numNodes; counter++) {
        srand((unsigned) time(NULL));
        uniqueNodeId = rand() % (numNodes + 1);
        if (eventExist(uniqueNodeId)) {
            continue;
        } else {
            break;
        }
    }
    return uniqueNodeId;
}

void ChaosManager::processCommand(int command, int nodeNum) {
    // resolve target node
    string targetString = "host[" + std::to_string(nodeNum) + "]";
    const char *target = &targetString[0];
    cModule *module = getModuleByPath(target);
    if (!module)
            throw cRuntimeError("Module '%s' not found", target);

    // resolve command
    LifecycleOperation *operation;
    if (nodeNum == 1) {
        operation = new ModuleStopOperation;
    } else {
        operation = new ModuleCrashOperation;
    }
    map<string, string> params; // empty string to fill parameters
    operation->initialize(module, params);

    // perform operation
    lifecycleController.initiateOperation(operation);

}

int ChaosManager::getResolvedSimTimeLimit() {
    int timeLimit = 0;
    cConfiguration *config = getEnvir()->getConfig();
    string simTimeLimit = config->getConfigValue("sim-time-limit");
    string simTimeLimitResolved = std::regex_replace(simTimeLimit,std::regex("[^0-9]*([0-9]+).*"),
            std::string("$1"));
    timeLimit = std::stoi(simTimeLimitResolved);
    return timeLimit;
}

} //namespace

