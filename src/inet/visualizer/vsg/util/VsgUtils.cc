//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/util/VsgUtils.h"

#include <vsgXchange/all.h>

#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>

#include "inet/common/geometry/common/PlyPointCloudReader.h"

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
// Shader-based wave ring: the OSG-equivalent. A minimal annulus band [inner, outer] whose PER-PIXEL
// opacity is computed by a GLSL fragment shader (distance fade x moving cosine ripple), so the ripple
// flows outward smoothly (waveOffset animates each frame) with no tessellation strobing. The graphics
// pipeline (shaders/blend/depth) is created ONCE and cached, so per frame we only rebuild the tiny
// vertex band + a small uniform buffer and rebind the cached pipeline — no per-frame pipeline recompile.
// ---------------------------------------------------------------------------------------------

// GLSL is compiled to SPIR-V at runtime (VSG_SUPPORTS_ShaderCompiler). The vertex shader forwards the
// local xy so the fragment shader can compute the exact radial distance per pixel. The wave parameters
// arrive in a small uniform block (packed into three vec4s).
static const char *waveRingVertexShader = R"(#version 450
layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; } pc;
layout(location = 0) in vec3 vsg_Vertex;
layout(location = 0) out vec2 vLocal;
void main() {
    vLocal = vsg_Vertex.xy;
    gl_Position = (pc.projection * pc.modelView) * vec4(vsg_Vertex, 1.0);
}
)";

static const char *waveRingFragmentShader = R"(#version 450
layout(location = 0) in vec2 vLocal;
layout(set = 0, binding = 0) uniform WaveParams {
    vec4 colorOffset;  // rgb = colour, w = waveOffset
    vec4 fadeWave;     // x = fadingFactor, y = fadingDistance, z = waveLength, w = amplitude
    vec4 bounds;       // x = innerRadius, y = outerRadius
} wp;
layout(location = 0) out vec4 fragColor;
void main() {
    // The geometry band is already [inner, outer], so no radial clipping is needed here (an earlier
    // shader-side discard on wp.bounds silently blanked everything whenever the uniform read as 0).
    float d = length(vLocal);
    float a = wp.fadeWave.w * 0.5;
    float phi = (wp.fadeWave.z > 0.0) ? (d - wp.colorOffset.w) / wp.fadeWave.z * 6.283185307179586 : 0.0;
    float fade = (wp.fadeWave.y > 0.0 && wp.fadeWave.x > 1.0) ? pow(wp.fadeWave.x, -d / wp.fadeWave.y) : 1.0;
    // Don't let the distance fade take the wavefront all the way to invisible: floor it so the outer
    // ripples stay readable, then ramp smoothly to zero only in the outermost margin so there is no
    // hard ring at the disc edge.
    fade = max(fade, 0.22);
    fade *= 1.0 - smoothstep(wp.bounds.y * 0.8, wp.bounds.y, d);
    float alpha = clamp(fade * (1.0 - a + cos(phi) * a), 0.0, 1.0);
    fragColor = vec4(wp.colorOffset.rgb, alpha);
}
)";

