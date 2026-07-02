# INET VSG (VulkanSceneGraph) 3D Visualizer

The VSG visualizer is INET's 3D network-simulation visualizer built on
[VulkanSceneGraph](https://vsg-dev.github.io/vsg-dev.io/) (VSG) — a modern,
OSG-free replacement for INET's older OpenSceneGraph-based visualizer
(`inet/visualizer/osg`). It renders the whole simulated topology (nodes,
connections, radio signals, mobility trails, routes, statistics, ...) into an
**off-screen** framebuffer, which OMNeT++'s Qtenv hands to its 3D canvas
widget for display. There is no on-screen Vulkan window of its own; VSG is
purely a rendering backend behind Qtenv's existing 3D view. The module layout
deliberately mirrors the OSG visualizer package-for-package, so anyone
familiar with `inet.visualizer.osg` will recognize the structure immediately.

This visualizer lives on the `topic/inet-vsg` development branch and is
enabled via the `VisualizationVsg` INET feature (mutually exclusive with
`VisualizationOsg` — OMNeT++ is built with either OSG or VSG support, not
both).

## Architecture

### Module map

The `inet/visualizer/vsg/` tree mirrors `inet/visualizer/osg/` (and the 2D
canvas visualizer) one folder per protocol layer / concern:

| Folder | Contents |
|---|---|
| `base/` | Shared base classes: `SceneVsgVisualizerBase`, `LinkVsgVisualizerBase`, `PathVsgVisualizerBase` |
| `common/` | `GateScheduleVsgVisualizer`, `InfoVsgVisualizer`, `PacketDropVsgVisualizer`, `QueueVsgVisualizer`, `StatisticVsgVisualizer` |
| `scene/` | `SceneVsgVisualizer` (floor/axes/viewpoint), `NetworkNodeVsgVisualizer`/`NetworkNodeVsgVisualization` (per-node markers, icons, labels), `NetworkConnectionVsgVisualizer` |
| `physicallayer/` | `MediumVsgVisualizer` (signals/ranges — see below), `RadioVsgVisualizer`, `PhysicalLinkVsgVisualizer`, `TracingObstacleLossVsgVisualizer` |
| `linklayer/` | `DataLinkVsgVisualizer`, `Ieee80211VsgVisualizer`, `InterfaceTableVsgVisualizer`, `LinkBreakVsgVisualizer` |
| `networklayer/` | `NetworkRouteVsgVisualizer`, `RoutingTableVsgVisualizer` |
| `transportlayer/` | `TransportConnectionVsgVisualizer`, `TransportRouteVsgVisualizer` |
| `mobility/` | `MobilityVsgVisualizer` (position updates, movement trails) |
| `power/` | `EnergyStorageVsgVisualizer` |
| `flow/` | `PacketFlowVsgVisualizer` |
| `environment/` | `PhysicalEnvironmentVsgVisualizer` (obstacles/terrain shapes) |
| `integrated/` | `IntegratedVsgVisualizer` / `IntegratedMultiVsgVisualizer` — the "one module wires up everything" convenience wrappers, analogous to `IntegratedVisualizer`/`IntegratedCanvasVisualizer` |
| `util/` | `VsgUtils.{h,cc}` (geometry/shader toolkit) and `VsgScene.{h,cc}` (scene-root plumbing) — shared by every visualizer above |

Every concrete visualizer follows the same INET convention: a `.ned` file
declaring the module and its parameters (`like I<Foo>Visualizer`), a `.h`/`.cc`
pair implementing it, typically extending a shared `<Foo>VisualizerBase` from
`inet/visualizer/base/` (the layer/concern-specific logic — event
subscriptions, parameter parsing, filtering — is shared with the 2D canvas and
OSG visualizers; only the actual node-building code is VSG-specific).

### Off-screen rendering and Qtenv integration

The VSG scene graph is rooted at a `inet::vsg::TopLevelScene` (a
`vsg::Group` subclass, see `util/VsgScene.h`), which in turn holds a
`SimulationScene` group that all the concrete visualizers add their nodes
into. `TopLevelScene::getSimulationScene(cModule *module)` is the standard
entry point every visualizer calls: it fetches (or lazily creates) the scene
root that OMNeT++'s `cOsgCanvas` wraps in an opaque `omnetpp::cScene3DNode`.
Under `WITH_VSG`, `cOsgCanvas` is the same OMNeT++ 3D-canvas API the OSG
visualizer uses — VSG just supplies a different backend for it
(`qtenv/vsg/vsgscenehandle.h`). This is why the module names/parameters keep
the historical "Osg" nomenclature in a few OMNeT++-side APIs (`getOsgCanvas()`
etc.) even though the rendering underneath is pure VSG.

When no `SceneVsgVisualizer` exists yet, `getSimulationScene()` creates the
scene with sensible defaults (`clearColor = #FFFFFF`, `zNear = 0.1`,
`zFar = 100000`, `CAM_TERRAIN` manipulator) so any single visualizer module
can be dropped into a network and "just work" without requiring the full
`IntegratedVsgVisualizer`.

VSG's off-screen viewer performs the single-`View` compile and supplies
lighting; `util/VsgUtils.cc` only builds the scene graph nodes (geometry +
graphics pipelines) that get attached under that root.

## The `util/` toolkit

`util/VsgUtils.{h,cc}` is the shared geometry/shader helper library that
every concrete visualizer builds its nodes with — the VSG counterpart of
`OsgUtils.h`. Because VSG (unlike OSG's fixed-function-friendly
`Geometry`+`StateSet` split) requires an explicit graphics pipeline for
*everything*, each helper returns a ready-to-render node: a `vsg::StateGroup`
that bundles its own pipeline, descriptor bindings, and draw commands.

Key entry points:

- **`createGeometry(vertices, normals, topology, color, opacity, lit, lineWidth, cullBackFace, depthTest)`**
  — the foundational helper. Builds a `GraphicsPipelineConfigurator` over the
  shared flat-shaded (`getFlatShaderSet()`) or Phong (`getPhongShaderSet()`)
  shader set, sets topology/rasterization/blend/depth state, binds a
  per-instance `vsg_Color`, and emits the `BindVertexBuffers`+`Draw` commands.
  Nearly every other helper (`createLine`, `createCircle`, `createAnnulus`,
  `createQuad`, `createPolygon`, `createArrowhead`, `createWireframeSphere`,
  ...) is built on top of it.
- **`createCircle` / `createAnnulus`** — flat 2D shapes (range indicators,
  filled discs), with `createCircle` honoring `cFigure::LineStyle` via real
  dash geometry (`dashifyVertices` walks the path by arc length and emits only
  the "dash" intervals — Vulkan core has no line-stipple equivalent).
- **`createWireframeSphere(center, radius, color, width, latRings, lonRings, segments)`**
  — a "loose mesh" wireframe sphere (latitude rings + meridian great circles,
  drawn as `VK_PRIMITIVE_TOPOLOGY_LINE_LIST`), used for 3D range indicators
  where a radio's range is truly a sphere and a flat circle would be
  misleading (e.g. a flying drone).
- **`createSphere` / `createBox`** (and `createTexturedQuad`) — thin wrappers
  around the VSG `Builder` (`getBuilder()->createSphere/createBox/createQuad`),
  which bakes its own pipeline internally.
- **`createWaveRing(center, innerRadius, outerRadius, color, waveLength, waveAmplitude, waveOffset, fadingFactor, fadingDistance, waveFadingFactor, segments)`**
  — the **baked** signal wavefront: a radially-subdivided annulus whose
  *per-vertex* alpha reproduces the OSG signal shader's formula (a distance
  fade `pow(fadingFactor, -d/fadingDistance)` times a cosine ripple across the
  radius). Rebuilt from scratch every display refresh with the current
  radii/offset — geometry, not a shader.
