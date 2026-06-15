//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/util/VsgUtils.h"

#include <vsgXchange/all.h>

#include <cmath>

namespace inet {

namespace vsg {

// ---------------------------------------------------------------------------------------------
// Shared resources (options, shader sets, builder) — created lazily and cached.
// ---------------------------------------------------------------------------------------------

// These shared VSG objects are cached as INTENTIONALLY-LEAKED singletons (heap ref_ptrs that are
// never destroyed). Destroying a vsg object during C++ static/exit teardown crashes, because vsg's
// global allocator may already be gone (vsg::deallocate -> null deref). Leaking them avoids that;
// the OS reclaims the memory at process exit. (The off-screen device is leaked for the same reason.)

ref_ptr<Options> getOptions()
{
    static ref_ptr<Options>& options = *(new ref_ptr<Options>());
    if (!options) {
        options = Options::create();
        options->add(vsgXchange::all::create());
    }
    return options;
}

static ref_ptr<ShaderSet> getFlatShaderSet()
{
    static ref_ptr<ShaderSet>& shaderSet = *(new ref_ptr<ShaderSet>());
    if (!shaderSet) shaderSet = createFlatShadedShaderSet(getOptions());
    return shaderSet;
}

static ref_ptr<ShaderSet> getPhongShaderSet()
{
    static ref_ptr<ShaderSet>& shaderSet = *(new ref_ptr<ShaderSet>());
    if (!shaderSet) shaderSet = createPhongShaderSet(getOptions());
    return shaderSet;
}

static ref_ptr<Builder> getBuilder()
{
    static ref_ptr<Builder>& builder = *(new ref_ptr<Builder>());
    if (!builder) {
        builder = Builder::create();
        builder->options = getOptions();
    }
    return builder;
}

// Fetch (or create) the single pipeline state of type T that the configurator seeds into
// pipelineStates, so we can override topology / rasterization / blending / depth per call.
template<typename T>
static ref_ptr<T> getPipelineState(GraphicsPipelineConfigurator *config)
{
    for (auto& s : config->pipelineStates)
        if (auto t = dynamic_cast<T *>(s.get()))
            return ref_ptr<T>(t);
    auto t = T::create();
    config->pipelineStates.push_back(t);
    return t;
}

// ---------------------------------------------------------------------------------------------
// The pipeline layer: turn a vertex array + topology + colour into a renderable node.
// ---------------------------------------------------------------------------------------------

ref_ptr<Node> createGeometry(ref_ptr<vec3Array> vertices, ref_ptr<vec3Array> normals,
        VkPrimitiveTopology topology, const cFigure::Color& color, double opacity,
        bool lit, double lineWidth, bool cullBackFace, bool depthTest)
{
    auto shaderSet = lit ? getPhongShaderSet() : getFlatShaderSet();
    auto config = GraphicsPipelineConfigurator::create(shaderSet);

    getPipelineState<InputAssemblyState>(config)->topology = topology;

    auto rasterization = getPipelineState<RasterizationState>(config);
    rasterization->cullMode = cullBackFace ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterization->lineWidth = (float)lineWidth; // clamps to 1 where wideLines is unsupported (e.g. MoltenVK)

    if (opacity < 1.0)
        getPipelineState<ColorBlendState>(config)->configureAttachments(true);

    if (!depthTest) {
        auto depth = getPipelineState<DepthStencilState>(config);
        depth->depthTestEnable = VK_FALSE;
        depth->depthWriteEnable = VK_FALSE;
    }

    // material left white so the per-vertex/instance vsg_Color carries the exact colour for both
    // the flat (unlit) and Phong (lit) shader sets; lighting then modulates it in the lit case.
    auto material = PhongMaterialValue::create();
    material->value().diffuse = ::vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material->value().ambient = ::vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    config->assignDescriptor("material", material);

    if (!normals)
        normals = vec3Array::create(vertices->size(), ::vsg::vec3(0.0f, 0.0f, 1.0f));
    auto colorValue = vec4Value::create(toVsgColor(color, opacity));
    auto texCoordValue = vec2Value::create(::vsg::vec2(0.0f, 0.0f));

    DataList vertexArrays;
    config->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    config->assignArray(vertexArrays, "vsg_Normal", VK_VERTEX_INPUT_RATE_VERTEX, normals);
    config->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_INSTANCE, colorValue);
    config->assignArray(vertexArrays, "vsg_TexCoord0", VK_VERTEX_INPUT_RATE_INSTANCE, texCoordValue);

