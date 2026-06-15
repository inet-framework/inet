//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODEVSGVISUALIZATION_H
#define __INET_NETWORKNODEVSGVISUALIZATION_H

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

//
// The 3D representation of one network node. It is both a renderer-neutral
// NetworkNodeVisualization and a VSG MatrixTransform (positioned by the visualizer's
// refreshDisplay). The main part is the node's visual marker; other visualizers attach
// annotations (info/queue/statistic/...) that stack above the node.
//
class INET_API NetworkNodeVsgVisualization : public NetworkNodeVisualizerBase::NetworkNodeVisualization,
                                             public ::vsg::Inherit<::vsg::MatrixTransform, NetworkNodeVsgVisualization>
{
  protected:
    class Annotation {
      public:
        ::vsg::ref_ptr<::vsg::Node> node;                          // the content (plain text/geometry)
        ::vsg::ref_ptr<inet::vsg::AutoScaleTransform> transform;   // billboard+autoscale wrapper (stable across content rebuilds)
        ::vsg::dvec3 size;
        double priority;
        Annotation(::vsg::ref_ptr<::vsg::Node> node, ::vsg::ref_ptr<inet::vsg::AutoScaleTransform> transform, ::vsg::dvec3 size, double priority) :
            node(node), transform(transform), size(size), priority(priority) {}
    };

    ::vsg::dvec3 size;
    ::vsg::ref_ptr<::vsg::Group> annotationNode;
    std::vector<Annotation> annotations;
    Coord position = Coord::ZERO;
    ::vsg::dvec3 labelPivot = {0, 0, 0};   // shared world anchor (box top) for the name + all annotations
    double labelBaseHeight = 0;            // on-screen height reserved below annotations (the name label)

    virtual void updateAnnotationPositions();

  public:
    NetworkNodeVsgVisualization(cModule *networkNode, bool displayModuleName);

    virtual ::vsg::ref_ptr<::vsg::Node> getMainPart() { return children.empty() ? ::vsg::ref_ptr<::vsg::Node>() : children[0]; }

    virtual Coord getPosition() const { return position; }
    virtual void setPosition(const Coord& position) { this->position = position; }

    virtual int getNumAnnotations() const { return annotations.size(); }
    virtual void addAnnotation(::vsg::ref_ptr<::vsg::Node> annotation, ::vsg::dvec3 size, double priority);
    virtual void removeAnnotation(::vsg::Node *node);
    virtual void removeAnnotation(int index);
};

} // namespace visualizer

} // namespace inet

#endif
