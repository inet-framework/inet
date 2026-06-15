# VSG off-screen: transparent-over-opaque compositing bug (DEFERRED)

Status: **deferred** (2026-06-15). Worked around (opaque rendering), not fixed.
This note records what we learned so a later attempt can pick it up quickly.

## Symptom

Translucent geometry (alpha < 1) produced by the VSG visualizers — e.g. the
medium **signal wave ring** (`createWaveRing`), or any `createGeometry(... opacity<1 ...)`
— renders as if blended with the **background / clear color**, NOT with the opaque
geometry (the floor) behind it.

- Over a light camera view → a pale near-white wash.
- Over a dark camera view → a near-**black** blob (this is the "huge black ring"
  the signal showed).

So the signal could never look right with transparency; it now uses an **opaque**
wavefront band instead (see Workaround).

## It is NOT a MoltenVK / driver bug — it is cross-platform

A minimal standalone test (one 50%-alpha red quad over a green floor; see Repro):

- macOS / **MoltenVK**: renders pale "red over **white**" (= floor occluded, blend
  reads the clear color), NOT olive "red over green".
- Linux / Mesa **lavapipe** (software Vulkan, on the Parallels Ubuntu 24.04 VM):
  **identical** pale result.

Two unrelated Vulkan implementations → deterministic → the bug is in **our code**
(the custom geometry pipeline and/or the off-screen render-pass/depth setup), not a
driver quirk. (lavapipe is a non-tiled software rasterizer, which argues against any
tiled-GPU/TBDR-only explanation.)

## Key localization

VSG's **own text pipeline** (GpuLayoutTechnique / the `vsg::Text` path) blends
**correctly** in the same off-screen render pass — anti-aliased labels render fine
over the floor. So the render pass itself CAN blend. The bug is specific to **our
custom pipeline** built in `inet::vsg::createGeometry` (`src/inet/visualizer/vsg/util/VsgUtils.cc`)
via `vsg::GraphicsPipelineConfigurator` over the flat/phong shader set with
`ColorBlendState::configureAttachments(true)`.

## What we observed / tried (all via an off-screen harness rendering to PNG)

- Single 50% quad over floor → pink (floor occluded) on BOTH platforms.
- Two overlapping 50% quads over white → not the clean orange/pink the math
  predicts (anomalous accumulation).
- Toggling the custom pipeline's `DepthStencilState` (depthTestEnable /
  depthWriteEnable on/off) via `getPipelineState<DepthStencilState>(config)` →
  **byte-identical** output. The depth-state change had NO visible effect — so
  either it is not actually applied to the built pipeline, or the floor occlusion
  is via another mechanism.
- Reversing draw order (floor-first vs quad-first) → both wrong (not a simple
  ordering fix).
- Opaque geometry composites correctly (it occludes via depth overwrite, no
  dest-read). Only the **blend dest-read** path is wrong.

## Root-cause hypotheses (for the next attempt)

1. **DepthStencilState not applied**: the byte-identical result when toggling depth
   suggests our pushed `DepthStencilState` may not reach the built `VkPipeline`.
   Dump the actual pipeline depth/blend state and confirm it matches what we set.
2. **Reversed-Z mismatch**: VSG uses reversed-Z (depth cleared to 0, compareOp
   GREATER). If the custom pipeline's `depthCompareOp` doesn't match, the floor or
   quad can fail the test, so the blend ends up over the clear color.
3. **Blend factors vs shader alpha**: `configureAttachments(true)` sets standard
   src-alpha / one-minus-src-alpha; verify the flat/phong fragment output is
   straight (not premultiplied) alpha, else accumulation is wrong.
4. **Render-pass dependency**: a missing color self-dependency for read-modify-write
   blending (less likely — text blends fine in the same pass).

## How to debug next (the right tooling — now possible on Linux)

The bug reproduces on the Linux VM under lavapipe, where good GPU tooling exists:

- Capture a frame with **RenderDoc** while running the repro, and inspect:
  - the custom geometry draw's actual VkPipeline **depth + blend** state vs the
    floor draw's, and vs the (working) text pipeline's;
  - the **framebuffer color** between the floor draw and the quad draw — is the
    floor's green actually present when the quad blends?
- Run with Vulkan **validation layers**
  (`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`) — may flag a state/dependency
  problem.

## Repro (minimal, standalone — no INET needed)

`vsgoffscreen.h` is header-only, so this links against just VSG + the plugin header.
Expected if blending works: an **olive** square (red over green). The BUG renders a
pale **pink/white** square (red over the clear color).

