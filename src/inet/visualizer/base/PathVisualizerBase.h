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

#ifndef __INET_PATHVISUALIZERBASE_H
#define __INET_PATHVISUALIZERBASE_H

#include "inet/common/StringFormat.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationPosition.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

// TODO: move to some utility file
inline bool isEmpty(const char *s) { return !s || !s[0]; }
inline bool isNotEmpty(const char *s) { return s && s[0]; }

class INET_API PathVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API PathVisualization : public LineManager::ModulePath {
      public:
        mutable AnimationPosition lastUsageAnimationPosition;

      public:
        PathVisualization(const std::vector<int>& path);
        virtual ~PathVisualization() {}
    };

    class DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const cPacket *packet = nullptr;

      public:
        DirectiveResolver(const cPacket *packet) : packet(packet) { }

        virtual const char *resolveDirective(char directive) const override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayRoutes = false;
    NetworkNodeFilter nodeFilter;
    PacketFilter packetFilter;
    ColorSet lineColorSet;
    cFigure::LineStyle lineStyle;
    double lineWidth = NaN;
    bool lineSmooth = false;
    double lineShift = NaN;
    const char *lineShiftMode = nullptr;
    double lineContactSpacing = NaN;
    const char *lineContactMode = nullptr;
    StringFormat labelFormat;
    cFigure::Font labelFont;
    const char *labelColorAsString = nullptr;
    cFigure::Color labelColor;
    const char *fadeOutMode = nullptr;
    double fadeOutTime = NaN;
    double fadeOutAnimationSpeed = NaN;
    //@}

    LineManager *lineManager = nullptr;
    /**
     * Maps packet to module vector.
     */
    std::map<int, std::vector<int>> incompletePaths;
    /**
     * Maps nodes to the number of paths that go through it.
     */
    std::map<int, int> numPaths;
    /**
     * Maps source/destination modules to multiple paths between them.
     */
    std::multimap<std::pair<int, int>, const PathVisualization *> pathVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual bool isPathStart(cModule *module) const = 0;
    virtual bool isPathEnd(cModule *module) const = 0;
    virtual bool isPathElement(cModule *module) const = 0;

    virtual const PathVisualization *createPathVisualization(const std::vector<int>& path, cPacket *packet) const = 0;
    virtual const PathVisualization *getPathVisualization(const std::vector<int>& path);
    virtual void addPathVisualization(const PathVisualization *pathVisualization);
    virtual void removePathVisualization(const PathVisualization *pathVisualization);
    virtual void removeAllPathVisualizations();
    virtual void setAlpha(const PathVisualization *pathVisualization, double alpha) const = 0;

    virtual const std::vector<int> *getIncompletePath(int chunkId);
    virtual void addToIncompletePath(int chunkId, cModule *module);
    virtual void removeIncompletePath(int chunkId);

    virtual std::string getPathVisualizationText(cPacket *packet) const;
    virtual void refreshPathVisualization(const PathVisualization *pathVisualization, cPacket *packet);
    virtual void updatePathVisualization(const std::vector<int>& path, cPacket *packet);

  public:
    virtual ~PathVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PATHVISUALIZERBASE_H

