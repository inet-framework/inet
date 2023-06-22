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

#ifndef __INET_PATHTRACKERBASE_H
#define __INET_PATHTRACKERBASE_H

#include <fstream>

#include "inet/common/packet/PacketFilter.h"
#include "inet/tracker/base/TrackerBase.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace tracker {

using namespace inet::visualizer;

class INET_API PathTrackerBase : public TrackerBase, public cListener
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
    bool trackPaths = false;
    ActivityLevel activityLevel = static_cast<ActivityLevel>(-1);
    NetworkNodeFilter nodeFilter;
    PacketFilter packetFilter;
    //@}

    /**
     * Maps packet to module vector.
     */
    std::map<int, std::vector<int>> incompletePaths;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual bool isPathStart(cModule *module) const = 0;
    virtual bool isPathEnd(cModule *module) const = 0;
    virtual bool isPathElement(cModule *module) const = 0;

    virtual const std::vector<int> *getIncompletePath(int chunkId);
    virtual void addToIncompletePath(int chunkId, cModule *module);
    virtual void removeIncompletePath(int chunkId);

    virtual const char *getTags() const = 0;

    virtual void trackPacketSend(Packet *packet, cModule *senderNetworkNode, cModule *senderModule, cModule *receiverNetworkNode, cModule *receiverModule, const std::vector<int> *path);

  public:
    virtual ~PathTrackerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace tracker

} // namespace inet

#endif // ifndef __INET_PATHTRACKERBASE_H