```cpp
// transparency-repro.cc  (see also the live copy on the VM at ~/DEV/VSG-test/transparency-repro.cc)
#include <vsg/all.h>
#include <vsgXchange/all.h>
#include "vsgoffscreen.h"     // omnetpp-dev/src/qtenv/vsg/vsgoffscreen.h (header-only off-screen view)
#include <cstring>
#include <vector>
using namespace omnetpp::qtenv;
namespace vt = vsg;
static vt::ref_ptr<vt::Options> g_opt;
static vt::ref_ptr<vt::ShaderSet> g_flat;
template<typename T> static vt::ref_ptr<T> getState(vt::ref_ptr<vt::GraphicsPipelineConfigurator> c)
{ for (auto& s : c->pipelineStates) if (auto t=dynamic_cast<T*>(s.get())) return vt::ref_ptr<T>(t);
  auto t=T::create(); c->pipelineStates.push_back(t); return t; }
// mirrors inet::vsg::createGeometry (flat shader, per-instance vsg_Color, optional blend)
static vt::ref_ptr<vt::Node> quad(double cx,double cy,double z,double h,vt::vec4 color,bool blend){
  auto v=vt::vec3Array::create({ {(float)(cx-h),(float)(cy-h),(float)z},{(float)(cx+h),(float)(cy-h),(float)z},
      {(float)(cx+h),(float)(cy+h),(float)z},{(float)(cx-h),(float)(cy-h),(float)z},
      {(float)(cx+h),(float)(cy+h),(float)z},{(float)(cx-h),(float)(cy+h),(float)z} });
  auto c=vt::GraphicsPipelineConfigurator::create(g_flat);
  getState<vt::InputAssemblyState>(c)->topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  getState<vt::RasterizationState>(c)->cullMode=VK_CULL_MODE_NONE;
  if(blend) getState<vt::ColorBlendState>(c)->configureAttachments(true);
  auto m=vt::PhongMaterialValue::create(); m->value().diffuse=vt::vec4(1,1,1,1); m->value().ambient=vt::vec4(1,1,1,1);
  c->assignDescriptor("material",m);
  auto n=vt::vec3Array::create(v->size(),vt::vec3(0,0,1)); auto col=vt::vec4Value::create(color); auto tc=vt::vec2Value::create(vt::vec2(0,0));
  vt::DataList a; c->assignArray(a,"vsg_Vertex",VK_VERTEX_INPUT_RATE_VERTEX,v);
  c->assignArray(a,"vsg_Normal",VK_VERTEX_INPUT_RATE_VERTEX,n);
  c->assignArray(a,"vsg_Color",VK_VERTEX_INPUT_RATE_INSTANCE,col);
  c->assignArray(a,"vsg_TexCoord0",VK_VERTEX_INPUT_RATE_INSTANCE,tc);
  c->init(); auto sg=vt::StateGroup::create(); c->copyTo(sg);
  auto cmd=vt::Commands::create(); cmd->addChild(vt::BindVertexBuffers::create(c->baseAttributeBinding,a));
  cmd->addChild(vt::Draw::create(v->size(),1,0,0)); sg->addChild(cmd); return sg;
}
int main(){
  g_opt=vt::Options::create(); g_opt->add(vsgXchange::all::create());
  g_flat=vt::createFlatShadedShaderSet(g_opt);
  auto root=vt::Group::create();
  root->addChild(quad(150,150,-2,250,vt::vec4(0.2f,0.8f,0.3f,1),false)); // opaque green floor
  root->addChild(quad(150,150, 1, 70,vt::vec4(1,0,0,0.5f),true));        // 50% red quad over it
  VsgOffscreenView v; v.clearColor=vt::vec4(1,1,1,1); v.setScene(root); v.resize(800,500);
  v.azimuth=0.0; v.elevation=1.2; v.distanceScale=1.4; v.updateCamera();
  std::vector<uint8_t> px; if(!v.render(px)){fprintf(stderr,"render FAIL\n");return 1;}
  auto img=vt::ubvec4Array2D::create(v.extent.width,v.extent.height,vt::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM});
  std::memcpy(img->dataPointer(),px.data(),px.size()); vt::write(img,"/tmp/tr_test.png",g_opt);
  fprintf(stderr,"/tmp/tr_test.png written\n"); return 0;
}
```

### Build + run

**Linux VM** (Ubuntu 24.04, VSG at `~/DEV/VSG-test/VSG`, lavapipe):
```sh
g++ -std=c++17 transparency-repro.cc -o /tmp/tr \
  -I ~/DEV/VSG-test/VSG/include -I ~/DEV/VSG-test/omnetpp-dev/src/qtenv/vsg \
  -L ~/DEV/VSG-test/VSG/lib -lvsgXchange -lvsg -lvulkan -Wl,-rpath,$HOME/DEV/VSG-test/VSG/lib
VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json /tmp/tr   # writes /tmp/tr_test.png
```

**macOS** (VSG at `~/VSG`, MoltenVK):
```sh
clang++ -std=c++17 transparency-repro.cc -o /tmp/tr \
  -I $HOME/VSG/include -I /opt/homebrew/include -I $HOME/DEV/omnetpp-dev/src/qtenv/vsg \
  -L $HOME/VSG/lib -lvsgXchange -lvsg -L /opt/homebrew/opt/vulkan-loader/lib -lvulkan \
  -Wl,-rpath,$HOME/VSG/lib -Wl,-rpath,/opt/homebrew/lib
VK_ICD_FILENAMES=/opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json \
  DYLD_LIBRARY_PATH=$HOME/VSG/lib:/opt/homebrew/lib /tmp/tr
```

(Exit code 139 on teardown is a separate, known process-exit issue — ignore it once
`/tmp/tr_test.png` is written.)

## Current workaround (in tree)

`MediumVsgVisualizer::refreshRingTransmissionNode` draws the signal as **opaque**
wavefront bands (`createCircle` / `createAnnulus` at opacity 1.0), not a translucent
faded disc. The faded, wave-modulated `createWaveRing` remains in `VsgUtils` for when
this bug is fixed. Other alpha-fade effects (data-link / mobility-trail / packet-drop
fade-outs) are subtly affected but use thin geometry, so the artifact is not obvious.

## Affected code

- `src/inet/visualizer/vsg/util/VsgUtils.cc` — `createGeometry` (the custom pipeline);
  `createWaveRing` (the faded signal, currently unused).
- `omnetpp-dev/src/qtenv/vsg/vsgoffscreen.h` — `createOffscreenRenderPass`, `buildOnce`
  (the off-screen render pass / View / depth attachment, reversed-Z clear).
