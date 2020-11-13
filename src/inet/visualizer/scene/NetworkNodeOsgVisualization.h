//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_NETWORKNODEOSGVISUALIZATION_H
#define __INET_NETWORKNODEOSGVISUALIZATION_H

#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"

#ifdef WITH_OSG
#include <osg/AutoTransform>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

#ifdef WITH_OSG

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

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

#endif

