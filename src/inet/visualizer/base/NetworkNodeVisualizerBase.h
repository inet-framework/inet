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

#ifndef __INET_NETWORKNODEVISUALIZERBASE_H
#define __INET_NETWORKNODEVISUALIZERBASE_H

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeVisualizerBase : public VisualizerBase, public cListener
{
  public:
    class INET_API NetworkNodeVisualization
    {
      public:
        const cModule *networkNode;

      public:
        NetworkNodeVisualization(const cModule *networkNode) : networkNode(networkNode) { }
        virtual ~NetworkNodeVisualization() { }
    };

  protected:
    NetworkNodeFilter nodeFilter;
    double annotationSpacing;
    double placementPenalty;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual NetworkNodeVisualization *createNetworkNodeVisualization(cModule *networkNode) const = 0;
    virtual void addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) = 0;
    virtual void removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) = 0;

  public:
    virtual NetworkNodeVisualization *getNetworkNodeVisualization(const cModule *networkNode) const = 0;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_NETWORKNODEVISUALIZERBASE_H

