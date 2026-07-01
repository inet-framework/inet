//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/util/VsgUtils.h"

#include <vsgXchange/all.h>

#include <cmath>
#include <map>

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

// Patch the alpha-channel blend factors in a pipeline built by the VSG Builder (createSphere /
// createBox / createTexturedQuad). The Builder uses configureAttachments(true) which sets the
// alpha blend to srcA*srcA (nearly 0) — the same "black blob" bug as createGeometry. The Builder
// bakes the VkPipeline during createSphere (via graphicsPipelineConfig->init()), so we can't just
// edit pipelineStates; we rebuild the pipeline from the existing one's state with the corrected
// alpha factors and swap the BindGraphicsPipeline in the returned StateGroup.
// See createGeometry's blend comment for the full explanation of the alpha-channel fix.
static void fixBuilderBlendAlpha(const ref_ptr<Node>& node)
{
    if (!node) return;
    // Collect EVERY BindGraphicsPipeline in the returned scene graph. Builder::createSphere /
    // createBox / createQuad currently each emit a single pipeline, but a future VSG version could
    // split multi-part geometry across several StateGroups, so patch them all rather than stopping
    // at the first one found.
    struct FindBindPipelines : public ::vsg::Visitor {
        std::vector<BindGraphicsPipeline *> bgps;
        void apply(Object& o) override { o.traverse(*this); }
        void apply(StateGroup& sg) override {
            for (auto& sc : sg.stateCommands)
                if (auto b = dynamic_cast<BindGraphicsPipeline *>(sc.get()))
                    bgps.push_back(b);
            sg.traverse(*this);
        }
    } v;
    node->accept(v);
    for (auto bgp : v.bgps) {
        if (!bgp->pipeline) continue;
        // Clone the pipeline states (don't mutate the originals in place): the Builder may cache and
        // reuse the same ColorBlendState ref_ptr across pipelines, so in-place edits would corrupt
        // the cached state. Build a fresh GraphicsPipelineStates vector with cloned ColorBlendStates.
        auto orig = bgp->pipeline;
        GraphicsPipelineStates states;
        for (auto& s : orig->pipelineStates) {
            if (auto cbs = dynamic_cast<ColorBlendState *>(s.get())) {
                auto cloned = ColorBlendState::create(*cbs);
                for (auto& att : cloned->attachments) {
                    att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    att.alphaBlendOp = VK_BLEND_OP_ADD;
                }
                states.push_back(cloned);
            }
            else
                states.push_back(s);
        }
        bgp->pipeline = GraphicsPipeline::create(orig->layout, orig->stages, states, orig->subpass);
    }
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

    if (opacity < 1.0) {
        // Enable standard alpha blending for the RGB channels. Override the ALPHA-channel blend
        // factors: VSG's configureAttachments(true) sets srcAlpha=SRC_ALPHA, dstAlpha=ZERO, which
        // makes the output alpha = srcA*srcA (nearly 0 for translucent geometry). The off-screen
        // viewer hands the framebuffer to Qt as a QImage with per-pixel alpha, so a near-0 alpha
        // makes Qt composite the (dark) widget background over the disc -> the disc appears as a
        // "black blob" even though the RGB blended correctly. Fix: keep the output alpha opaque by
        // blending srcA*1 + dstA*(1-srcA), so a translucent pixel over an opaque background stays
        // alpha=1 and Qt displays the blended RGB as-is.
        auto blend = getPipelineState<ColorBlendState>(config);
        blend->configureAttachments(true);
        for (auto& att : blend->attachments) {
            att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            att.alphaBlendOp = VK_BLEND_OP_ADD;
        }
    }

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

// A radially-subdivided ring (annulus) whose PER-VERTEX opacity reproduces the OSG signal wave: a
// distance fade pow(fadingFactor, -d/fadingDistance) times a cosine ripple across the radius (period
// waveLength, leading edge at waveOffset; 'a' = waveAmplitude * waveFadingFactor / 2). The medium
// visualizer rebuilds this each frame with the current radii/waveOffset; per-vertex vsg_Color
// (interpolated) + alpha blending produce the gradient with no custom shader/uniforms (which don't
// work in the off-screen path). waveFadingFactor mirrors the OSG shader uniform: it scales the ripple
// amplitude so the wave flattens toward a smooth disc as the animation speed increases.
ref_ptr<Node> createWaveRing(const Coord& center, double innerRadius, double outerRadius,
        const cFigure::Color& color, double waveLength, double waveAmplitude, double waveOffset,
        double fadingFactor, double fadingDistance, double waveFadingFactor, int segments)
{
    double a = waveAmplitude * waveFadingFactor / 2.0;
    auto alphaAt = [&](double d) {
        double fade = (fadingDistance > 0 && fadingFactor > 1.0) ? std::pow(fadingFactor, -d / fadingDistance) : 1.0;
        double phi = (waveLength > 0) ? (d - waveOffset) / waveLength * 2.0 * M_PI : 0.0;
        return (float)std::max(0.0, std::min(1.0, fade * (1.0 - a + std::cos(phi) * a)));
    };
    // Clamp the OUTER radius to where the distance fade renders the wave invisible (alpha < alphaFloor).
    // A signal propagates at light speed, so its true radius reaches hundreds of km within a packet's
    // duration; without clamping, the whole scene would fall inside one huge radial band and render as a
    // flat opaque disc instead of a faded, rippling wavefront.
    const double alphaFloor = 0.02;
    double inner = std::max(0.0, innerRadius);
    double outer = outerRadius;
    if (fadingDistance > 0 && fadingFactor > 1.0)
        outer = std::min(outer, fadingDistance * std::log(1.0 / alphaFloor) / std::log(fadingFactor));
    if (outer <= inner || segments < 3)
        return Group::create();  // fully faded out, or nothing to draw
    // Adaptive radial subdivision: ~4 bands per wavelength so the ripple is resolved (bounded so a wide
    // ring doesn't explode the vertex count).
    double step = (waveLength > 0 ? waveLength : (outer - inner)) / 4.0;
    int radialBands = std::max(4, std::min(200, (int)std::ceil((outer - inner) / std::max(1.0, step))));
    float cr = (float)color.red / 255.0f, cg = (float)color.green / 255.0f, cb = (float)color.blue / 255.0f;
    int quads = radialBands * segments;
    auto vertices = vec3Array::create(quads * 6);
    auto colors = vec4Array::create(quads * 6);
    size_t k = 0;
    for (int i = 0; i < radialBands; i++) {
        double r0 = inner + (outer - inner) * i / radialBands;
        double r1 = inner + (outer - inner) * (i + 1) / radialBands;
        ::vsg::vec4 c0(cr, cg, cb, alphaAt(r0)), c1(cr, cg, cb, alphaAt(r1));
        for (int j = 0; j < segments; j++) {
            double t0 = 2.0 * M_PI * j / segments, t1 = 2.0 * M_PI * (j + 1) / segments;
            auto P = [&](double r, double t) { return ::vsg::vec3((float)(center.x + r * cos(t)), (float)(center.y + r * sin(t)), (float)center.z); };
            ::vsg::vec3 p00 = P(r0, t0), p01 = P(r0, t1), p10 = P(r1, t0), p11 = P(r1, t1);
            vertices->set(k, p00); colors->set(k++, c0);
            vertices->set(k, p10); colors->set(k++, c1);
            vertices->set(k, p11); colors->set(k++, c1);
            vertices->set(k, p00); colors->set(k++, c0);
            vertices->set(k, p11); colors->set(k++, c1);
            vertices->set(k, p01); colors->set(k++, c0);
        }
    }
    // flat (unlit) shader set, alpha blending, vsg_Color bound PER-VERTEX (so the alpha interpolates)
    auto config = GraphicsPipelineConfigurator::create(getFlatShaderSet());
    getPipelineState<InputAssemblyState>(config)->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    getPipelineState<RasterizationState>(config)->cullMode = VK_CULL_MODE_NONE;
    {
        // Same alpha-channel fix as createGeometry (see comment there): keep output alpha opaque
        // so Qt's display composite doesn't show the dark widget bg through the translucent ring.
        auto blend = getPipelineState<ColorBlendState>(config);
        blend->configureAttachments(true);
        for (auto& att : blend->attachments) {
            att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            att.alphaBlendOp = VK_BLEND_OP_ADD;
        }
    }
    auto material = PhongMaterialValue::create();
    material->value().diffuse = ::vsg::vec4(1, 1, 1, 1);
    material->value().ambient = ::vsg::vec4(1, 1, 1, 1);
    config->assignDescriptor("material", material);
    auto normals = vec3Array::create(vertices->size(), ::vsg::vec3(0, 0, 1));
    auto texCoordValue = vec2Value::create(::vsg::vec2(0, 0));
    DataList vertexArrays;
    config->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    config->assignArray(vertexArrays, "vsg_Normal", VK_VERTEX_INPUT_RATE_VERTEX, normals);
    config->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_VERTEX, colors);
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

ref_ptr<Node> createArrowhead(const Coord& start, const Coord& end, const cFigure::Color& color, double width, double height, double opacity)
{
    // The triangle is built in world space at 'end'; an AutoScaleTransform then holds its on-screen
    // size ~constant across zoom (OSG used AutoTransform autoScaleToScreen for the same effect).
    auto vertices = createArrowheadVertices(start, end, width, height);
    auto geometry = createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, color, opacity, /*lit*/ false, 1.0, /*cullBackFace*/ false);
    auto autoScale = AutoScaleTransform::create();
    autoScale->pivot = toVsgDouble(end);
    autoScale->subgraphRequiresLocalFrustum = false;  // tiny dynamic-scaled shape; skip per-node frustum
    autoScale->addChild(geometry);
    return autoScale;
}

