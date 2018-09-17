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

#ifndef __INET_MOBILITYOSGVISUALIZER_H
#define __INET_MOBILITYOSGVISUALIZER_H

#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/MobilityVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualization.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API MobilityOsgVisualizer : public MobilityVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API MobilityOsgVisualization : public MobilityVisualization {
      public:
        osg::Geode *trail;

      public:
        MobilityOsgVisualization(osg::Geode *trail, IMobility *mobility);
    };

  protected:
    std::map<const IMobility *, MobilityOsgVisualization *> mobilityVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual MobilityOsgVisualization *getMobilityVisualization(const IMobility *mobility) const;
    virtual void setMobilityVisualization(const IMobility *mobility, MobilityOsgVisualization *entry);
    virtual void removeMobilityVisualization(const IMobility *mobility);
    virtual MobilityOsgVisualization* ensureMobilityVisualization(IMobility *mobility);

    virtual void extendMovementTrail(osg::Geode *trail, const Coord& position) const;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MOBILITYOSGVISUALIZER_H

