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

#ifndef __INET4_CHAOSMANAGER_H_
#define __INET4_CHAOSMANAGER_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include <list>
#include <vector>
#include <algorithm>
#include <string>
#include "inet/common/lifecycle/LifecycleController.h"

using namespace omnetpp;

namespace inet {

/**
 * TODO - Generated class
 */
class INET_API ChaosManager : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void generateChaos(int faultPerc, int numNodes);
    void processCommand(int command, int nodeNum);
    bool eventExist(int nodeId);
    int getUniqueNodeId(int numNodes);

    int numNodes = 0;
    int faultPerc = 0;

    LifecycleController lifecycleController;

};

} //namespace

#endif