// Dash/gap pattern (world units) for a non-solid line style; returns false for LINE_SOLID. Scaled by
// the line width so wider lines get proportionally longer dashes (mirrors cFigure's width-scaled
// stipple). Values are world-space because the off-screen path has no screen-space line stipple.
static bool dashPattern(const cFigure::LineStyle& style, double width, double& dashLen, double& gapLen)
{
    double w = std::max(1.0, width);
    switch (style) {
        case cFigure::LINE_DASHED: dashLen = 12.0 * w; gapLen = 8.0 * w; return true;
        case cFigure::LINE_DOTTED: dashLen = 2.0 * w;  gapLen = 6.0 * w; return true;
        default:                   return false;  // LINE_SOLID
    }
}

// Turn a polyline (vertices, optionally closed) into a dashed/dotted LINE_LIST: walk the path by arc
// length and emit only the "dash" intervals of the dash+gap pattern as real geometry (Vulkan core has
// no line stipple). Dashing is continuous across vertices, so circles/polylines dash smoothly.
static ref_ptr<vec3Array> dashifyVertices(ref_ptr<vec3Array> path, bool closed, double dashLen, double gapLen)
{
    double period = dashLen + gapLen;
    std::vector<::vsg::vec3> pts;
    for (size_t i = 0; i < path->size(); i++) pts.push_back(path->at(i));
    if (closed && path->size() > 1) pts.push_back(path->at(0));
    std::vector<::vsg::vec3> out;
    if (period > 1e-9) {
        double globalS = 0.0;
        for (size_t i = 0; i + 1 < pts.size(); i++) {
            ::vsg::vec3 p0 = pts[i], d = pts[i + 1] - pts[i];
            double segLen = ::vsg::length(d);
            if (segLen < 1e-9) continue;
            ::vsg::vec3 dir = d / (float)segLen;
            double localS = 0.0;
            while (localS < segLen) {
                double phase = std::fmod(globalS + localS, period);
                if (phase < dashLen) {  // inside a dash
                    double dashEnd = std::min(localS + (dashLen - phase), segLen);
                    out.push_back(p0 + dir * (float)localS);
                    out.push_back(p0 + dir * (float)dashEnd);
                    localS = (dashEnd > localS) ? dashEnd : localS + 1e-6;  // always advance
                }
                else  // inside a gap: skip to the next dash
                    localS += (period - phase);
            }
            globalS += segLen;
        }
    }
    auto result = vec3Array::create(out.size());
    for (size_t i = 0; i < out.size(); i++) result->set(i, out[i]);
    return result;
}

