//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TREEVISUALIZERBASE_H
#define __INET_TREEVISUALIZERBASE_H

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

// TODO move to some utility file
inline bool isEmpty(const char *s) { return !s || !s[0]; }
inline bool isNotEmpty(const char *s) { return s && s[0]; }

class INET_API TreeVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API TreeVisualization {
      public:
        std::vector<LineManager::ModulePath> paths;

      public:
        TreeVisualization(const std::vector<std::vector<int>>& tree);
        virtual ~TreeVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayTrees = false;
    ColorSet lineColorSet;
    cFigure::LineStyle lineStyle;
    double lineWidth = NaN;
    bool lineSmooth = false;
    double lineShift = NaN;
    const char *lineShiftMode = nullptr;
    double lineContactSpacing = NaN;
    const char *lineContactMode = nullptr;
    //@}

    LineManager *lineManager = nullptr;
    /**
     * Maps nodes to the number of trees that go through it.
     */
    std::map<int, int> numTrees;
    /**
     * Maps source/destination modules to multiple trees between them.
     */
    std::multimap<std::pair<int, int>, const TreeVisualization *> treeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual const TreeVisualization *createTreeVisualization(const std::vector<std::vector<int>>& tree) const = 0;
    virtual void addTreeVisualization(const TreeVisualization *treeVisualization);
    virtual void removeTreeVisualization(const TreeVisualization *treeVisualization);
    virtual void removeAllTreeVisualizations();
};

} // namespace visualizer

} // namespace inet

#endif

