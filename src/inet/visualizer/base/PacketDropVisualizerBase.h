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

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/PacketFilter.h"

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
    bool displayPacketDrops = false;
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
    PacketFilter packetFilter;
    const char *icon = nullptr;
    cFigure::Color iconTintColor;
    double iconTintAmount = NaN;
    cFigure::Font labelFont;
    cFigure::Color labelColor;
    const char *fadeOutMode = nullptr;
    double fadeOutTime = NaN;
    double fadeOutAnimationSpeed = NaN;
    //@}

    std::vector<const PacketDropVisualization *> packetDropVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const PacketDropVisualization *createPacketDropVisualization(cModule *module, cPacket *packet) const = 0;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization);
    virtual void removeAllPacketDropVisualizations();
    virtual void setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const = 0;

  public:
    virtual ~PacketDropVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PACKETDROPVISUALIZERBASE_H

