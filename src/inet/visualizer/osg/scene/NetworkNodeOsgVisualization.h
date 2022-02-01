//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODEOSGVISUALIZATION_H
#define __INET_NETWORKNODEOSGVISUALIZATION_H

#include <osg/AutoTransform>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>

#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeOsgVisualization : public NetworkNodeVisualizerBase::NetworkNodeVisualization, public osg::PositionAttitudeTransform
{
  protected:
    class INET_API Annotation {
      public:
        osg::Node *node;
        osg::Vec3d size;
        double priority;

      public:
        Annotation(osg::Node *node, osg::Vec3d size, double priority);
    };

  protected:
    osg::Vec3d size;
    osg::Group *annotationNode = nullptr;
    std::vector<Annotation> annotations;

  protected:
    virtual std::string getOsgModelPath(cModule *networkNode);
    virtual void updateAnnotationPositions();

  public:
    NetworkNodeOsgVisualization(cModule *networkNode, bool displayModuleName);

    virtual osg::Node *getMainPart() { return getChild(0); }

    virtual int getNumAnnotations() const { return annotations.size(); }
    virtual void addAnnotation(osg::Node *annotation, osg::Vec3d size, double priority);
    virtual void removeAnnotation(osg::Node *node);
    virtual void removeAnnotation(int index);
};

} // namespace visualizer

} // namespace inet

#endif