ref_ptr<Node> createLine(const Coord& start, const Coord& end, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style, double width, double opacity)
{
    ref_ptr<Node> line;
    double dashLen, gapLen;
    if (dashPattern(style, width, dashLen, gapLen)) {  // dotted/dashed -> real dash segments (LINE_LIST)
        auto path = vec3Array::create({ toVsg(start), toVsg(end) });
        line = createGeometry(dashifyVertices(path, false, dashLen, gapLen), {}, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, color, opacity, /*lit*/ false, width);
    }
    else {
        auto vertices = vec3Array::create({ toVsg(start), toVsg(end) });
        line = createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, color, opacity, /*lit*/ false, width);
    }
    if (!startArrowhead && !endArrowhead)
        return line;
    auto group = Group::create();
    group->addChild(line);
    if (startArrowhead)
        group->addChild(createArrowhead(end, start, color, 10.0, 20.0, opacity));
    if (endArrowhead)
        group->addChild(createArrowhead(start, end, color, 10.0, 20.0, opacity));
    return group;
}

ref_ptr<Node> createPolyline(const std::vector<Coord>& coords, cFigure::Arrowhead startArrowhead, cFigure::Arrowhead endArrowhead,
        const cFigure::Color& color, const cFigure::LineStyle& style, double width, double opacity)
{
    auto vertices = vec3Array::create(coords.size());
    for (size_t i = 0; i < coords.size(); i++)
        vertices->set(i, toVsg(coords[i]));
    ref_ptr<Node> line;
    double dashLen, gapLen;
    if (dashPattern(style, width, dashLen, gapLen))  // dotted/dashed -> real dash segments (LINE_LIST)
        line = createGeometry(dashifyVertices(vertices, false, dashLen, gapLen), {}, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, color, opacity, /*lit*/ false, width);
    else
        line = createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, color, opacity, /*lit*/ false, width);
    if ((!startArrowhead && !endArrowhead) || coords.size() < 2)
        return line;
    auto group = Group::create();
    group->addChild(line);
    if (startArrowhead)
        group->addChild(createArrowhead(coords[1], coords[0], color, 10.0, 20.0, opacity));
    if (endArrowhead)
        group->addChild(createArrowhead(coords[coords.size() - 2], coords[coords.size() - 1], color, 10.0, 20.0, opacity));
    return group;
}

