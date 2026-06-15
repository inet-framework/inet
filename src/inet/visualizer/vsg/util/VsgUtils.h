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
//  - osg::AutoTransform has no VSG 1.1 node equivalent, and the shader-side billboard/auto-scale
//    (StandardLayout::billboard / billboardAutoScaleDistance) does NOT work in the off-screen path.
//    So its screen-relative effects are reproduced by a custom AutoScaleTransform (a vsg::Transform
//    subclass that computes the matrix from the live modelview during the record traversal): labels
//    use it in billboard mode (camera-facing + ~constant on-screen size); arrowheads use it in
//    scale-only mode (constant on-screen size, orientation preserved).
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

// Native text (vsg::Text) with glyphs laid out in the X-Y plane. Returned Text can be mutated later
// (set ->text, call ->setup(0, getOptions())). It is plain, world-space text; wrap it in a billboard
// AutoScaleTransform (see createLabel) to make it face the camera at a constant on-screen size.
ref_ptr<Text> createText(const char *string, const Coord& position, const cFigure::Color& color, double characterSize = 18);

// Camera-facing, constant-on-screen-size text label at a world position (plain text wrapped in a
// billboard AutoScaleTransform). Reads correctly from any view angle and does not change apparent
// size with zoom — the VSG counterpart of an OSG label under AutoTransform(autoScaleToScreen).
ref_ptr<Node> createLabel(const char *string, const Coord& position, const cFigure::Color& color, double characterSize = 18);

// --- transforms ---------------------------------------------------------------------------
// refDistance for label text: chosen so characterSize ~18 renders at roughly OSG's on-screen label
// size in a typical viewport. Smaller -> larger on-screen text. Shared by createLabel and by the
// node-annotation wrapper so every label keeps a consistent on-screen size + stacking.
constexpr double LABEL_AUTOSCALE_REF_DISTANCE = 1400.0;

// Reproduces osg::AutoTransform's screen-relative effects (VSG 1.1 has no such node, and the
// shader-side StandardLayout::billboard/billboardAutoScaleDistance do NOT work off-screen). It is a
// vsg::Transform whose matrix is computed from the live modelview during the record traversal:
// children are scaled by (eye-distance / refDistance) so their projected (on-screen) size stays
// ~constant across zoom. In billboard mode children also face the camera (== autoScaleToScreen +
// ROTATE_TO_SCREEN); screenOffset then shifts them by a constant on-screen amount (used to stack
// node annotations in screen space so they never overlap regardless of zoom). In non-billboard mode
// orientation is preserved (for directional shapes such as arrowheads).
class INET_API AutoScaleTransform : public Inherit<Transform, AutoScaleTransform>
{
  public:
    ::vsg::dvec3 pivot;                    // world-space anchor; scaling/billboarding is about this point
    double refDistance = 600.0;           // eye distance at which children render at their natural size
    bool billboard = false;               // true -> children also face the camera (screen-aligned)
    ::vsg::dvec3 screenOffset = {0, 0, 0}; // billboard-only: constant on-screen offset (in label units)

    ::vsg::dmat4 transform(const ::vsg::dmat4& mv) const override {
        ::vsg::dvec3 eye = mv * pivot;     // pivot in eye space (the camera looks down -Z)
        double dist = -eye.z;
        double s = (dist > 0.0 && refDistance > 0.0) ? dist / refDistance : 1.0;
        if (billboard)
            return ::vsg::translate(eye) * ::vsg::scale(s, s, s) * ::vsg::translate(screenOffset);
        return mv * ::vsg::translate(pivot) * ::vsg::scale(s, s, s) * ::vsg::translate(-pivot);
    }
};

ref_ptr<MatrixTransform> createPositionAttitudeTransform(const Coord& position, const Quaternion& orientation);
// Fixed-orientation stand-in for osg::AutoTransform (legacy; createLabel/AutoScaleTransform supersede
// it). faces the default camera. autoScaleToScreen is accepted for call-site compatibility (no-op).
ref_ptr<MatrixTransform> createAutoTransform(ref_ptr<Node> child, const Coord& position, bool autoScaleToScreen = false);

// --- images ------------------------------------------------------------------------------
std::string resolveImageResource(const char *imageName, cComponent *context = nullptr);
ref_ptr<Data> createImage(const char *fileName);
ref_ptr<Data> createImageFromResource(const char *imageName);

// Textured quad in the X-Y plane (unlit, alpha-blended), larger side = screenSize (label units),
// aspect preserved. 'tint' multiplies the texture (use WHITE for none, e.g. a per-icon tint color).
// Wrap in a billboard AutoScaleTransform for a camera-facing, constant-size icon.
ref_ptr<Node> createTexturedQuad(ref_ptr<Data> image, double screenSize, const cFigure::Color& tint = cFigure::WHITE, double opacity = 1.0);
// Camera-facing, constant-on-screen-size textured icon at a world position (textured quad in a
// billboard AutoScaleTransform) — the VSG counterpart of an OSG icon under AutoTransform.
ref_ptr<Node> createTexturedBillboard(ref_ptr<Data> image, const Coord& position, double screenSize, const cFigure::Color& tint = cFigure::WHITE, double opacity = 1.0);

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
