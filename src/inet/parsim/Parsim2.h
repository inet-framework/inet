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

#ifndef __INET_PARSIM2_H
#define __INET_PARSIM2_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API Parsim2 : public cSimpleModule
{
  protected:
    class INET_API Entry {
      public:
        simtime_t delay;

      public:
        Entry(simtime_t delay) : delay(delay) { }
    };

    class INET_API ModuleEntry : public Entry {
      public:
        cModule *source;
        cModule *destination;

      public:
        ModuleEntry(cModule *source, cModule *destination, simtime_t delay) : Entry(delay), source(source), destination(destination) { }
    };

    class INET_API ModuleVectorEntry : public Entry {
      public:
        std::vector<cModule *> sources;
        std::vector<cModule *> destinations;

      public:
        ModuleVectorEntry(simtime_t delay) : Entry(delay) { }
    };

    class INET_API ModuleTreeEntry : public Entry {
      public:
        cModule *source;
        cModule *destination;

      public:
        ModuleTreeEntry(cModule *source, cModule *destination, simtime_t delay) : Entry(delay), source(source), destination(destination) { }
    };

  protected:
    cMessage *timer = nullptr;

    std::vector<ModuleEntry *> entries;
    std::vector<ModuleTreeEntry *> optimizedEntries;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void collectEntries();

    virtual void optimizeEntries();
    virtual void verifyOptimizedEntries();

    virtual void printEntries();
    virtual void printOptimizedEntries();

    virtual void calculateEarliestInputTimes();
};

} // namespace

#endif

