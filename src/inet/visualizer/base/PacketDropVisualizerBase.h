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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PACKETDROPVISUALIZERBASE_H
#define __INET_PACKETDROPVISUALIZERBASE_H

#include "inet/common/PatternMatcher.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API PacketDropVisualization {
      public:
        int moduleId;
        cPacket *packet;
        simtime_t dropSimulationTime;
        double dropAnimationTime;
        double dropRealTime;

      public:
        PacketDropVisualization(int moduleId, cPacket *packet, simtime_t dropSimulationTime, double dropAnimationTime, int dropRealTime);
        virtual ~PacketDropVisualization();
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    PatternMatcher packetNameMatcher;
    const char *icon = nullptr;
    cFigure::Color iconTintColor;
    double iconTintAmount = NaN;
    const char *fadeOutMode = nullptr;
    double fadeOutHalfLife = NaN;
    //@}

    std::vector<const PacketDropVisualization *> packetDropVisualizations;

  protected:
    virtual ~PacketDropVisualizerBase();

    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG) override;

    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const = 0;
    virtual const PacketDropVisualization *createPacketDropVisualization(cModule *module, cPacket *packet) const = 0;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PACKETDROPVISUALIZERBASE_H