- **`createWaveRingShader(...)`** — the **shader** counterpart: a minimal
  `[inner, outer]` annulus band whose *per-pixel* opacity is computed by a
  cached GLSL fragment shader, so the ripple flows smoothly with no
  tessellation strobing and no per-frame pipeline rebuild (only the tiny
  vertex band + a small uniform buffer are rebuilt each frame). **Contract:**
  `center.xy` must be `(0,0)` — position the ring via an enclosing
  `MatrixTransform` — because the shader derives the radial distance as
  `length(vertex.xy)`; only `center.z` (a ground-lift) is honored.
- **`createSphereWaveShader(innerRadius, outerRadius, color, waveLength, waveAmplitude, waveOffset, fadingFactor, fadingDistance, shells, latSegments, lonSegments)`**
  — the 3D/volumetric counterpart: fills the `[inner, outer]` shell with `N`
  concentric translucent UV-sphere shells, each shell's opacity computed by
  the same per-pixel fade × ripple formula (now driven by the true 3D radius
  `length(localPos)`), so it reads as an expanding, rippling glowing ball. A
  per-shell alpha scale (`~2/shells`) keeps the stack translucent instead of
  blending to opaque as the eye ray crosses many shells.
- **Cached, intentionally-leaked singletons**: `getOptions()`,
  `getFlatShaderSet()`/`getPhongShaderSet()`, `getBuilder()`,
  `getWaveRingPipeline()`, `getSphereWavePipeline()` are all lazily created
  `static ref_ptr<T>&` heap objects that are **never destroyed**. This is
  deliberate: destroying a VSG object during C++ static-destructor/process-exit
  teardown crashes, because VSG's global allocator may already be torn down
  by then (`vsg::deallocate` → null deref). Leaking these avoids the crash;
  the OS reclaims the memory at process exit.