// The wave-ring graphics pipeline, cached (intentionally leaked, as the other shared vsg singletons).
static ref_ptr<GraphicsPipeline> getWaveRingPipeline()
{
    static ref_ptr<GraphicsPipeline>& pipeline = *(new ref_ptr<GraphicsPipeline>());
    if (pipeline) return pipeline;

    auto vertexShader = ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", waveRingVertexShader);
    auto fragmentShader = ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", waveRingFragmentShader);
    // Pre-compile GLSL -> SPIR-V now so the pipeline holds ready SPIR-V, regardless of whether the
    // off-screen viewer's own compile path would run the shader compiler on source-only stages.
    ShaderStages shaderStages{vertexShader, fragmentShader};
    auto shaderCompiler = ShaderCompiler::create();
    if (!shaderCompiler->supported() || !shaderCompiler->compile(shaderStages))
        throw cRuntimeError("signalWaveShader needs a VSG built with the GLSL compiler (glslang), and the "
                            "wave shaders must compile; set signalWaveShader=false to use the baked wavefront");

    // set 0, binding 0: the wave-parameter uniform buffer (fragment stage).
    auto descriptorSetLayout = DescriptorSetLayout::create(DescriptorSetLayoutBindings{
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    // 128 bytes of vertex push constants = projection + modelView, pushed automatically by the
    // RecordTraversal for the enclosing MatrixTransform (the standard vsg convention).
    auto pipelineLayout = PipelineLayout::create(DescriptorSetLayouts{descriptorSetLayout},
        PushConstantRanges{{VK_SHADER_STAGE_VERTEX_BIT, 0, 128}});

    // vertex input: one vec3 position per vertex.
    VertexInputState::Bindings vertexBindings{{0, sizeof(::vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}};
    VertexInputState::Attributes vertexAttributes{{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}};

    auto inputAssembly = InputAssemblyState::create();
    inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    auto rasterization = RasterizationState::create();
    rasterization->cullMode = VK_CULL_MODE_NONE;
    auto depthStencil = DepthStencilState::create();
    depthStencil->depthTestEnable = VK_TRUE;
    depthStencil->depthWriteEnable = VK_FALSE; // translucent overlay: test but don't write (no z-fighting)

    // Standard alpha blend for RGB; keep the OUTPUT alpha opaque (srcA*1 + dstA*(1-srcA)) so the
    // off-screen framebuffer isn't near-transparent for Qt's composite — same fix as createGeometry.
    auto colorBlend = ColorBlendState::create();
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.blendEnable = VK_TRUE;
    blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlend->attachments = ColorBlendState::ColorBlendAttachments{blendAttachment};

    GraphicsPipelineStates pipelineStates{
        VertexInputState::create(vertexBindings, vertexAttributes),
        inputAssembly,
        rasterization,
        MultisampleState::create(),
        depthStencil,
        colorBlend};

    pipeline = GraphicsPipeline::create(pipelineLayout, shaderStages, pipelineStates);
    return pipeline;
}

ref_ptr<Node> createWaveRingShader(const Coord& center, double innerRadius, double outerRadius,
        const cFigure::Color& color, double waveLength, double waveAmplitude, double waveOffset,
        double fadingFactor, double fadingDistance, double waveFadingFactor, int segments)
{
    double inner = std::max(0.0, innerRadius);
    double outer = outerRadius;
    // Clamp the outer radius to where the distance fade renders the wave invisible (same cutoff the
    // baked createWaveRing uses), so a light-speed wavefront doesn't try to tessellate the whole scene.
    const double alphaFloor = 0.02;
    if (fadingDistance > 0 && fadingFactor > 1.0)
        outer = std::min(outer, fadingDistance * std::log(1.0 / alphaFloor) / std::log(fadingFactor));
    if (outer <= inner || segments < 3)
        return Group::create();

    // Minimal geometry: a single annulus band [inner, outer] as a triangle strip around the ring. The
    // fragment shader paints the full radial gradient/ripple per pixel, so no radial subdivision needed.
    auto vertices = vec3Array::create((segments + 1) * 2);
    float cx = (float)center.x, cy = (float)center.y, cz = (float)center.z;
    for (int j = 0; j <= segments; j++) {
        double t = 2.0 * M_PI * (j % segments) / segments;
        float ct = (float)std::cos(t), st = (float)std::sin(t);
        vertices->set(j * 2,     ::vsg::vec3(cx + (float)inner * ct, cy + (float)inner * st, cz));
        vertices->set(j * 2 + 1, ::vsg::vec3(cx + (float)outer * ct, cy + (float)outer * st, cz));
    }

    // Wave parameters (three packed vec4s) in a per-frame uniform buffer.
    auto params = vec4Array::create(3);
    params->properties.dataVariance = DataVariance::STATIC_DATA; // rebuilt fresh each frame
    float amplitude = (float)(waveAmplitude * waveFadingFactor);
    params->set(0, ::vsg::vec4((float)color.red / 255.0f, (float)color.green / 255.0f, (float)color.blue / 255.0f, (float)waveOffset));
    params->set(1, ::vsg::vec4((float)fadingFactor, (float)fadingDistance, (float)waveLength, amplitude));
    params->set(2, ::vsg::vec4((float)inner, (float)outer, 0.0f, 0.0f));

    auto pipeline = getWaveRingPipeline();
    auto uniform = DescriptorBuffer::create(params, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    auto descriptorSet = DescriptorSet::create(pipeline->layout->setLayouts[0], Descriptors{uniform});
    auto bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, descriptorSet);

    auto stateGroup = StateGroup::create();
    stateGroup->add(BindGraphicsPipeline::create(pipeline));
    stateGroup->add(bindDescriptorSet);

    auto commands = Commands::create();
    DataList vertexArrays{vertices};
    commands->addChild(BindVertexBuffers::create(0, vertexArrays));
    commands->addChild(Draw::create(vertices->size(), 1, 0, 0));
    stateGroup->addChild(commands);
    return stateGroup;
}

// ---------------------------------------------------------------------------------------------
// Shader-based VOLUMETRIC sphere wavefront: the 3D counterpart of createWaveRingShader. The signal
// shell [inner, outer] is filled with N concentric translucent sphere shells; a GLSL fragment shader
// sets each shell's per-pixel opacity from its 3D radius (distance fade x moving cosine ripple), so
// the ripples read as an expanding, rippling glowing ball. Geometry is built at the LOCAL ORIGIN (the
// enclosing MatrixTransform places it at the emission point), so d = length(localPos) IS the true
// radial distance — no origin-vs-centre offset to worry about. One cached pipeline; per frame only the
// shell vertices + a small uniform are rebuilt. A per-shell alpha scale keeps the stack translucent.
// ---------------------------------------------------------------------------------------------

static const char *sphereWaveVertexShader = R"(#version 450
layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; } pc;
layout(location = 0) in vec3 vsg_Vertex;
layout(location = 0) out vec3 vLocal;
void main() {
    vLocal = vsg_Vertex;
    gl_Position = (pc.projection * pc.modelView) * vec4(vsg_Vertex, 1.0);
}
)";

static const char *sphereWaveFragmentShader = R"(#version 450
layout(location = 0) in vec3 vLocal;
layout(set = 0, binding = 0) uniform WaveParams {
    vec4 colorOffset;  // rgb = colour, w = waveOffset
    vec4 fadeWave;     // x = fadingFactor, y = fadingDistance, z = waveLength, w = amplitude
    vec4 bounds;       // x = innerRadius, y = outerRadius, z = per-shell alpha scale
} wp;
layout(location = 0) out vec4 fragColor;
void main() {
    float d = length(vLocal);   // 3D radial distance = this shell's radius
    float a = wp.fadeWave.w * 0.5;
    float phi = (wp.fadeWave.z > 0.0) ? (d - wp.colorOffset.w) / wp.fadeWave.z * 6.283185307179586 : 0.0;
    float fade = (wp.fadeWave.y > 0.0 && wp.fadeWave.x > 1.0) ? pow(wp.fadeWave.x, -d / wp.fadeWave.y) : 1.0;
    fade = max(fade, 0.22);
    fade *= 1.0 - smoothstep(wp.bounds.y * 0.8, wp.bounds.y, d);
    // Scale down per shell: the eye looks through many stacked shells, so a full-strength alpha would
    // blend to near-opaque. bounds.z keeps the accumulated glow translucent.
    float alpha = clamp(fade * (1.0 - a + cos(phi) * a) * wp.bounds.z, 0.0, 1.0);
    fragColor = vec4(wp.colorOffset.rgb, alpha);
}
)";

