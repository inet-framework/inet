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

#include "inet/common/OSGUtils.h"

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)
#include <osg/CullFace>
#include <osg/Depth>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LineWidth>
#include <osg/LineStipple>
#include <osgDB/ReadFile>
#endif // ifdef WITH_OSG

namespace inet {

namespace osg {

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)

Vec3Array *createCircleVertices(const Coord& center, double radius, int polygonSize)
{
    auto vertices = new Vec3Array();
    double theta, px, py;
    for (int i = 1; i <= polygonSize; i++) {
        theta = 2.0 * M_PI / polygonSize * i;
        px = center.x + radius * cos(theta);
        py = center.y + radius * sin(theta);
        vertices->push_back(Vec3d(px, py, center.z));
    }
    return vertices;
}

Vec3Array *createAnnulusVertices(const Coord& center, double outerRadius, double innerRadius, int polygonSize)
{
    auto vertices = new Vec3Array();
    if (outerRadius > 0) {
        double theta, px, py;
        for (int i = 0; i <= polygonSize; i++) {
            theta = 2.0 * M_PI / polygonSize * i;
            px = center.x + outerRadius * cos(theta);
            py = center.y + outerRadius * sin(theta);
            vertices->push_back(Vec3d(px, py, center.z));
            px = center.x + innerRadius * cos(theta);
            py = center.y + innerRadius * sin(theta);
            vertices->push_back(Vec3d(px, py, center.z));
        }
    }
    else {
        vertices->push_back(Vec3d(0.0, 0.0, 0.0));
        vertices->push_back(Vec3d(0.0, 0.0, 0.0));
    }
    return vertices;
}

Geometry *createLineGeometry(const Coord& begin, const Coord& end)
{
    auto geometry = new Geometry();
    auto vertices = new Vec3Array();
    vertices->push_back(Vec3(begin.x, begin.y, begin.z));
    vertices->push_back(Vec3(end.x, end.y, end.z));
    auto drawArrays = new DrawArrays(PrimitiveSet::LINE_STRIP);
    drawArrays->setFirst(0);
    drawArrays->setCount(vertices->size());
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(drawArrays);
    return geometry;
}

Geometry *createArrowheadGeometry(const Coord& begin, const Coord& end, double width, double height)
{
    auto geometry = new Geometry();
    auto vertices = new Vec3Array();
    Coord direction = end - begin;
    Vec3 e = Vec3(end.x, end.y, end.z);
    Vec3 v = Vec3(direction.x, direction.y, direction.z);
    v.normalize();
    Vec3 d = v * height;
    Vec3 zero;
    // NOTE: make sure the normal is not parallel with the rotation vector
    auto cpX = v ^ X_AXIS;
    auto cpY = v ^ Y_AXIS;
    auto cpZ = v ^ Z_AXIS;
    if (cpX != zero)
        v = cpX;
    else if (cpY != zero)
        v = cpY;
    else if (cpZ != zero)
        v = cpZ;
    else
        ASSERT(false);
    v *= width / 2;
    vertices->push_back(e);
    vertices->push_back(e - d + v);
    vertices->push_back(e - d - v);
    auto drawArrays = new DrawArrays(PrimitiveSet::TRIANGLES);
    drawArrays->setFirst(0);
    drawArrays->setCount(3);
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(drawArrays);
    return geometry;
}

Geometry *createPolylineGeometry(const std::vector<Coord>& coords)
{
    auto geometry = new Geometry();
    auto vertices = new Vec3Array();
    for (auto coord : coords)
        vertices->push_back(Vec3(coord.x, coord.y, coord.z));
    auto drawArrays = new DrawArrays(PrimitiveSet::LINE_STRIP);
    drawArrays->setFirst(0);
    drawArrays->setCount(vertices->size());
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(drawArrays);
    return geometry;
}

Geometry *createCircleGeometry(const Coord& center, double radius, int polygonSize)
{
    auto geometry = new Geometry();
    geometry->setVertexArray(createCircleVertices(center, radius, polygonSize));
    geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::LINE_LOOP, 0, polygonSize));
    return geometry;
}

Geometry *createAnnulusGeometry(const Coord& center, double outerRadius, double innerRadius, int polygonSize)
{
    auto geometry = new Geometry();
    geometry->setVertexArray(createAnnulusVertices(center, outerRadius, innerRadius, polygonSize));
    geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::TRIANGLE_STRIP, 0, 2 * polygonSize + 2));
    return geometry;
}

Geometry *createQuadGeometry(const Coord& min, const Coord& max)
{
    auto geometry = new Geometry();
    auto vertices = new Vec3Array();
    vertices->push_back(Vec3(min.x, min.y, min.z));
    vertices->push_back(Vec3(min.x, max.y, min.z));
    vertices->push_back(Vec3(max.x, max.y, min.z));
    vertices->push_back(Vec3(max.x, min.y, min.z));
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS, 0, 4));
    return geometry;
}

Geometry *createPolygonGeometry(const std::vector<Coord>& points, const Coord& translation)
{
    auto geometry = new Geometry();
    auto vertices = new Vec3Array();
    for (auto point : points)
        vertices->push_back(Vec3d(point.x + translation.x, point.y + translation.y, point.z + translation.z));
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::POLYGON, 0, vertices->size()));
    return geometry;
}

osg::Node *createArrowhead(const Coord& begin, const Coord &end)
{
    auto direction = begin - end;
    auto arrowhead = inet::osg::createArrowheadGeometry(direction, Coord::ZERO);
    auto autoTransform = inet::osg::createAutoTransform(arrowhead, osg::AutoTransform::ROTATE_TO_AXIS, true, end);
    auto vertexArray = static_cast<osg::Vec3Array *>(arrowhead->getVertexArray());
    auto normal = (vertexArray->at(0) - vertexArray->at(1)) ^ (vertexArray->at(0) - vertexArray->at(2));
    autoTransform->setNormal(normal);
    autoTransform->setAxis(toVec3d(direction));
    return autoTransform;
}