- **`fixBuilderBlendAlpha(node)`** — patches the alpha-channel blend factors
  of pipelines baked by the VSG `Builder` (see the compositing fix below).
- **`AutoScaleTransform`** — a `vsg::Transform` subclass that recomputes its
  matrix from the live modelview during the record traversal, reproducing
  `osg::AutoTransform`'s screen-relative behavior (VSG 1.1 has no equivalent
  node, and the shader-side `StandardLayout::billboard` doesn't work
  off-screen). In `billboard` mode, children face the camera and hold a
  ~constant on-screen size (used for node icons, name labels, and
  annotations, which are stacked via `screenOffset` so they never overlap
  regardless of zoom); in non-billboard mode orientation is preserved (used
  for arrowheads).
- **`createTexturedQuad` / `createTexturedBillboard`** — icon rendering (see
  Node rendering below), and `createImage`/`createImageFromResource` — a
  path-keyed, leaked image cache so repeated icon loads (e.g. fade-out
  rebuilds) don't re-read from disk every frame.

### The translucent-over-opaque compositing fix

This was the first major VSG-visualizer bug fixed (merged as PR #1112,
`fix/vsg-translucent-compositing-v2`): any translucent geometry (signal
discs, range fills, fading packet-drop markers, ...) rendered as an opaque
**black blob** instead of blending into the scene.

**Root cause**: VSG's standard `ColorBlendState::configureAttachments(true)`
sets up the expected RGB blend (`srcColor·srcAlpha + dstColor·(1−srcAlpha)`)
but sets the **alpha channel** blend to `srcAlpha·srcAlpha + dstAlpha·0`
— i.e. output alpha ≈ `srcAlpha²`, which is close to 0 for any translucent
draw. The off-screen Qtenv backend hands its framebuffer to Qt as a `QImage`
with a real per-pixel alpha channel; a near-0 output alpha means Qt composites
its own (dark) widget background *over* the correctly-blended RGB, so the
translucent shape renders as a solid black/dark shape.

**Fix**: wherever alpha blending is enabled, explicitly override the
alpha-channel blend factors to keep the *output* alpha opaque:
`srcAlphaBlendFactor = ONE`, `dstAlphaBlendFactor = ONE_MINUS_SRC_ALPHA`,
`alphaBlendOp = ADD` — i.e. `outA = srcA·1 + dstA·(1−srcA)`, which stays 1
whenever the destination alpha is already 1 (an opaque background). This is
applied in four places:

- `createGeometry` (the generic pipeline path, whenever `opacity < 1.0`),
- `createWaveRing` (the baked wavefront),
- the two shader pipelines (`getWaveRingPipeline`, `getSphereWavePipeline`) —
  set directly on the `ColorBlendState` at pipeline-creation time,