ref_ptr<Node> createCircle(const Coord& center, double radius, const cFigure::Color& color,
        const cFigure::LineStyle& style, double width, int polygonSize)
{
    auto base = createCircleVertices(center, radius, polygonSize);
    double dashLen, gapLen;
    if (dashPattern(style, width, dashLen, gapLen))  // dotted/dashed -> dash around the circumference
        return createGeometry(dashifyVertices(base, /*closed*/ true, dashLen, gapLen), {}, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, color, 1.0, /*lit*/ false, width);
    // solid: close the loop (Vulkan has no LINE_LOOP) by repeating the first vertex with a LINE_STRIP
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
    auto node = getBuilder()->createSphere(gi, si);
    if (opacity < 1.0) fixBuilderBlendAlpha(node);
    return node;
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
    auto node = getBuilder()->createBox(gi, si);
    if (opacity < 1.0) fixBuilderBlendAlpha(node);
    return node;
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
    // Glyphs are laid out in the X-Y plane (horizontal +X, vertical +Y) so that when createLabel wraps
    // the text in a billboard AutoScaleTransform (which gives the subgraph an identity rotation in eye
    // space) +X maps to screen-right and +Y to screen-up — i.e. the text reads upright and facing the
    // camera. The default (Cpu) layout technique produces plain glyph geometry, which the transform can
    // freely scale; the shader-side billboard/auto-scale is unused (it has no effect off-screen).
    layout->horizontal = ::vsg::vec3(s, 0.0f, 0.0f);
    layout->vertical = ::vsg::vec3(0.0f, s, 0.0f);
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
    // Plain text at the local origin, wrapped in a billboard AutoScaleTransform so the label (a) always
    // faces the camera (reads correctly from any orbit angle, unlike the old fixed 180-deg-Z wrap that
    // mirrored from behind) and (b) holds a ~constant on-screen size regardless of zoom/scene scale
    // (== OSG AutoTransform autoScaleToScreen). 'position' becomes the world-space pivot and still
    // honors any parent transform the label is added under.
    auto autoScale = AutoScaleTransform::create();
    autoScale->pivot = toVsgDouble(position);
    autoScale->billboard = true;
    autoScale->refDistance = LABEL_AUTOSCALE_REF_DISTANCE;
    autoScale->subgraphRequiresLocalFrustum = false;
    autoScale->addChild(createText(string, Coord::ZERO, color, characterSize));
    return autoScale;
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
    // Cache loaded images by path (LEAKED, like the other VsgUtils singletons): the same icon is
    // reused by many nodes, and fade-out rebuilds (e.g. PacketDrop) would otherwise re-read it every
    // frame. Sharing the Data also lets the Builder dedup the resulting texture.
    static std::map<std::string, ref_ptr<Data>>& cache = *(new std::map<std::string, ref_ptr<Data>>());
    auto it = cache.find(fileName);
    if (it != cache.end())
        return it->second;
    auto image = read_cast<Data>(fileName, getOptions());
    if (!image)
        throw cRuntimeError("Cannot load image '%s'", fileName);
    cache[fileName] = image;
    return image;
}

