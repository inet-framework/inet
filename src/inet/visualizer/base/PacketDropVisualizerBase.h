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
#include "inet/visualizer/common/AnimationPosition.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API PacketDropVisualization {
      public:
        mutable AnimationPosition packetDropAnimationPosition;
        const int moduleId = -1;
        const cPacket *packet = nullptr;
        const Coord position;

      public:
        PacketDropVisualization(int moduleId, const cPacket *packet, const Coord& position);
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
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const = 0;
    virtual const PacketDropVisualization *createPacketDropVisualization(cModule *module, cPacket *packet) const = 0;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PACKETDROPVISUALIZERBASE_H

