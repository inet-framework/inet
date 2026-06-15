//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// VulkanSceneGraph (VSG) port of OsgUtils.h.
//
// Unlike OpenSceneGraph (where a bare osg::Geometry + osg::StateSet renders via the
// fixed-function pipeline), VSG requires an explicit graphics pipeline (shaders). So the
// OSG split of "geometry" + "state set" does NOT translate 1:1: here each helper returns a
// ready-to-render node (a vsg::StateGroup carrying its own pipeline + draw commands), with
// the colour supplied as a per-draw vsg_Color instance attribute. Geometry is rendered
// through the standard VSG flat-shaded (unlit) or Phong (lit) shader set via a
// GraphicsPipelineConfigurator. The off-screen Qtenv backend supplies the lights and
// performs the single-View compile; this code only builds the scene graph.
//
// Known capability gaps (see docs/04-capability-mapping.md), handled pragmatically here:
//  - line width > 1 needs the wideLines device feature (absent on MoltenVK) -> requested but
//    may clamp to 1; TODO geometry-thickened lines.
//  - line stipple (dotted/dashed) has no Vulkan core equivalent -> TODO fragment-shader dash;
//    currently rendered solid.
//  - osg::AutoTransform has no VSG 1.1 node equivalent, so its two screen-relative effects are
//    reproduced separately: text labels use native billboard text (StandardLayout::billboard, faces
//    the camera via the GPU layout shader); arrowheads use a custom AutoScaleTransform (a
//    vsg::Transform subclass that scales to a ~constant on-screen size from the live modelview).
//

#ifndef __INET_VSGUTILS_H
#define __INET_VSGUTILS_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"

#include <vsg/all.h>

namespace inet {

namespace vsg {

using namespace ::vsg;

// --- coordinate conversions ---------------------------------------------------------------
inline ::vsg::vec3 toVsg(const Coord& c) { return ::vsg::vec3((float)c.x, (float)c.y, (float)c.z); }
inline ::vsg::dvec3 toVsgDouble(const Coord& c) { return ::vsg::dvec3(c.x, c.y, c.z); }
inline Coord toCoord(const ::vsg::vec3& v) { return Coord(v.x, v.y, v.z); }
inline Coord toCoord(const ::vsg::dvec3& v) { return Coord(v.x, v.y, v.z); }
inline ::vsg::vec4 toVsgColor(const cFigure::Color& color, double opacity = 1.0)
    { return ::vsg::vec4((float)color.red / 255.0f, (float)color.green / 255.0f, (float)color.blue / 255.0f, (float)opacity); }

// Shared vsg::Options (with vsgXchange readers, for fonts/images). Cached.
ref_ptr<Options> getOptions();

// --- vertex-array builders (counterparts of OSG create*Vertices) --------------------------
ref_ptr<vec3Array> createCircleVertices(const Coord& center, double radius, int polygonSize);
ref_ptr<vec3Array> createAnnulusVertices(const Coord& center, double outerRadius, double innerRadius, int polygonSize);

// --- the pipeline layer -------------------------------------------------------------------
// Wrap a vertex array in a self-contained renderable node (a StateGroup binding a graphics
// pipeline built over the flat (lit=false) or Phong (lit=true) shader set, plus the draw
// commands). normals may be null (defaults to +Z). topology is a VkPrimitiveTopology.
ref_ptr<Node> createGeometry(ref_ptr<vec3Array> vertices, ref_ptr<vec3Array> normals,
        VkPrimitiveTopology topology, const cFigure::Color& color, double opacity = 1.0,
        bool lit = false, double lineWidth = 1.0, bool cullBackFace = false, bool depthTest = true);

// --- high-level node creators (return ready-to-render nodes) -------------------------------
ref_ptr<Node> createLine(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style = cFigure::LINE_SOLID, double width = 1.0, double opacity = 1.0);
ref_ptr<Node> createPolyline(const std::vector<Coord>& coords, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style = cFigure::LINE_SOLID, double width = 1.0, double opacity = 1.0);
ref_ptr<Node> createCircle(const Coord& center, double radius, const cFigure::Color& color,
        const cFigure::LineStyle& style = cFigure::LINE_SOLID, double width = 1.0, int polygonSize = 64);
ref_ptr<Node> createAnnulus(const Coord& center, double outerRadius, double innerRadius,
        const cFigure::Color& color, double opacity = 1.0, int polygonSize = 64);
ref_ptr<Node> createQuad(const Coord& min, const Coord& max, const cFigure::Color& color, double opacity = 1.0);
ref_ptr<Node> createPolygon(const std::vector<Coord>& points, const cFigure::Color& color, double opacity = 1.0, const Coord& translation = Coord::ZERO);
ref_ptr<Node> createArrowhead(const Coord& start, const Coord& end, const cFigure::Color& color, double width = 10.0, double height = 20.0, double opacity = 1.0);
ref_ptr<Node> createSphere(const Coord& center, double radius, const cFigure::Color& color, double opacity = 1.0);
ref_ptr<Node> createBox(const Coord& center, const Coord& size, const cFigure::Color& color, double opacity = 1.0);

// Native text (vsg::Text). The returned Text can be mutated later: set ->text and call ->setup(0, getOptions()).
// NOTE: bare text faces away from the default off-screen camera (renders mirrored); place it via
// createAutoTransform, or use createLabel() below for the common static-label case.
ref_ptr<Text> createText(const char *string, const Coord& position, const cFigure::Color& color, double characterSize = 18, bool billboard = false);

// Camera-facing text label at a world position (createText + createAutoTransform). Use this for
// static labels so they always read correctly; use createText directly only when you must mutate it.
ref_ptr<Node> createLabel(const char *string, const Coord& position, const cFigure::Color& color, double characterSize = 18);

// --- transforms ---------------------------------------------------------------------------
ref_ptr<MatrixTransform> createPositionAttitudeTransform(const Coord& position, const Quaternion& orientation);
// Fixed-orientation stand-in for osg::AutoTransform (the off-screen path has no live billboard);
// faces the default camera. autoScaleToScreen is accepted for call-site compatibility (no-op).
ref_ptr<MatrixTransform> createAutoTransform(ref_ptr<Node> child, const Coord& position, bool autoScaleToScreen = false);

// --- images ------------------------------------------------------------------------------
std::string resolveImageResource(const char *imageName, cComponent *context = nullptr);
ref_ptr<Data> createImage(const char *fileName);
ref_ptr<Data> createImageFromResource(const char *imageName);

// --- mutable line with optional arrowheads (port of OSG LineNode) -------------------------
// Used by LinkVsgVisualizerBase for links whose endpoints move. Color/style/width are baked
// into the geometry (VSG has no detachable state set), so they are set together with the
// endpoints. Updating an endpoint rebuilds the (small) line subgraph.
class INET_API LineNode : public Inherit<Group, LineNode>
{
  protected:
    Coord start, end;
    cFigure::Arrowhead startArrowhead = cFigure::ARROW_NONE;
    cFigure::Arrowhead endArrowhead = cFigure::ARROW_NONE;
    cFigure::Color color = cFigure::BLACK;
    cFigure::LineStyle style = cFigure::LINE_SOLID;
    double lineWidth = 1.0;
    double opacity = 1.0;

    void rebuild();

  public:
    LineNode() {}

    void set(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
            const cFigure::Color& color, const cFigure::LineStyle& style, double lineWidth);
    void setStart(const Coord& start);
    void setEnd(const Coord& end);
    void setAlpha(double opacity);   // for fade-out; rebuilds the line at the new opacity
};

} // namespace vsg

} // namespace inet

#endif
