# Geospatial Simulations with Rocky + VSG in OMNeT++/INET

This guide shows how to build OMNeT++/INET simulations that use **real geospatial
data** — world maps, terrain elevation, and a rendered Earth globe — via
[Rocky](https://github.com/pelicanmapping/rocky), the VulkanSceneGraph (VSG)
successor to osgEarth. It covers the three reusable tools built on top of the
Qtenv VSG backend:

| Tool | Repo | What it gives you |
|------|------|-------------------|
| **`RockyGround`** | INET | Real terrain elevation driving the radio physics (an `IGround`) — works headless |
| **`SceneRockyVisualizer`** | INET | A real geospatial map rendered under your nodes in the 3D view |
| **`VsgScene3DNode` render hook + `vsg-satellites` sample** | omnetpp-dev | The building blocks for your own Rocky 3D visualizations (globe, orbits, custom overlays) |

If you just want *real terrain physics*, read §3. If you want a *map in the 3D
view*, read §4. If you want to build *custom geospatial 3D* (satellites,
coverage, custom overlays), read §5. §6 is the list of gotchas — read it before
you spend hours debugging.

---

## 1. Overview

Rocky renders a geocentric (ECEF) Earth in a VSG scene graph and samples GDAL
elevation/imagery sources. OMNeT++'s Qtenv VSG backend renders **off-screen**
(no window — a `VkImage` read back into a `QImage`), so Rocky is embedded there
rather than in an on-screen viewer. Two independent capabilities result:

- **Physics** (`RockyGround`) is pure C++ — it samples elevation with
  `rocky::ElevationSampler` and needs no viewer, so it runs in Cmdenv too.
- **Rendering** (`SceneRockyVisualizer`, custom visualizers) needs the live
  `vsg::Viewer` and a per-frame `update()`; those are obtained through a small
  hook on the shared scene handle (`VsgScene3DNode`).

Both use the backend-neutral `IGeographicCoordinateSystem` (INET) to map between
the local scene ENU frame (metres) and geographic coordinates (lat/lon/alt).

---

## 2. Prerequisites and building

### 2.1 Rocky SDK

Build and install Rocky (v1.x) against your VSG + Vulkan. On macOS three small
local source patches are currently required (a GDAL 3.13 `CSLConstList` fix,
`#ifdef ROCKY_HAS_IMGUI` guards so it builds without vsgImGui, and a
`-framework CoreFoundation -framework Security` link line); these should be
upstreamed. Install to a prefix, e.g. `~/rocky-install`, so you have
`include/rocky/…` and `lib/librocky.*`.

At **runtime** Rocky must find its shaders — set:

```
export ROCKY_FILE_PATH=/path/to/rocky-install/share/rocky
```

(Or pass the path through the module parameter that sets it for you — see §4/§5.)

### 2.2 OMNeT++ VSG backend

You need the Qtenv VSG backend (`oppqtenv-vsg`) built **with the viewer hook**
(the `VsgScene3DNode` `onViewerChanged` / `perFrameCallbacks` /
`cameraCentre`/`cameraRadius` additions and the bounded render pump). These live
on the omnetpp-dev branch that adds Rocky support.

### 2.3 INET with Rocky

INET's `src/makefrag` has an opt-in `WITH_ROCKY` block. Build with:

```
cd inet/src
make MODE=release WITH_ROCKY=yes ROCKY_ROOT=/path/to/rocky-install
```

`WITH_ROCKY` adds Rocky's include/lib and defines `-DWITH_ROCKY`. Leaving it
unset builds INET normally (the Rocky modules compile to empty stubs). After
toggling `WITH_ROCKY`, `touch` the affected `.cc` files or remove their `.o`s,
since the build caches compiler options.

---

## 3. Real terrain in the radio physics — `RockyGround`

`RockyGround` implements INET's `IGround` by sampling a GDAL-openable elevation
source (a GeoTIFF DEM, etc.). Any consumer of `physicalEnvironment.getGround()`
— e.g. `TwoRayGroundReflection` path loss — then sees the real surface with **no
changes to that consumer**. It is headless (Cmdenv-safe).

### 3.1 Minimal setup

```ini
# a geographic anchor for the scene's local ENU tangent plane
*.coordinateSystem.typename = "SimpleGeographicCoordinateSystem"
*.coordinateSystem.sceneLatitude = 37.8deg
*.coordinateSystem.sceneLongitude = -122.4deg
*.coordinateSystem.sceneAltitude = 0m

# the ground surface comes from a real DEM via Rocky
*.physicalEnvironment.ground.typename = "RockyGround"
*.physicalEnvironment.ground.elevationFile = "dem.tif"          # any GDAL elevation source
*.physicalEnvironment.ground.coordinateSystemModule = "^.^.coordinateSystem"
```

`computeGroundProjection(p)` converts the scene point to geographic, samples the
DEM, and converts back; `computeGroundNormal` uses a 3-sample cross product.
Points outside the DEM return `outOfBoundsElevation` (default 0 m).

See `examples/environment/rockyground/` for a runnable smoke test (it logs the
sampled elevation at a few scene points at `INITSTAGE_LAST`; run Cmdenv with
`--cmdenv-express-mode=false --cmdenv-log-level=info` to see the log).

### 3.2 Notes

- The `elevationFile` path is resolved relative to the run's working directory.
- The DEM's geographic extent should straddle the coordinate-system origin.
- No GUI is required — this is physics; use it in Cmdenv for batch studies.

---

## 4. A geospatial map in the 3D view — `SceneRockyVisualizer`

`SceneRockyVisualizer` (extends `SceneVsgVisualizer`) renders a Rocky map in the
Qtenv VSG 3D view, transformed into the scene's local ENU frame so it sits under
your nodes. Imagery/terrain can come from **local GDAL files** or **online TMS
tiles** (e.g. readymap.org).

### 4.1 Local imagery/DEM

```ini
*.visualizer.sceneVisualizer.typename = "SceneRockyVisualizer"
*.visualizer.sceneVisualizer.coordinateSystemModule = "^.^.coordinateSystem"
*.visualizer.sceneVisualizer.imageFile = "img.tif"        # a GDAL imagery source (drapes over the terrain)
*.visualizer.sceneVisualizer.elevationFile = "dem.tif"    # a GDAL elevation source (optional)
*.visualizer.sceneVisualizer.rockyResourcePath = "/path/to/rocky-install/share/rocky"
```

### 4.2 Real Earth from readymap.org (online tiles)

```ini
*.visualizer.sceneVisualizer.imageUrl = "https://readymap.org/readymap/tiles/1.0.0/7/"
*.visualizer.sceneVisualizer.elevationUrl = "https://readymap.org/readymap/tiles/1.0.0/116/"
```

Tiles page in over the network. The backend runs a **bounded render burst**
(~10 s) whenever the map is built or the view changes, so the map loads without
you having to nudge it, then stops so an idle view costs nothing.

### 4.3 Behaviour

- The visualizer sets a **top-down** camera (`CAM_OVERVIEW`) and a sky-blue
  clear colour; wheel zoom keeps the tilt, so you always look down at the map.
- It sets `ROCKY_FILE_PATH` from `rockyResourcePath` for you.
- Nodes are placed by whatever mobility you configure, in scene metres relative
  to the coordinate-system origin.

See `examples/visualizer/vsgrocky/` — configs `General` (synthetic offline tile),
`RealMap` (readymap San Francisco Bay), and `Communication` (nodes exchanging
UDP over the map with signal spheres + packet routes).

### 4.4 Communication demo recipe

To draw radio signals as animated spheres over the map, use the *realistic*
802.11 stack (the idealized unit-disk radio does **not** produce the waveform the
sphere visualizer animates) plus `signalWaveShader`, scaled up for the map:

```ini
*.host*.wlan[*].mgmt.typename = "Ieee80211MgmtAdhoc"
*.host*.wlan[*].agent.typename = ""
*.host*.wlan[*].radio.transmitter.power = 300mW      # boost for km-scale links
*.visualizer.mediumVisualizer.displaySignals = true
*.visualizer.mediumVisualizer.signalShape = "sphere"
*.visualizer.mediumVisualizer.signalWaveShader = true
*.visualizer.mediumVisualizer.signalFadingDistance = 300m   # sized for the km-scale map
*.visualizer.mediumVisualizer.signalWaveLength = 150m
```

Run in **animated** mode (the ▶ button, not Fast) to see the propagation
animate.

---

## 5. Building your own Rocky 3D visualization — the render hook

For anything beyond a ground map — a globe seen from space, satellites, custom
geospatial overlays — you build directly on the Qtenv VSG backend. The
`vsg-satellites` sample (omnetpp-dev) is the reference; the pattern is:

### 5.1 The `VsgScene3DNode` render hook

The backend renders a scene root (`vsg::Group`) wrapped in a `VsgScene3DNode`.
Model code both authors that scene and, for Rocky, needs the live viewer and a
per-frame update. `VsgScene3DNode` exposes:

```cpp
std::function<void(vsg::ref_ptr<vsg::Viewer>)> onViewerChanged; // fires when the backend (re)builds its viewer
std::vector<std::function<void()>>            perFrameCallbacks; // invoked once per rendered frame
vsg::dvec3 cameraCentre;  double cameraRadius; // optional camera framing hint (see below)
```

Typical scene-module setup:

```cpp
setenv("ROCKY_FILE_PATH", resourcePath, 1);          // shaders
auto root = vsg::Group::create();
auto sceneNode = omnetpp::createScene3DNode(root);
auto vsgSceneNode = static_cast<omnetpp::VsgScene3DNode*>(sceneNode);

vsgSceneNode->onViewerChanged = [this](vsg::ref_ptr<vsg::Viewer> viewer) {
    // the viewer only exists now — build the Rocky context + map here
    context = rocky::VSGContextFactory::create(viewer);   // keep the singleton owner alive
    mapNode = rocky::MapNode::create(context.get());
    // add layers, add mapNode to root, position your GeoTransforms, ...
};
vsgSceneNode->perFrameCallbacks.push_back([this]() { if (context) context->update(); }); // page/compile tiles

getParentModule()->getOsgCanvas()->setScene(sceneNode);
```

`onViewerChanged` fires on the first render and again whenever the backend
rebuilds its viewer (e.g. on resize), so make it idempotent / rebuild-safe.

### 5.2 Camera framing hint

The backend frames its orbit camera on the scene's bounding sphere. If your
content is added **after** `setScene` (Rocky's map is built in
`onViewerChanged`), that sphere is empty and the camera sits too close. Publish
an explicit framing:

```cpp
vsgSceneNode->cameraCentre = vsg::dvec3(0,0,0);     // e.g. Earth's centre
vsgSceneNode->cameraRadius = someRadiusMeters;       // e.g. the largest orbit
```

The backend applies it once when `cameraRadius > 0`; the user can still orbit/
zoom from there.

### 5.3 Placing things on the globe — `rocky::GeoTransform`

To put a node at a geographic location (a satellite, a ground marker):

```cpp
auto xform = rocky::GeoTransform::create();
xform->topocentric = true;                                    // (1) orient children in the local ENU frame (Z=up)
xform->position = rocky::GeoPoint(rocky::SRS::WGS84, lon, lat, alt); // (2) geographic, NOT raw ECEF
xform->radius = boundingRadiusMeters;                         // (3) cull bound big enough to enclose the children
xform->addChild(myGeometry);
mapNode->addChild(xform);                                     // (4) MUST be a child of the MapNode
xform->dirty();                                               // after changing position each frame
```

The four numbered points are the ones people get wrong — see §6.

### 5.4 Making time advance

Nodes that move over time (orbits) need simulation time to advance. In a demo
with no traffic, schedule a periodic self-message (a "clock") so `refreshDisplay`
is called with fresh `simTime()`; update your `GeoTransform` positions there.

---

## 6. Gotchas (read this)

1. **`ROCKY_FILE_PATH` must point at `share/rocky`** or Rocky asserts
   "may not find its shaders" at first render. `SceneRockyVisualizer` sets it
   from `rockyResourcePath`; custom code must `setenv` it before building the
   context.
2. **`GeoTransform` needs `topocentric = true`** to orient children into the
   local ENU tangent frame. The default (`false`) only *translates*, so oriented
   children (cones, models) keep a fixed world orientation.
3. **Position `GeoTransform` with a WGS84 `GeoPoint`, not raw ECEF.** A raw
   geocentric point positions correctly but does not give the ENU frame the
   right orientation. Convert:
   `GeoPoint(SRS::ECEF, ecef).transform(SRS::WGS84)`.
4. **Set `GeoTransform::radius` to enclose the child geometry.** It defaults to
   0, so the transform is culled on its anchor point alone — a large child (a
   coverage cone) vanishes as soon as the anchor scrolls off-screen even though
   the child is still in view.
5. **Add `GeoTransform`s as children of the `MapNode`.** `MapNode::traverse()`
   publishes the world-SRS/horizon values that `GeoTransform` reads; a sibling
   group won't have them (no ECS/registry is otherwise required).
6. **Do not use raw VSG line primitives in the off-screen backend.** A
   flat-shaded line-list (`createFlatShadedShaderSet` + `VertexDraw` +
   `LINE_LIST`) crashes during record in the off-screen path. Draw "lines" as
   thin `vsg::Builder` cylinders oriented with a `MatrixTransform` (see
   `orientSegment` in the sample).
7. **readymap TMS URLs** are the layer root, e.g.
   `https://readymap.org/readymap/tiles/1.0.0/7/` (imagery) and `.../116/`
   (elevation). They need internet at runtime.
8. **Cmdenv vs Qtenv.** Physics (`RockyGround`) runs headless. Visualizers skip
   their Rocky setup under `!hasGUI()`, so a Cmdenv run of a visualizer config
   just instantiates the network (useful for validating params).
9. **The map/tiles load progressively.** The render pump loads the initial view
   over ~10 s; zooming re-triggers it. This is normal paged-terrain behaviour.
10. **Projection mismatch at large scale.** `SceneRockyVisualizer` places the map
    with an ellipsoidal (WGS84) tangent plane, while `SimpleGeographicCoordinateSystem`
    (used for node placement / `RockyGround`) is a flat equirectangular
    approximation. They agree exactly only at the origin and drift apart with
    distance (metres near the origin, more at tens of km) — so nodes may not sit
    *exactly* on the matching map feature far from the origin. Keep scenes within
    a few km of the origin, or provide a coordinate system derived from the map's
    SRS if you need exact agreement over a large area.
11. **Known issue:** the off-screen VSG backend has a pre-existing teardown
    crash (a release-only Heisenbug) on some exits, unrelated to these tools.

---

## 7. Worked examples

| Example | Where | Shows |
|---------|-------|-------|
| `rockyground` | INET `examples/environment/` | `RockyGround` physics, headless-friendly |
| `vsgrocky` | INET `examples/visualizer/` | `SceneRockyVisualizer`: `General` (offline), `RealMap` (readymap), `Communication` (UDP + signals over the map) |
| `vsg-satellites` | omnetpp-dev `samples/` | Custom Rocky 3D: globe from space, orbiting satellites (`GeoTransform`), coverage cones, sat↔sat + sat↔ground links; configs `Geostationary`, `PolarOrbits`, `RandomOrbits` |

Run any of them with `ROCKY_FILE_PATH` set and (for the online configs) an
internet connection.
