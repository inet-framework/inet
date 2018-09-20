//
// Copyright (C) 2006-2015 Opensim Ltd
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

#ifndef __INET_OSGUTILS_H
#define __INET_OSGUTILS_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)
#include <omnetpp/osgutil.h>
#include <osg/AutoTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgText/Text>
#endif // ifdef WITH_OSG

namespace inet {

namespace osg {

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)

using namespace ::osg;

inline Vec3d toVec3d(const Coord& coord) { return Vec3d(coord.x, coord.y, coord.z); }
inline Coord toCoord(const Vec3d& vec3d) { return Coord(vec3d.x(), vec3d.y(), vec3d.z()); }

Vec3Array *createCircleVertices(const Coord& center, double radius, int polygonSize);
Vec3Array *createAnnulusVertices(const Coord& center, double outerRadius, double innerRadius, int polygonSize);

Geometry *createLineGeometry(const Coord& start, const Coord& end);
Geometry *createArrowheadGeometry(const Coord& start, const Coord& end, double width, double height);
Geometry *createPolylineGeometry(const std::vector<Coord>& coords);
Geometry *createCircleGeometry(const Coord& center, double radius, int polygonSize);
Geometry *createAnnulusGeometry(const Coord& center, double outerRadius, double innerRadius, int polygonSize);
Geometry *createQuadGeometry(const Coord& start, const Coord& end);
Geometry *createPolygonGeometry(const std::vector<Coord>& points, const Coord& translation = Coord::ZERO);

Node *createArrowhead(const Coord& start, const Coord &end, double width = 10.0, double height = 20.0);
Node *createLine(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead);
Node *createPolyline(const std::vector<Coord>& coords, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead);
osgText::Text *createText(const char *string, const Coord& position, const cFigure::Color& color);

AutoTransform *createAutoTransform(Drawable *drawable, AutoTransform::AutoRotateMode mode, bool autoScaleToScreen, const Coord& position = Coord::ZERO);
PositionAttitudeTransform *createPositionAttitudeTransform(const Coord& position, const Quaternion& orientation);

std::string resolveImageResource(const char *imageName, cComponent *context=nullptr);
Image* createImage(const char *fileName);
Image* createImageFromResource(const char *imageName);
Texture2D *createTexture(const char *name, bool repeat);
Texture2D *createTextureFromResource(const char *imageName, bool repeat);

StateSet *createStateSet(const cFigure::Color& color, double opacity, bool cullBackFace = true);
StateSet *createLineStateSet(const cFigure::Color& color, const cFigure::LineStyle& style, double width, bool overlay = false);

// TODO: move to separate file, recreate node similar to omnetpp figures as a set of basic building blocks
class INET_API LineNode : public Group
{
  protected:
    double lineWidth;
    cFigure::Arrowhead startArrowhead;
    cFigure::Arrowhead endArrowhead;

  protected:
    Geode *getLineGeode() { return static_cast<Geode *>(getChild(0)); }
    Geode *getStartArrowheadGeode() { return static_cast<Geode *>(getStartArrowheadAutoTransform()->getChild(0)); }
    Geode *getEndArrowheadGeode() { return static_cast<Geode *>(getEndArrowheadAutoTransform()->getChild(0)); }

    Geometry *getLineGeometry() { return static_cast<Geometry *>(getLineGeode()->getDrawable(0)); }
    Geometry *getStartArrowheadGeometry() { return static_cast<Geometry *>(getStartArrowheadGeode()->getDrawable(0)); }
    Geometry *getEndArrowheadGeometry() { return static_cast<Geometry *>(getEndArrowheadGeode()->getDrawable(0)); }

    AutoTransform *getStartArrowheadAutoTransform() { return static_cast<AutoTransform *>(getChild(1)); }
    AutoTransform *getEndArrowheadAutoTransform() { return static_cast<AutoTransform *>(getChild(startArrowhead ? 2 : 1)); }

  public:
    LineNode(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead, double lineWidth);

    void setStart(const Coord& start);
    void setEnd(const Coord& end);
};

#endif // ifdef WITH_OSG

} // namespace osg

} // namespace inet

#endif // ifndef __INET_OSGUTILS_H