- `fixBuilderBlendAlpha()` — for nodes baked by the VSG `Builder`
  (`createSphere`/`createBox`/`createTexturedQuad`), which bakes its
  `VkPipeline` internally during `createSphere()`/etc., so the fix has to
  clone the `ColorBlendState` (Builder may share/cache the same
  `ColorBlendState` `ref_ptr` across pipelines — don't mutate in place),
  patch the alpha factors on the clone, and rebuild+swap the
  `BindGraphicsPipeline` in the returned scene graph via a `vsg::Visitor`
  that finds every `BindGraphicsPipeline` in the subgraph.

## Signal / medium visualization (`physicallayer/MediumVsgVisualizer`)

`MediumVsgVisualizer` is the VSG port of `MediumOsgVisualizer` — it displays
radio signals as they propagate through the medium, plus per-radio departure/
arrival markers and communication/interference range indicators.

### Ring vs. sphere

The `signalShape` parameter (`"ring"` | `"sphere"` | `"both"`, default
`"ring"`) selects the signal representation:

- **`ring`** (`createRingSignalNode` / `refreshRingTransmissionNode`) — a flat
  2D annulus, positioned at the transmission's start point via a
  `MatrixTransform`, tilted per `signalPlane` (`"xy"` default/ground plane,
  `"xz"`/`"yz"` vertical planes; `"camera"` behaves like `"xy"` since a live
  camera-facing billboard for a world-sized growing ring has no off-screen
  equivalent).
- **`sphere`** (`createSphereSignalNode` / `refreshSphereTransmissionNode`) —
  a true 3D volumetric wavefront centered at the transmitter's 3D position;
  meaningful for nodes off the ground plane (drones).
- **`both`** — draws one ring node and one sphere node per transmission.

### Baked vs. shader

Independently of the shape, `signalWaveShader` (default `false`) switches
between two rendering strategies:

| | **Baked (default)** | **Shader (`signalWaveShader = true`)** |
|---|---|---|
| Ring | `inet::vsg::createWaveRing` — per-vertex alpha, whole annulus geometry rebuilt every display refresh | `inet::vsg::createWaveRingShader` — cached GLSL pipeline, per-pixel fade × ripple, only a thin band + tiny uniform rebuilt per frame |
| Sphere | `inet::vsg::createSphere` ×2 (start/end wavefront), single alpha per sphere, rebuilt every refresh | `inet::vsg::createSphereWaveShader` — cached GLSL pipeline, concentric shells with per-pixel fade × ripple |
| Ripple flow | Fixed `waveOffset = 0` when expanding (a moving offset would strobe against the coarse radial tessellation); `waveFadingFactor` damps the ripple toward a flat disc as animation speed rises | Ripple flows continuously; `waveFadingFactor` is pinned to `1.0` (never strobes, since the offset advances in **animation time**, not simulation time) |
| Compile cost | None (plain geometry) | GLSL compiled to SPIR-V **at runtime** via `VSG_SUPPORTS_ShaderCompiler` (glslang); the pipeline is built once and cached (`getWaveRingPipeline`/`getSphereWavePipeline`) |

If the shader path is requested but the VSG build lacks the GLSL compiler (or
compilation fails), `getWaveRingPipeline()`/`getSphereWavePipeline()` throw a
clear `cRuntimeError`:
`"signalWaveShader needs a VSG built with the GLSL compiler (glslang), ... set signalWaveShader=false to use the baked wavefront"`.

Both paths share the same **fade cap**: the drawn radius is bounded not just
by the transmitter's interference range but also by the radius at which the
exponential distance fade drops below a floor (`alphaFloor = 0.02`,
i.e. `signalFadingDistance · ln(1/0.02) / ln(signalFadingFactor)`). This
decouples the disc/sphere's *on-screen size* from the *transmit power*: a
power high enough for real communication (so the data-link visualizer draws
connection arrows) has an interference range many times larger than the
communication range, and without this cap the wavefront would balloon past
the whole scene. The cap is purely a visualization choice governed by
`signalFadingDistance`/`signalFadingFactor`.

The ring path also lifts the disc a hair above the ground plane
(`z = clamp(maxRadius * 0.02, 0.5, 3.0)`) so its coplanar triangles don't
z-fight the floor.

### The ripple ↔ animation-time relationship

In the shader path, the ripple offset is driven by **animation time**
(`getSimulation()->getEnvir()->getAnimationTime()`) at a fixed visual rate
(`ripplesPerAnimSecond = 1.2`, wrapped to one wavelength), **not** by the
light-speed leading-edge radius. Anchoring the ripple to `startRadius` made
it race past too fast to see during the long "holding while sending" window;
driving it from animation time makes the waves flow outward steadily for the
whole transmission and then fade out as the shape shrinks/empties.

### Signal parameters (`physicallayer/MediumVsgVisualizer.ned`)

| Parameter | Default | Meaning |
|---|---|---|
| `signalShape` | `"ring"` | `"ring"` \| `"sphere"` \| `"both"` |
| `signalPlane` | `"xy"` | Plane for 2D signal shapes: `"camera"`, `"xy"`, `"xz"`, `"yz"` |
| `signalFadingDistance` | `100m` | Distance scale of the exponential opacity/fade-cap decay |
| `signalFadingFactor` | `1.5` | Exponential decay base, must be `> 1.0` |
| `signalWaveLength` | `100m` | Distance between ripple wave crests |
| `signalWaveAmplitude` | `0.5` | Relative opacity amplitude of the ripple, `[0, 1]` |
| `signalWaveFadingAnimationSpeedFactor` | `1.0` | Baked-path only: tunes how fast the ripple flattens toward a smooth disc as animation speed rises |
| `signalWaveShader` | `false` | Use the GLSL shader path instead of baked geometry (experimental) |
| `rangeSphere` | `false` | Draw range indicators as 3D wireframe spheres instead of flat ground circles |
| `transmissionPlane` / `receptionPlane` | `"camera"` | Plane for the departure/arrival indicator icons |
| `communicationRangePlane` / `interferenceRangePlane` | `"xy"` | Plane for the range circles |

Inherited from `MediumVisualizerBase.ned` (shared with the OSG/canvas
visualizers) — the notable animation-speed knobs:

| Parameter | Default | Meaning |
|---|---|---|
| `signalPropagationAnimationSpeed` | `nan` (auto) | Animation speed while the leading/trailing edge is propagating (sweeping) |
| `signalPropagationAnimationTime` | `1s` | Used instead, when `signalPropagationAnimationSpeed` is unset |
| `signalTransmissionAnimationSpeed` | `nan` (auto) | Animation speed while the signal is actively being transmitted (i.e. how long the disc/sphere stays lit while "sending") |
| `signalTransmissionAnimationTime` | `1s` | Used instead, when `signalTransmissionAnimationSpeed` is unset |

### Animation-speed model in practice

Two independent knobs govern how a transmission *feels*:

- `signalPropagationAnimationSpeed` controls how fast the wavefront's
  **edge sweeps** outward.
- `signalTransmissionAnimationSpeed` controls how long the disc/sphere
  **stays lit** while the node is actively sending (the gap between the
  leading and trailing edge).

Proven values from the `radiomediumactivity` showcase (also used by
`vsgsignalwave`/`vsgsignalwave3d`, see below):

```
signalPropagationAnimationSpeed  = 500/3e8     # ~175 m wavefront expands in ~0.35s
signalTransmissionAnimationSpeed = 50000/3e8   # keeps a ~0.4ms transmission lit for ~2s
```

i.e. the transmission-hold speed is ~100× slower than the propagation-sweep
speed, so a wavefront visibly grows to its full size and then holds/pulses
for a watchable duration before fading, instead of flashing once and
vanishing (real propagation is at light speed — sub-microsecond for
in-scene distances).

## Node / range rendering

- **Node representation** (`scene/NetworkNodeVsgVisualization.cc`): each
  network node's display-string `i=` icon tag is loaded
  (`inet::vsg::createImage`/`resolveImageResource`) and rendered as a
  camera-facing, constant-on-screen-size **textured billboard quad**
  (`createTexturedQuad` wrapped in a billboard `AutoScaleTransform`), with a
  colored box (`createBox`) fallback if there is no icon or it fails to
  load. There is currently **no support for real 3D models** — the code has
  an explicit `TODO: 3D model from the "osgModel"/"model" parameter`, unlike
  the OSG visualizer's optional model-file loading. Every node also gets a
  selectable id (`omnetpp.object` user value) so clicking it in the 3D view
  selects the corresponding module.
- **Name label + annotations**: stacked as camera-facing, screen-constant-size
  billboards sharing one `labelPivot`, offset from each other purely in
  *screen* space (`screenOffset.y`) via `updateAnnotationPositions()` — so the
  stack (name, then per-visualizer annotations such as the signal
  departure/arrival marker) never overlaps regardless of zoom level, mirroring
  how OSG stacks annotations inside its `autoScaleToScreen` transform.
- **Range indicators**: `displayCommunicationRanges` / `displayInterferenceRanges`
  draw a circle (`createCircle`, honoring line color/style/width, including
  dashed/dotted via real dash geometry) around each radio by default. When
  `rangeSphere = true`, a 3D wireframe sphere (`createWireframeSphere`) is
  drawn instead — a radio's range is physically a sphere, so a flat ground
  circle is misleading once nodes leave the ground plane (e.g. a drone).
- **Icons in 3D examples**: drones and antenna towers are just ordinary
  display-string icons — `@display("i=misc/drone")` /
  `@display("i=device/antennatower")` — rendered through the same textured
  billboard path as any other node icon; there is nothing sphere/drone-specific
  in the node-rendering code, only in how the examples position/label nodes.

## Scene terrain from a point cloud (`sceneModel`)

By default the scene ground is a flat colored quad (`SceneVsgVisualizerBase::createSceneFloor` →
`createQuad`, color `sceneColor`, at `z ≈ 0`). Setting the scene parameter **`sceneModel`** to a PLY
file path replaces that quad with a real 3D terrain loaded from the file — e.g. a LIDAR scan — so
nodes fly over actual landscape instead of a flat plane:

```ini
*.visualizer.sceneVisualizer.sceneModel = "terrain.ply"
```

The path is resolved **relative to the working directory** (the example folder when you run
`inet -u Qtenv omnetpp.ini` there); an absolute path also works. If the file can't be opened the
simulation stops with a clear `cRuntimeError` instead of silently drawing nothing.

**How it works** (`inet::vsg::createTerrainFromPLY`, `util/VsgUtils.cc`) — the PLY is parsed directly
(no mesh importer), which keeps it general and lets us handle raw LIDAR:

- **Any PLY point cloud** — ascii or `binary_little_endian`, any property layout. It reads the
  `element vertex` block, locates `x`/`y`/`z` by name (and `red`/`green`/`blue` if present), and skips
  the other properties by their declared byte sizes. Faces are ignored — it renders points, not a mesh.
- **Coordinate fit** — LIDAR tiles use huge projected coordinates (e.g. UTM easting/northing in the
  millions). The cloud is **recentered** to its own centroid and **aspect-preserving-scaled to fit the
  configured scene bounds** (`sceneMinX..sceneMaxZ`), base sitting at `sceneMinZ`. So any PLY, at any
  real-world scale, drops into the scene box — set the scene bounds to roughly the tile's footprint
  aspect for a natural ~1:1 look.
- **Color** — the file's `red/green/blue` if present, otherwise points are colored by **elevation**
  (a teal → green → tan → brown → white height ramp), which reads as a heightmap.
- **Rendering** — a colored `POINT_LIST` through a small cached custom pipeline
  (`getPointCloudPipeline`) that sets `gl_PointSize` so the points read as a surface rather than 1px
  specks (point size clamps to 1 on GPUs without the `largePoints` feature). Loaded once at scene init.

This is separate from per-*node* 3D models (still a TODO — nodes are icon billboards); `sceneModel` is
specifically the scene ground/terrain, and works with **any** `.ply` point cloud, not a specific file.

**Example — `vsglidar`** (see below): a 5-drone swarm flying over a subsampled USGS Hurricane-Sandy
LIDAR tile of New York (`terrain.ply`, buildings included). To reuse it, drop any other `.ply` point
cloud into an example folder and point `sceneModel` at it.

## Examples (`examples/visualizer/`)

| Example | Demonstrates |
|---|---|
| `vsgsmoketest` | Minimal end-to-end smoke test: `SceneVsgVisualizer` + `NetworkNodeVsgVisualizer` + `NetworkConnectionVsgVisualizer` + `MobilityVsgVisualizer` wired up individually (no `IntegratedVsgVisualizer`), on a hand-built `VsgSmokeNode` with no protocol stack. A `Mobility` config variant adds moving nodes + movement trails. |
| `vsgicons` | Infrastructure WiFi (`AccessPoint` + associating stations) exercising Ieee80211 SSID badges, a TCP connection flow (transport-connection icons), a UDP flood causing packet drops, plus node icons/labels, data links, routes, communication ranges (dashed-line style), and wave-modulated medium signals with a custom `signalColor` palette. |
| `vsgsignal` | Tuning ground for the 2D ring signal visualization: three ad-hoc hosts close enough to actually communicate (so data-link arrows also draw), with `signalFadingDistance`/`signalWaveLength` tuned so the translucent discs fit the 300×300 scene; one host drifts to show growing transmissions as nodes move. |
| `vsgsignal3d` | Same idea in 3D: `signalShape = "sphere"`, `rangeSphere = true`; two ground-station hosts plus a "drone" (`i=misc/drone`) flying through the volume above them via 3D `LinearMobility` (non-zero movement elevation), so its transmission spheres are visibly centered in the air. |
| `vsgsignalwave` | Showcases the GLSL **shader** ring wavefront (`signalWaveShader = true`) with the proven `signalPropagationAnimationSpeed`/`signalTransmissionAnimationSpeed` pair from `radiomediumactivity`; a hub + 4 spokes all transmit so several color-coded, continuously-rippling discs overlap and pulse at once. |
| `vsgsignalwave3d` | The volumetric 3D counterpart: `signalShape = "sphere"` + `signalWaveShader = true` + `rangeSphere = true`; a ground control station plus a 4-drone swarm flying through a 3D box, each transmission an expanding, rippling glowing sphere in the air. |
| `vsgwireless` | A comprehensive "everything at once" checkpoint: signals, data links, network routes, interface tables, statistics, and mobility trails, all through `IntegratedVsgVisualizer`, on a simple 3-host ad-hoc network (settings proven by the `radiomediumactivity` showcase). |
| `vsglidar` | A **LIDAR point-cloud terrain** as the scene ground (`sceneVisualizer.sceneModel = "terrain.ply"`, a subsampled USGS Sandy scan of New York, elevation-colored): a 5-drone swarm flies over the landscape exchanging UDP, each transmission a volumetric shader sphere; range indicators off to keep the view clean. Shows the general `sceneModel` mechanism (works with any `.ply`). |

All examples import `inet.visualizer.vsg.integrated.IntegratedVsgVisualizer`
(except `vsgsmoketest`, which wires up individual VSG visualizer modules to
demonstrate that they work standalone) and are meant to be run under Qtenv,
switching to the 3D view, and pressing Run.

## Building & running

- Requires a VSG-enabled OMNeT++ build (in this environment: the
  `omnetpp-dev` checkout, configured/built with VSG support,
  i.e. `WITH_VSG=yes`).
- INET side: the `VisualizationVsg` feature (`.oppfeatures`, id
  `VisualizationVsg`) must be enabled; it compiles with
  `-DINET_WITH_VISUALIZATIONVSG`, requires the `PhysicalEnvironment` and
  `VisualizationCommon` features, and is **mutually exclusive** with
  `VisualizationOsg` — an OMNeT++ build has either OSG or VSG support, never
  both, so the two visualizer features cannot be enabled simultaneously.
- Build with `make MODE=release` (from the INET root, after
  `opp_featuretool` / configure has enabled `VisualizationVsg`).
- Run an example with `inet -u Qtenv` from its directory (e.g.
  `examples/visualizer/vsgsignalwave3d`), then switch to the 3D view in the
  Qtenv toolbar and press Run.

### Gotchas

- **Only one VSG Qtenv window at a time.** Two simultaneously open VSG 3D
  views (e.g. two separate Qtenv instances) conflict over the shared
  off-screen VSG device/resources and can render black. Close one before
  opening another.
- **A harmless startup log pair** is expected and not an error:
  ```
  dlopen liboppqtenv-vsg.dylib ...
  Loaded 3D viewer backend
  ```
  This is OMNeT++ dynamically loading the VSG-based Qtenv 3D-viewer backend
  plugin; it is the normal way the VSG support library gets pulled in.

## Limitations & TODOs

- **No real 3D node models yet** — nodes render as 2D icon billboards
  (`NetworkNodeVsgVisualization.cc`), not 3D meshes; a `TODO` marks the
  planned `osgModel`/`model` display-string support.
- **Per-frame geometry rebuild cost for the baked wavefront** — the default
  (non-shader) ring/sphere paths rebuild their annulus/sphere geometry from
  scratch on every `refreshDisplay()` call (VSG bakes geometry+color at
  construction time; there's no cheap "just change the alpha" update path),
  which is why the shader path exists as a lower-overhead, smoother-looking
  alternative.
- **Per-instance opacity has no cheap update path** — several visualizers
  (`PacketDropVsgVisualizer`, sphere signal wavefronts, `LinkBreakVsgVisualizer`)
  note that a fade-out currently rebuilds the whole geometry child at the new
  alpha; a push-constant or descriptor-set alpha uniform would be more
  efficient (`TODO`s at each site).
- **No VSG node-mask equivalent** — `MediumVsgVisualizer`'s departure/arrival
  indicators are shown/hidden by adding/clearing child nodes rather than
  toggling visibility, since VSG has no OSG-style node mask.
  (`MediumVsgVisualizer.cc`)
  - Departure/arrival indicators are also still plain colored spheres, not
    the textured billboard (`signalDepartureImage`/`signalArrivalImage`) the
    OSG visualizer draws — `TODO`s mark the planned switch to
    `createTexturedBillboard`.
- **Label position is baked, not mutable** — the per-transmission ring
  label's world position is set once at construction; the code notes this
  needs a mutable-position API on `createLabel`/`VsgUtils` before it can
  track the moving edge each frame (`MediumVsgVisualizer.cc`).
- **Line width > 1 needs the `wideLines` device feature**, which is absent
  on MoltenVK (macOS); requested widths silently clamp to 1px there
  (`VsgUtils.h`).
- **Line stipple (dotted/dashed) has no Vulkan-core equivalent** — emulated
  by generating real dash-segment geometry in world-space units
  (`dashifyVertices` in `VsgUtils.cc`), rather than a true screen-space
  stipple.
- **`sceneModel` terrain is points-only** — a PLY loaded via `sceneModel`
  renders as a `POINT_LIST` (`createTerrainFromPLY`), not a meshed/triangulated
  surface; point size relies on `gl_PointSize`, which clamps to 1px on GPUs
  without the `largePoints` feature (a fallback to world-space point quads would
  be more robust for sparse clouds). Only PLY is parsed — ascii /
  `binary_little_endian`, `x`/`y`/`z` + optional `red`/`green`/`blue`; other
  formats and `binary_big_endian` aren't handled, and the whole cloud is read
  into memory once at scene init.
- Assorted per-visualizer approximations, each flagged with an inline `TODO`
  in its `.cc`: `EnergyStorageVsgVisualizer` (unimplemented, same as the OSG
  original), `GateScheduleVsgVisualizer` (no gantt-style timeline bar yet),
  `QueueVsgVisualizer` (box + label instead of a proper bar figure),
  `RadioVsgVisualizer` (no radio-mode/rx-tx state icons yet),
  `RoutingTableVsgVisualizer` (route-line labels not yet rebuilt on refresh),
  `SceneVsgVisualizerBase` (textured floor image and exponential edge shading
  not yet ported), `PhysicalEnvironmentVsgVisualizer` (only bounding-box
  approximations for non-standard `ShapeBase` subclasses),
  `MobilityVsgVisualizer` (movement-trail geometry rebuilds the whole
  pipeline as the trail grows rather than using a dynamic vertex buffer).

## History

- **PR #1112** (`fix/vsg-translucent-compositing-v2`, merged as
  `671b29a034`) — diagnosed and fixed the translucent-over-opaque
  compositing bug (the "black blob" issue): overrode the alpha-channel blend
  factors so the off-screen framebuffer's output alpha stays opaque for Qt's
  compositor, applied across `createGeometry`, `createWaveRing`, and (via
  `fixBuilderBlendAlpha`) every VSG-`Builder`-baked node.
- **PR #1113** (`fix/vsg-signal-ripple`, merged as `dbe6bab83f`) — added the
  GLSL shader-based signal wavefront (`signalWaveShader`,
  `createWaveRingShader`) with a continuously flowing, per-pixel ripple
  driven by animation time, plus the fade-cap/animation-speed tuning
  (`signalFadingDistance`-bounded disc size decoupled from transmit power)
  proven out by the `vsgsignal`/`vsgsignalwave` example showcases.
- **PR #1114** (`fix/vsg-signal-sphere-shader`, merged as `07bcfdb61c`) —
  extended the shader wavefront to true 3D: `createSphereWaveShader` (a
  volumetric, concentric-shell rippling sphere) as the `signalShape = "sphere"`
  counterpart to the ring shader, plus `createWireframeSphere` and the
  `rangeSphere` parameter for physically-meaningful 3D range indicators, and
  the `vsgsignal3d`/`vsgsignalwave3d` drone-swarm showcases that exercise them.
  Also added this README and addressed the #1113 review feedback.
- **Follow-up** — `sceneModel`: load an external PLY point cloud (e.g. a LIDAR
  scan) as the scene ground (`createTerrainFromPLY`), recentered, auto-fit to
  the scene bounds, and elevation-colored; the `vsglidar` drone-swarm-over-terrain
  showcase demonstrates it.