    config->init();

    auto stateGroup = StateGroup::create();
    config->copyTo(stateGroup);

    auto commands = Commands::create();
    commands->addChild(BindVertexBuffers::create(config->baseAttributeBinding, vertexArrays));
    commands->addChild(Draw::create(vertices->size(), 1, 0, 0));
    stateGroup->addChild(commands);

    return stateGroup;
}

// ---------------------------------------------------------------------------------------------
// Vertex-array builders
// ---------------------------------------------------------------------------------------------

ref_ptr<vec3Array> createCircleVertices(const Coord& center, double radius, int polygonSize)
{
    auto vertices = vec3Array::create(polygonSize);
    for (int i = 0; i < polygonSize; i++) {
        double theta = 2.0 * M_PI / polygonSize * (i + 1);
        vertices->set(i, ::vsg::vec3((float)(center.x + radius * cos(theta)), (float)(center.y + radius * sin(theta)), (float)center.z));
    }
    return vertices;
}

ref_ptr<vec3Array> createAnnulusVertices(const Coord& center, double outerRadius, double innerRadius, int polygonSize)
{
    int count = 2 * (polygonSize + 1);
    auto vertices = vec3Array::create(count);
    if (outerRadius > 0) {
        for (int i = 0; i <= polygonSize; i++) {
            double theta = 2.0 * M_PI / polygonSize * i;
            vertices->set(2 * i, ::vsg::vec3((float)(center.x + outerRadius * cos(theta)), (float)(center.y + outerRadius * sin(theta)), (float)center.z));
            vertices->set(2 * i + 1, ::vsg::vec3((float)(center.x + innerRadius * cos(theta)), (float)(center.y + innerRadius * sin(theta)), (float)center.z));
        }
    }
    else
        for (int i = 0; i < count; i++)
            vertices->set(i, ::vsg::vec3(0.0f, 0.0f, 0.0f));
    return vertices;
}

// Arrowhead: a flat triangle at 'end', pointing along start->end (the axis is baked in, so no
// AutoTransform is needed). Mirrors OsgUtils::createArrowheadGeometry's world-space math.
static ref_ptr<vec3Array> createArrowheadVertices(const Coord& start, const Coord& end, double width, double height)
{
    ::vsg::vec3 e = toVsg(end);
    ::vsg::vec3 dir = toVsg(end - start);
    double len = ::vsg::length(dir);
    ::vsg::vec3 v = (len > 1e-9) ? dir / (float)len : ::vsg::vec3(1, 0, 0);
    ::vsg::vec3 d = v * (float)height;
    // a perpendicular to v: cross with whichever axis is not parallel
    ::vsg::vec3 perp = ::vsg::cross(v, ::vsg::vec3(1, 0, 0));
    if (::vsg::length(perp) < 1e-6f) perp = ::vsg::cross(v, ::vsg::vec3(0, 1, 0));
    if (::vsg::length(perp) < 1e-6f) perp = ::vsg::cross(v, ::vsg::vec3(0, 0, 1));
    perp = ::vsg::normalize(perp) * (float)(width / 2);
    auto vertices = vec3Array::create(3);
    vertices->set(0, e);
    vertices->set(1, e - d + perp);
    vertices->set(2, e - d - perp);
    return vertices;
}

// ---------------------------------------------------------------------------------------------
// High-level node creators
// ---------------------------------------------------------------------------------------------

ref_ptr<Node> createArrowhead(const Coord& start, const Coord& end, const cFigure::Color& color, double width, double height)
{
    // TODO: arrowheads are world-fixed size (OSG used AutoTransform autoScaleToScreen for a
    // constant on-screen size, which the off-screen path can't reproduce).
    auto vertices = createArrowheadVertices(start, end, width, height);
    return createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, color, 1.0, /*lit*/ false, 1.0, /*cullBackFace*/ false);
}

ref_ptr<Node> createLine(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style, double width)
{
    // TODO: style (dotted/dashed) is rendered solid for now (no Vulkan core line stipple, R-STIPPLE).
    (void)style;
    auto vertices = vec3Array::create({ toVsg(start), toVsg(end) });
    auto line = createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, color, 1.0, /*lit*/ false, width);
    if (!startArrowhead && !endArrowhead)
        return line;
    auto group = Group::create();
    group->addChild(line);
    if (startArrowhead)
        group->addChild(createArrowhead(end, start, color));
    if (endArrowhead)
        group->addChild(createArrowhead(start, end, color));
    return group;
}