// The sphere-wavefront graphics pipeline, cached (intentionally leaked, like getWaveRingPipeline).
static ref_ptr<GraphicsPipeline> getSphereWavePipeline()
{
    static ref_ptr<GraphicsPipeline>& pipeline = *(new ref_ptr<GraphicsPipeline>());
    if (pipeline) return pipeline;

    auto vertexShader = ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", sphereWaveVertexShader);
    auto fragmentShader = ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", sphereWaveFragmentShader);
    ShaderStages shaderStages{vertexShader, fragmentShader};
    auto shaderCompiler = ShaderCompiler::create();
    if (!shaderCompiler->supported() || !shaderCompiler->compile(shaderStages))
        throw cRuntimeError("signalWaveShader needs a VSG built with the GLSL compiler (glslang), and the "
                            "wave shaders must compile; set signalWaveShader=false to use the baked wavefront");

    auto descriptorSetLayout = DescriptorSetLayout::create(DescriptorSetLayoutBindings{
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    auto pipelineLayout = PipelineLayout::create(DescriptorSetLayouts{descriptorSetLayout},
        PushConstantRanges{{VK_SHADER_STAGE_VERTEX_BIT, 0, 128}});

    VertexInputState::Bindings vertexBindings{{0, sizeof(::vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}};
    VertexInputState::Attributes vertexAttributes{{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}};

    auto inputAssembly = InputAssemblyState::create();
    inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    auto rasterization = RasterizationState::create();
    rasterization->cullMode = VK_CULL_MODE_NONE;
    auto depthStencil = DepthStencilState::create();
    depthStencil->depthTestEnable = VK_TRUE;
    depthStencil->depthWriteEnable = VK_FALSE; // translucent overlay: test but don't write

    auto colorBlend = ColorBlendState::create();
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.blendEnable = VK_TRUE;
    blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlend->attachments = ColorBlendState::ColorBlendAttachments{blendAttachment};

    GraphicsPipelineStates pipelineStates{
        VertexInputState::create(vertexBindings, vertexAttributes),
        inputAssembly, rasterization, MultisampleState::create(), depthStencil, colorBlend};

    pipeline = GraphicsPipeline::create(pipelineLayout, shaderStages, pipelineStates);
    return pipeline;
}

// Append one UV-sphere shell (radius R, centred at the local origin) as a triangle list into `out`,
// starting at vertex index k; returns the new index. latSegments x lonSegments quads, 6 verts each.
static size_t appendSphereShell(vec3Array& out, size_t k, double radius, int latSegments, int lonSegments)
{
    float R = (float)radius;
    auto P = [&](double th, double ph) {
        return ::vsg::vec3(R * (float)(std::sin(th) * std::cos(ph)),
                           R * (float)(std::sin(th) * std::sin(ph)),
                           R * (float)std::cos(th));
    };
    for (int i = 0; i < latSegments; i++) {
        double th0 = M_PI * i / latSegments, th1 = M_PI * (i + 1) / latSegments;
        for (int j = 0; j < lonSegments; j++) {
            double ph0 = 2.0 * M_PI * j / lonSegments, ph1 = 2.0 * M_PI * (j + 1) / lonSegments;
            ::vsg::vec3 p00 = P(th0, ph0), p01 = P(th0, ph1), p10 = P(th1, ph0), p11 = P(th1, ph1);
            out.set(k++, p00); out.set(k++, p10); out.set(k++, p11);
            out.set(k++, p00); out.set(k++, p11); out.set(k++, p01);
        }
    }
    return k;
}

ref_ptr<Node> createSphereWaveShader(double innerRadius, double outerRadius, const cFigure::Color& color,
        double waveLength, double waveAmplitude, double waveOffset, double fadingFactor, double fadingDistance,
        int shells, int latSegments, int lonSegments)
{
    double inner = std::max(0.0, innerRadius);
    double outer = outerRadius;
    // Same fade cutoff as the ring: cap the outer radius to where the wave becomes invisible, so a
    // light-speed wavefront doesn't tessellate the whole scene.
    const double alphaFloor = 0.02;
    if (fadingDistance > 0 && fadingFactor > 1.0)
        outer = std::min(outer, fadingDistance * std::log(1.0 / alphaFloor) / std::log(fadingFactor));
    if (outer <= inner || shells < 1 || latSegments < 2 || lonSegments < 3)
        return Group::create();

    size_t vertsPerShell = (size_t)latSegments * lonSegments * 6;
    auto vertices = vec3Array::create(vertsPerShell * shells);
    size_t k = 0;
    for (int s = 0; s < shells; s++) {
        double r = (shells == 1) ? outer : inner + (outer - inner) * s / (shells - 1);
        k = appendSphereShell(*vertices, k, r, latSegments, lonSegments);
    }

    // Per-shell alpha scale: a viewer ray crosses ~half the shells, so ~2/shells keeps the stack from
    // blending to near-opaque while still reading as a solid volume.
    double shellAlphaScale = std::min(1.0, 2.0 / std::max(1, shells));

    auto params = vec4Array::create(3);
    params->properties.dataVariance = DataVariance::STATIC_DATA;
    params->set(0, ::vsg::vec4((float)color.red / 255.0f, (float)color.green / 255.0f, (float)color.blue / 255.0f, (float)waveOffset));
    params->set(1, ::vsg::vec4((float)fadingFactor, (float)fadingDistance, (float)waveLength, (float)waveAmplitude));
    params->set(2, ::vsg::vec4((float)inner, (float)outer, (float)shellAlphaScale, 0.0f));

    auto pipeline = getSphereWavePipeline();
    auto uniform = DescriptorBuffer::create(params, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    auto descriptorSet = DescriptorSet::create(pipeline->layout->setLayouts[0], Descriptors{uniform});
    auto bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, descriptorSet);

    auto stateGroup = StateGroup::create();
    stateGroup->add(BindGraphicsPipeline::create(pipeline));
    stateGroup->add(bindDescriptorSet);
    auto commands = Commands::create();
    DataList vertexArrays{vertices};
    commands->addChild(BindVertexBuffers::create(0, vertexArrays));
    commands->addChild(Draw::create(vertices->size(), 1, 0, 0));
    stateGroup->addChild(commands);
    return stateGroup;
}

// A "loose mesh" wireframe sphere: latRings latitude circles + lonRings meridian great circles, drawn
// as a LINE_LIST in the given colour/width. Used for 3D range indicators (a radio's communication /
// interference range is a sphere, so a flat circle is meaningless once nodes leave the ground plane).
ref_ptr<Node> createWireframeSphere(const Coord& center, double radius, const cFigure::Color& color,
        double width, int latRings, int lonRings, int segments)
{
    if (radius <= 0 || segments < 3 || latRings < 2 || lonRings < 1)
        return Group::create();
    float cx = (float)center.x, cy = (float)center.y, cz = (float)center.z, R = (float)radius;
    std::vector<::vsg::vec3> pts;
    // latitude circles (horizontal, poles excluded)
    for (int i = 1; i < latRings; i++) {
        double th = M_PI * i / latRings;
        float z = R * (float)std::cos(th), r = R * (float)std::sin(th);
        for (int j = 0; j < segments; j++) {
            double a0 = 2.0 * M_PI * j / segments, a1 = 2.0 * M_PI * (j + 1) / segments;
            pts.push_back(::vsg::vec3(cx + r * (float)std::cos(a0), cy + r * (float)std::sin(a0), cz + z));
            pts.push_back(::vsg::vec3(cx + r * (float)std::cos(a1), cy + r * (float)std::sin(a1), cz + z));
        }
    }
    // meridian great circles (vertical, through the poles)
    for (int k = 0; k < lonRings; k++) {
        double phi = M_PI * k / lonRings;
        float cph = (float)std::cos(phi), sph = (float)std::sin(phi);
        auto Meridian = [&](double t) {
            float st = (float)std::sin(t), ct = (float)std::cos(t);
            return ::vsg::vec3(cx + R * st * cph, cy + R * st * sph, cz + R * ct);
        };
        for (int j = 0; j < segments; j++) {
            pts.push_back(Meridian(2.0 * M_PI * j / segments));
            pts.push_back(Meridian(2.0 * M_PI * (j + 1) / segments));
        }
    }
    auto vertices = vec3Array::create(pts.size());
    for (size_t i = 0; i < pts.size(); i++)
        vertices->set(i, pts[i]);
    return createGeometry(vertices, {}, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, color, 1.0, /*lit*/ false, width);
}

// ---------------------------------------------------------------------------------------------
// Point-cloud terrain from a PLY file (e.g. a LIDAR scan used as the scene ground). We parse the PLY
// ourselves (ascii or binary_little_endian) rather than lean on a mesh importer: LIDAR clouds have no
// faces and often no colours, and we need to recentre huge projected coordinates, fit to the scene box,
// and colour by elevation. Rendered as a coloured POINT_LIST with a tiny cached pipeline.
// ---------------------------------------------------------------------------------------------

static const char *pointCloudVertexShader = R"(#version 450
layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; } pc;
layout(location = 0) in vec3 vsg_Vertex;
layout(location = 1) in vec4 vsg_Color;
layout(location = 0) out vec4 vColor;
void main() {
    vColor = vsg_Color;
    gl_Position = (pc.projection * pc.modelView) * vec4(vsg_Vertex, 1.0);
    gl_PointSize = 3.0;   // fat enough to read as a surface (clamps to 1 without the largePoints feature)
}
)";

static const char *pointCloudFragmentShader = R"(#version 450
layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 fragColor;
void main() { fragColor = vColor; }
)";