ref_ptr<Data> createImageFromResource(const char *imageName)
{
    return createImage(resolveImageResource(imageName).c_str());
}

// A textured quad in the X-Y plane, centred at the origin, sized so its larger side is screenSize
// (in label units; aspect ratio preserved). Unlit + alpha-blended, two-sided. Intended to be wrapped
// in a billboard AutoScaleTransform (see createTexturedBillboard) so it faces the camera at a constant
// on-screen size — the VSG counterpart of an OSG textured-quad icon under autoScaleToScreen.
ref_ptr<Node> createTexturedQuad(ref_ptr<Data> image, double screenSize, const cFigure::Color& tint, double opacity)
{
    double w = image ? (double)image->width() : 1.0;
    double h = image ? (double)image->height() : 1.0;
    double maxDim = std::max(w, h);
    if (maxDim <= 0.0) maxDim = 1.0;
    GeometryInfo gi;
    gi.position = ::vsg::vec3(0, 0, 0);
    gi.dx = ::vsg::vec3((float)(w / maxDim * screenSize), 0, 0);
    gi.dy = ::vsg::vec3(0, (float)(h / maxDim * screenSize), 0);
    gi.dz = ::vsg::vec3(0, 0, 0);
    gi.color = toVsgColor(tint, opacity);  // multiplies the texture (alpha enables fade)
    StateInfo si;
    si.image = image;
    si.lighting = false;
    si.blending = true;
    si.two_sided = true;
    auto node = getBuilder()->createQuad(gi, si);
    fixBuilderBlendAlpha(node);  // always blended — patch the alpha-channel blend factors
    return node;
}

// A textured-quad icon that always faces the camera at a constant on-screen size (textured quad in a
// billboard AutoScaleTransform). screenSize is the larger on-screen side in label units.
ref_ptr<Node> createTexturedBillboard(ref_ptr<Data> image, const Coord& position, double screenSize, const cFigure::Color& tint, double opacity)
{
    auto autoScale = AutoScaleTransform::create();
    autoScale->pivot = toVsgDouble(position);
    autoScale->billboard = true;
    autoScale->refDistance = LABEL_AUTOSCALE_REF_DISTANCE;
    autoScale->subgraphRequiresLocalFrustum = false;
    autoScale->addChild(createTexturedQuad(image, screenSize, tint, opacity));
    return autoScale;
}

// ---------------------------------------------------------------------------------------------
// LineNode (mutable line with optional arrowheads)
// ---------------------------------------------------------------------------------------------

void LineNode::rebuild()
{
    children.clear();
    addChild(createLine(start, end, startArrowhead, endArrowhead, color, style, lineWidth, opacity));
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

void LineNode::setAlpha(double opacity)
{
    this->opacity = opacity;
    rebuild();
}

} // namespace vsg

} // namespace inet