ref_ptr<Node> createPolyline(const std::vector<Coord>& coords, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style, double width)
{
    (void)style;
    auto vertices = vec3Array::create(coords.size());
    for (size_t i = 0; i < coords.size(); i++)
        vertices->set(i, toVsg(coords[i]));
    auto line = createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, color, 1.0, /*lit*/ false, width);
    if ((!startArrowhead && !endArrowhead) || coords.size() < 2)
        return line;
    auto group = Group::create();
    group->addChild(line);
    if (startArrowhead)
        group->addChild(createArrowhead(coords[1], coords[0], color));
    if (endArrowhead)
        group->addChild(createArrowhead(coords[coords.size() - 2], coords[coords.size() - 1], color));
    return group;
}

ref_ptr<Node> createCircle(const Coord& center, double radius, const cFigure::Color& color,
        const cFigure::LineStyle& style, double width, int polygonSize)
{
    (void)style;
    // close the loop (Vulkan has no LINE_LOOP): repeat the first vertex with a LINE_STRIP
    auto base = createCircleVertices(center, radius, polygonSize);
    auto vertices = vec3Array::create(polygonSize + 1);
    for (int i = 0; i < polygonSize; i++)
        vertices->set(i, base->at(i));
    vertices->set(polygonSize, base->at(0));
    return createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, color, 1.0, /*lit*/ false, width);
}

ref_ptr<Node> createAnnulus(const Coord& center, double outerRadius, double innerRadius,
        const cFigure::Color& color, double opacity, int polygonSize)
{
    auto vertices = createAnnulusVertices(center, outerRadius, innerRadius, polygonSize);
    return createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, color, opacity, /*lit*/ false, 1.0, /*cullBackFace*/ false);
}

ref_ptr<Node> createQuad(const Coord& min, const Coord& max, const cFigure::Color& color, double opacity)
{
    ::vsg::vec3 a((float)min.x, (float)min.y, (float)min.z);
    ::vsg::vec3 b((float)max.x, (float)min.y, (float)min.z);
    ::vsg::vec3 c((float)max.x, (float)max.y, (float)min.z);
    ::vsg::vec3 d((float)min.x, (float)max.y, (float)min.z);
    auto vertices = vec3Array::create({ a, b, c, a, c, d });
    return createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, color, opacity, /*lit*/ false, 1.0, /*cullBackFace*/ false);
}

ref_ptr<Node> createPolygon(const std::vector<Coord>& points, const cFigure::Color& color, double opacity, const Coord& translation)
{
    auto vertices = vec3Array::create(points.size());
    for (size_t i = 0; i < points.size(); i++)
        vertices->set(i, toVsg(points[i] + translation));
    return createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, color, opacity, /*lit*/ false, 1.0, /*cullBackFace*/ false);
}

ref_ptr<Node> createSphere(const Coord& center, double radius, const cFigure::Color& color, double opacity)
{
    GeometryInfo gi;
    StateInfo si;
    gi.position = toVsg(center);
    gi.dx = ::vsg::vec3((float)(2 * radius), 0, 0);
    gi.dy = ::vsg::vec3(0, (float)(2 * radius), 0);
    gi.dz = ::vsg::vec3(0, 0, (float)(2 * radius));
    gi.color = toVsgColor(color, opacity);
    if (opacity < 1.0) si.blending = true;
    return getBuilder()->createSphere(gi, si);
}

ref_ptr<Node> createBox(const Coord& center, const Coord& size, const cFigure::Color& color, double opacity)
{
    GeometryInfo gi;
    StateInfo si;
    gi.position = toVsg(center);
    gi.dx = ::vsg::vec3((float)size.x, 0, 0);
    gi.dy = ::vsg::vec3(0, (float)size.y, 0);
    gi.dz = ::vsg::vec3(0, 0, (float)size.z);
    gi.color = toVsgColor(color, opacity);
    if (opacity < 1.0) si.blending = true;
    return getBuilder()->createBox(gi, si);
}