// Cached opaque point-cloud pipeline (per-vertex position + colour, POINT_LIST, depth test + write).
static ref_ptr<GraphicsPipeline> getPointCloudPipeline()
{
    static ref_ptr<GraphicsPipeline>& pipeline = *(new ref_ptr<GraphicsPipeline>());
    if (pipeline) return pipeline;

    auto vertexShader = ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", pointCloudVertexShader);
    auto fragmentShader = ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", pointCloudFragmentShader);
    ShaderStages shaderStages{vertexShader, fragmentShader};
    auto shaderCompiler = ShaderCompiler::create();
    if (!shaderCompiler->supported() || !shaderCompiler->compile(shaderStages))
        throw cRuntimeError("sceneModel point-cloud rendering needs a VSG built with the GLSL compiler (glslang)");

    auto pipelineLayout = PipelineLayout::create(DescriptorSetLayouts{},
        PushConstantRanges{{VK_SHADER_STAGE_VERTEX_BIT, 0, 128}});
    VertexInputState::Bindings vertexBindings{
        {0, sizeof(::vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        {1, sizeof(::vsg::vec4), VK_VERTEX_INPUT_RATE_VERTEX}};
    VertexInputState::Attributes vertexAttributes{
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}};
    auto inputAssembly = InputAssemblyState::create();
    inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    auto rasterization = RasterizationState::create();
    rasterization->cullMode = VK_CULL_MODE_NONE;
    auto depthStencil = DepthStencilState::create();   // defaults: depth test + write on (opaque)
    auto colorBlend = ColorBlendState::create();
    VkPipelineColorBlendAttachmentState att{};
    att.blendEnable = VK_FALSE;                          // opaque terrain
    att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlend->attachments = ColorBlendState::ColorBlendAttachments{att};

    GraphicsPipelineStates pipelineStates{
        VertexInputState::create(vertexBindings, vertexAttributes),
        inputAssembly, rasterization, MultisampleState::create(), depthStencil, colorBlend};
    pipeline = GraphicsPipeline::create(pipelineLayout, shaderStages, pipelineStates);
    return pipeline;
}

// Terrain elevation ramp for a normalised height t in [0,1]: teal (low) -> green -> tan -> brown -> white.
static ::vsg::vec4 elevationColor(double t) {
    t = std::max(0.0, std::min(1.0, t));
    static const double stops[5] = {0.0, 0.30, 0.60, 0.85, 1.0};
    static const float rgb[5][3] = {
        {0.16f, 0.36f, 0.36f}, {0.24f, 0.55f, 0.28f}, {0.72f, 0.68f, 0.40f}, {0.55f, 0.42f, 0.32f}, {0.95f, 0.95f, 0.97f}};
    int i = 0; while (i < 4 && t > stops[i + 1]) i++;
    double f = (stops[i + 1] > stops[i]) ? (t - stops[i]) / (stops[i + 1] - stops[i]) : 0.0;
    return ::vsg::vec4((float)(rgb[i][0] + (rgb[i + 1][0] - rgb[i][0]) * f),
                       (float)(rgb[i][1] + (rgb[i + 1][1] - rgb[i][1]) * f),
                       (float)(rgb[i][2] + (rgb[i + 1][2] - rgb[i][2]) * f), 1.0f);
}

ref_ptr<Node> createTerrainFromPLY(const std::string& path, const Coord& sceneMin, const Coord& sceneMax)
{
    // shared PLY reader (also used by the physical terrain models); coordinates
    // arrive untransformed, colors (if any) already normalized to [0,1]
    PlyPointCloud cloud = PlyPointCloudReader::read(path);
    int vertexCount = cloud.getNumPoints();
    const std::vector<double>& xs = cloud.xs;
    const std::vector<double>& ys = cloud.ys;
    const std::vector<double>& zs = cloud.zs;
    bool hasRGB = cloud.hasRGB;

    // --- recentre, fit into the scene box (aspect-preserving), colour by elevation if no RGB ---
    double minX = cloud.minX, maxX = cloud.maxX, minY = cloud.minY, maxY = cloud.maxY, minZ = cloud.minZ, maxZ = cloud.maxZ;
    double cx = (minX + maxX) / 2, cy = (minY + maxY) / 2, dz = (maxZ - minZ);
    double plyW = maxX - minX, plyH = maxY - minY;
    double sceneW = sceneMax.x - sceneMin.x, sceneH = sceneMax.y - sceneMin.y;
    double scale = 1.0;
    if (plyW > 0 && plyH > 0 && std::isfinite(sceneW) && std::isfinite(sceneH) && sceneW > 0 && sceneH > 0)
        scale = std::min(sceneW / plyW, sceneH / plyH);
    double centerX = (std::isfinite(sceneMin.x) && std::isfinite(sceneMax.x)) ? (sceneMin.x + sceneMax.x) / 2 : 0.0;
    double centerY = (std::isfinite(sceneMin.y) && std::isfinite(sceneMax.y)) ? (sceneMin.y + sceneMax.y) / 2 : 0.0;
    double baseZ = std::isfinite(sceneMin.z) ? sceneMin.z : 0.0;

    auto vertices = vec3Array::create(vertexCount);
    auto colors = vec4Array::create(vertexCount);
    for (int i = 0; i < vertexCount; i++) {
        vertices->set(i, ::vsg::vec3((float)((xs[i] - cx) * scale + centerX),
                                     (float)((ys[i] - cy) * scale + centerY),
                                     (float)((zs[i] - minZ) * scale + baseZ)));
        colors->set(i, hasRGB ? ::vsg::vec4((float)cloud.rs[i], (float)cloud.gs[i], (float)cloud.bs[i], 1.0f)
                              : elevationColor(dz > 0 ? (zs[i] - minZ) / dz : 0.0));
    }

    // --- build the node (fit baked into the vertices, so no transform needed) ---
    auto stateGroup = StateGroup::create();
    stateGroup->add(BindGraphicsPipeline::create(getPointCloudPipeline()));
    auto commands = Commands::create();
    DataList arrays{vertices, colors};
    commands->addChild(BindVertexBuffers::create(0, arrays));
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
