//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_LINKTRACKERBASE_H
#define __INET_LINKTRACKERBASE_H

#include <fstream>

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/StringFormat.h"
#include "inet/tracker/base/TrackerBase.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace tracker {

using namespace inet::visualizer;

class INET_API LinkTrackerBase : public TrackerBase, public cListener
{
  protected:
    enum ActivityLevel {
        ACTIVITY_LEVEL_SERVICE,
        ACTIVITY_LEVEL_PEER,
        ACTIVITY_LEVEL_PROTOCOL,
    };

  protected:
    /** @name Parameters */
    //@{
    std::ofstream file;
    bool trackLinks = false;
    ActivityLevel activityLevel = static_cast<ActivityLevel>(-1);
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
    PacketFilter packetFilter;
    //@}

    /**
     * Maps packet to last module.
     */
    std::map<int, int> lastModules;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual bool isLinkStart(cModule *module) const = 0;
    virtual bool isLinkEnd(cModule *module) const = 0;

    virtual cModule *getLastModule(int treeId);
    virtual void setLastModule(int treeId, cModule *lastModule);
    virtual void removeLastModule(int treeId);

    virtual const char *getTags() const = 0;

    virtual void trackPacketSend(Packet *packet, cModule *senderNetworkNode, cModule *senderNetworkInterface, cModule *senderModule, cModule *receiverNetworkNode, cModule *receiverNetworkInterface, cModule *receiverModule);

  public:
    virtual ~LinkTrackerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace tracker

} // namespace inet

#endif

