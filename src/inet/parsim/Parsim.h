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

#ifndef __INET_PARSIM_H
#define __INET_PARSIM_H

#include <functional>
#include "inet/common/INETDefs.h"

namespace inet {

typedef std::function<simtime_t (simtime_t limit)> tf;

class INET_API Parsim : public cSimpleModule
{
  protected:
    cMessage *timer = nullptr;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    // TODO: these should be virtual methods in cModule and cChannel?
    tf computeMinimumChannelDelayFunction(cGate *outputGate);
    tf computeEarliestModuleMethodCallFunction(cModule *module);
    tf computeEarliestModuleEventTimeFunction(cModule *module);
    tf computeEarliestOutputTimeFunction(cGate *outputGate);
    tf computeEarliestInputTimeFunction(cGate *inputGate);

    simtime_t getEarliestInputTime(cGate *inputGate);
};

} // namespace

#endif