ref_ptr<Text> createText(const char *string, const Coord& position, const cFigure::Color& color, double characterSize)
{
    static ref_ptr<Font>& font = *(new ref_ptr<Font>());  // leaked singleton (see getOptions note)
    if (!font) {
        for (const char *path : {
                 "/System/Library/Fonts/Supplemental/Arial.ttf",                     // macOS
                 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",                  // Debian/Ubuntu
                 "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                 "/usr/share/fonts/dejavu/DejaVuSans.ttf",                           // Fedora/RHEL
                 "C:/Windows/Fonts/arial.ttf" }) {                                   // Windows
            if ((font = read_cast<Font>(path, getOptions()))) break;
        }
    }
    auto layout = StandardLayout::create();
    layout->position = toVsg(position);
    float s = (float)characterSize;
    layout->horizontal = ::vsg::vec3(s, 0.0f, 0.0f);
    layout->vertical = ::vsg::vec3(0.0f, 0.0f, s);   // glyphs stand up along +Z
    layout->color = toVsgColor(color);
    layout->horizontalAlignment = StandardLayout::CENTER_ALIGNMENT;
    auto text = Text::create();
    text->font = font;
    text->layout = layout;
    text->text = stringValue::create(string ? string : "");
    if (font)
        text->setup(0, getOptions());
    return text;
}

// ---------------------------------------------------------------------------------------------
// Transforms
// ---------------------------------------------------------------------------------------------

ref_ptr<Node> createLabel(const char *string, const Coord& position, const cFigure::Color& color, double characterSize)
{
    return createAutoTransform(createText(string, Coord::ZERO, color, characterSize), position, true);
}

ref_ptr<MatrixTransform> createPositionAttitudeTransform(const Coord& position, const Quaternion& orientation)
{
    // orientation is a unit quaternion with vector part (v.x,v.y,v.z) and scalar part s.
    double s = std::max(-1.0, std::min(1.0, orientation.s));
    double angle = 2.0 * std::acos(s);
    double sinHalf = std::sqrt(std::max(0.0, 1.0 - s * s));
    ::vsg::dvec3 axis = (sinHalf < 1e-9) ? ::vsg::dvec3(0, 0, 1)
            : ::vsg::dvec3(orientation.v.x / sinHalf, orientation.v.y / sinHalf, orientation.v.z / sinHalf);
    auto mt = MatrixTransform::create();
    mt->matrix = translate(toVsgDouble(position)) * rotate(angle, axis);
    return mt;
}

ref_ptr<MatrixTransform> createAutoTransform(ref_ptr<Node> child, const Coord& position, bool autoScaleToScreen)
{
    (void)autoScaleToScreen; // no live billboard/auto-scale in the off-screen path
    auto mt = MatrixTransform::create();
    // face the default camera (which looks along -Y): rotate 180 deg about Z so text/glyphs read correctly
    mt->matrix = translate(toVsgDouble(position)) * rotate(::vsg::PI, 0.0, 0.0, 1.0);
    if (child)
        mt->addChild(child);
    return mt;
}

// ---------------------------------------------------------------------------------------------
// Images
// ---------------------------------------------------------------------------------------------

std::string resolveImageResource(const char *imageName, cComponent *context)
{
    if (context == nullptr)
        context = cSimulation::getActiveSimulation()->getContextModule();
    std::string path;
    for (auto ext : { "", ".png", ".gif", ".jpg" }) {
        path = context->resolveResourcePath((std::string(imageName) + ext).c_str());
        if (!path.empty())
            return path;
    }
    throw cRuntimeError("Image '%s' not found", imageName);
}

ref_ptr<Data> createImage(const char *fileName)
{
    auto image = read_cast<Data>(fileName, getOptions());
    if (!image)
        throw cRuntimeError("Cannot load image '%s'", fileName);
    return image;
}

ref_ptr<Data> createImageFromResource(const char *imageName)
{
    return createImage(resolveImageResource(imageName).c_str());
}

// ---------------------------------------------------------------------------------------------
// LineNode (mutable line with optional arrowheads)
// ---------------------------------------------------------------------------------------------

void LineNode::rebuild()
{
    children.clear();
    addChild(createLine(start, end, startArrowhead, endArrowhead, color, style, lineWidth));
}

void LineNode::set(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style, double lineWidth)
{
    this->start = start;
    this->end = end;
    this->startArrowhead = startArrowhead;
    this->endArrowhead = endArrowhead;
    this->color = color;
    this->style = style;
    this->lineWidth = lineWidth;
    rebuild();
}

void LineNode::setStart(const Coord& start)
{
    this->start = start;
    rebuild();
}

void LineNode::setEnd(const Coord& end)
{
    this->end = end;
    rebuild();
}

} // namespace vsg

} // namespace inet