osg::Node *createLine(const Coord& begin, const Coord& end, cFigure::Arrowhead beginArrowhead, cFigure::Arrowhead endArrowhead)
{
    auto line = inet::osg::createLineGeometry(begin, end);
    auto geode = new osg::Geode();
    geode->addDrawable(line);
    auto group = new osg::Group();
    group->addChild(geode);
    if (beginArrowhead)
        group->addChild(createArrowhead(end, begin));
    if (endArrowhead)
        group->addChild(createArrowhead(begin, end));
    return group;
}

osg::Node *createPolyline(const std::vector<Coord>& coords, cFigure::Arrowhead beginArrowhead, cFigure::Arrowhead endArrowhead)
{
    auto line = inet::osg::createPolylineGeometry(coords);
    auto geode = new osg::Geode();
    geode->addDrawable(line);
    auto group = new osg::Group();
    group->addChild(geode);
    if (beginArrowhead)
        group->addChild(createArrowhead(coords[1], coords[0]));
    if (endArrowhead)
        group->addChild(createArrowhead(coords[coords.size() -  2], coords[coords.size() - 1]));
    return group;
}

osgText::Text *createText(const char *string, const Coord& position, const cFigure::Color& color)
{
    auto text = new osgText::Text();
    text->setColor(Vec4(color.red, color.green, color.blue, 1.0));
    text->setCharacterSize(18);
    text->setText(string);
    text->setPosition(Vec3(position.x, position.y, position.z));
    return text;
}

AutoTransform *createAutoTransform(Drawable *drawable, AutoTransform::AutoRotateMode mode, bool autoScaleToScreen, const Coord& position)
{
    auto geode = new Geode();
    geode->addDrawable(drawable);
    auto autoTransform = new AutoTransform();
    autoTransform->setAutoScaleToScreen(autoScaleToScreen);
    autoTransform->setAutoRotateMode(mode);
    autoTransform->setPosition(Vec3(position.x, position.y, position.z));
    autoTransform->addChild(geode);
    return autoTransform;
}

PositionAttitudeTransform *createPositionAttitudeTransform(const Coord& position, const EulerAngles& orientation)
{
    auto pat = new PositionAttitudeTransform();
    pat->setPosition(Vec3d(position.x, position.y, position.z));
    pat->setAttitude(Quat(orientation.alpha, Vec3d(1.0, 0.0, 0.0)) *
                     Quat(orientation.beta, Vec3d(0.0, 1.0, 0.0)) *
                     Quat(orientation.gamma, Vec3d(0.0, 0.0, 1.0)));
    return pat;
}

osg::Image* createImage(const char *fileName)
{
    auto image = osgDB::readImageFile(fileName);
    if (image == nullptr)
        throw cRuntimeError("Image '%s' not found", fileName);
    return image;
}

Texture2D *createTexture(const char *name, bool repeat)
{
    auto image = createImage(name);
    if (image == nullptr)
        throw cRuntimeError("Cannot find image: '%s'", name);
    auto texture = new Texture2D();
    texture->setImage(image);
    if (repeat) {
        texture->setWrap(Texture2D::WRAP_S, Texture2D::REPEAT);
        texture->setWrap(Texture2D::WRAP_T, Texture2D::REPEAT);
        texture->setWrap(Texture2D::WRAP_R, Texture2D::REPEAT);
    }
    return texture;
}

StateSet *createStateSet(const cFigure::Color& color, double opacity, bool cullBackFace)
{
    auto stateSet = new StateSet();
    auto material = new Material();
    Vec4 colorVec((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0, opacity);
    material->setAmbient(Material::FRONT_AND_BACK, colorVec);
    material->setDiffuse(Material::FRONT_AND_BACK, colorVec);
    material->setAlpha(Material::FRONT_AND_BACK, opacity);
    stateSet->setAttribute(material);
    stateSet->setMode(GL_BLEND, StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, StateAttribute::ON);
    stateSet->setRenderingHint(opacity == 1.0 ? StateSet::OPAQUE_BIN :  StateSet::TRANSPARENT_BIN);
    if (cullBackFace) {
        auto cullFace = new CullFace();
        cullFace->setMode(CullFace::BACK);
        stateSet->setAttributeAndModes(cullFace, StateAttribute::ON);
    }
    return stateSet;
}

StateSet *createLineStateSet(const cFigure::Color& color, const cFigure::LineStyle& style, double width)
{
    auto stateSet = new StateSet();
    auto material = new Material();
    Vec4 colorVec((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0, 1.0);
    material->setAmbient(Material::FRONT_AND_BACK, colorVec);
    material->setDiffuse(Material::FRONT_AND_BACK, colorVec);
    stateSet->setAttribute(material);
    stateSet->setMode(GL_BLEND, StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    if (width != 1.0) {
        auto lineWidth = new osg::LineWidth();
        lineWidth->setWidth(width);
        stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    }
    if (style != cFigure::LINE_SOLID) {
        auto lineStipple = new osg::LineStipple();
        switch (style) {
            case cFigure::LINE_DOTTED:
                lineStipple->setPattern(0xAAAA);
                break;
            case cFigure::LINE_DASHED:
                lineStipple->setPattern(0xF0F0);
                break;
            default:
                throw cRuntimeError("Unknown line style");
        }
        stateSet->setAttributeAndModes(lineStipple, osg::StateAttribute::ON);
    }
    return stateSet;
}

#endif // ifdef WITH_OSG

} // namespace osg

} // namespace inet

