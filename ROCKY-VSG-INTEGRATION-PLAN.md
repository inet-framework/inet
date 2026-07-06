# Rocky (VSG geospatial) integration into INET — implementation plan

**Branch:** `vsg-rocky-geospatial` (based on `vsg-lidar-pathloss`).
**Goal:** run INET wireless simulations on **real geospatial data** — global imagery + terrain elevation from GeoTIFF/TMS/etc. — rendered in the existing VSG Qtenv backend, with the terrain elevation driving RF path loss/occlusion (as the LIDAR `PointCloudGround` already does, but now on a georeferenced round-earth map).

This plan is written to be executed by a capable-but-not-expert model. **Follow the phases in order. Each phase has an explicit GO/NO-GO gate. Do not start a phase until the previous gate passes.** When a step says "copy the pattern from `X`", open `X` and mirror it — do not invent a new structure.

---

## 0. Orientation — read these first (they are the templates)

INET already does this exact thing for **OSG** via osgEarth. Rocky is the **VSG** successor to osgEarth (same author, Glenn Waldron / Pelican Mapping). The Rocky work mirrors the OSG geospatial trio, one file at a time:

| Existing OSG file (the template) | New Rocky file (what you write) |
|---|---|
| `src/inet/visualizer/osg/scene/SceneOsgEarthVisualizer.{h,cc}` — owns the osgEarth `MapNode`, hangs it in the scene | `src/inet/visualizer/vsg/scene/SceneRockyVisualizer.{h,cc}` — owns a Rocky `MapNode`, hangs it in the VSG scene |
| `src/inet/environment/ground/OsgEarthGround.{h,cc,ned}` — `IGround` that samples osgEarth elevation | `src/inet/environment/ground/RockyGround.{h,cc,ned}` — `IGround` that samples Rocky elevation |
| `src/inet/common/geometry/common/GeographicCoordinateSystem.{h,cc}` — `IGeographicCoordinateSystem` (scene ENU ↔ lat/lon/alt) | **REUSE AS-IS** — it is already backend-neutral |

**Also read for context (do not modify):**
- `src/inet/visualizer/vsg/base/SceneVsgVisualizerBase.{h,cc}` — our VSG scene base; `initializeSceneFloor()` (INITSTAGE_LAST) is where terrain gets added. `SceneRockyVisualizer` extends this.
- `src/qtenv/vsg/vsgoffscreen.h` (in `omnetpp-dev`) — **our VSG backend renders OFF-SCREEN** (to a `VkImage`, read back to a `QImage`). There is **no on-screen `vsg::Window`/swapchain**. This is the single most important constraint; see Risk R-2.
- `src/inet/environment/ground/PointCloudGround.{h,cc}` — the LIDAR ground; `RockyGround` plays the same role (`IGround` returning `computeGroundProjection`/`Normal`), just sourcing elevation from Rocky instead of a heightfield.
- `.oppfeatures` (search `VisualizationOsg`) and `src/makefrag` (lines ~58–75, the `WITH_OSGEARTH` block) — how an optional external 3D dependency is feature-gated. Rocky gets the same treatment under a new `WITH_ROCKY`.

**Rocky facts you will rely on (verified against pelicanmapping/rocky `main`, v1.1.0, MIT license — NOT the stale MAPSWorks fork):**
- Rocky can attach to an **externally-owned `vsg::Viewer`** and be added as an ordinary `vsg::Group` node — you do NOT let Rocky own the window/frame loop. The enabling API is `rocky::VSGContextFactory::create(viewer)` → `rocky::MapNode::create(vsgcontext.get())`. `MapNode` is `vsg::Inherit<vsg::Group, MapNode>`.
- Elevation can be **queried programmatically** (not just rendered) via `rocky::ElevationSampler` against the source data. This is what feeds RF physics.
- Coordinates: `rocky::GeoPoint(SRS, lon, lat, alt)` and `GeoPoint::transform(SRS::ECEF)` convert geodetic → geocentric world meters. `rocky::SRS::WGS84`, `SRS::ECEF`. `rocky::Ellipsoid` has `geodeticToGeocentric`/`geocentricToGeodetic` and `topocentricToGeocentricMatrix` for local ENU frames.
- Dependencies (vcpkg): `vsg`, `vsgxchange`, `gdal`, `proj`, `entt`, `nlohmann-json`, `glm`, `spdlog`, `sqlite3`, `openssl`, `cpp-httplib`, `zlib`. CMake target `rocky::rocky`. Requires Vulkan 1.3.268+.
- You must call `vsgcontext->update()` **once per frame** (an integration point in the render loop).

---

## 1. Target architecture (what "done" looks like)

Three coordinate frames, and the conversions between them:

1. **Simulation/scene ENU** — what INET nodes use (`Coord`, meters, local east-north-up). Drones, gcs, `getCurrentPosition()`.
2. **Geodetic** — `GeoCoord`/lat-lon-alt. The bridge is the existing `IGeographicCoordinateSystem` module: `computeGeographicCoordinate(Coord) → GeoCoord` and `computeSceneCoordinate(GeoCoord) → Coord`, anchored at a scene origin lat/lon (`getScenePosition()`).
3. **Rocky world (ECEF)** — geocentric meters, what Rocky renders in. Reached from geodetic via `GeoPoint::transform(SRS::ECEF)`.

**Data/physics path (`RockyGround::computeGroundProjection`, mirroring `OsgEarthGround`):**
```
Coord (scene ENU)
  → coordinateSystem->computeGeographicCoordinate()  → GeoCoord(lat,lon,alt)
  → rocky::ElevationSampler.sample(GeoPoint(WGS84,lon,lat,0))  → ground height (m)
  → GeoCoord with altitude = height
  → coordinateSystem->computeSceneCoordinate()  → Coord (scene ENU, on the ground)
```
`computeGroundNormal` = the 3-sample cross-product trick, copied verbatim from `OsgEarthGround::computeGroundNormal`.

**Render path (`SceneRockyVisualizer`, mirroring `SceneOsgEarthVisualizer` + our `SceneVsgVisualizerBase`):**
- At init, wrap the Qtenv VSG viewer via `VSGContextFactory::create(viewer)`, build a `rocky::MapNode`, add configured layers (image + elevation) to `mapNode->map`, and add `mapNode` into the VSG scene under a transform that places the scene-origin geodetic point at the VSG world origin our camera/nodes use.
- Expose `getMapNode()` / `getElevationLayer()` so `RockyGround` can reach the elevation source.
- Pump `vsgcontext->update()` each render.

**The elephant in the room:** our VSG backend is **off-screen** (`vsgoffscreen.h`), not a windowed swapchain like every Rocky example. Whether Rocky's terrain (tile paging, compile scheduling) works under our off-screen render-to-texture path is **unverified**. This is de-risked in Phase R1, and there is a fallback (Section 8).

---

## 2. Risks and the spikes that retire them (DO THESE BEFORE WRITING INET CODE)

| ID | Risk | Retired by | Fallback if it fails |
|---|---|---|---|
| R-1 | Rocky may not build on macOS/MoltenVK (Linux/Windows CI only, no macOS testing upstream) | **Spike S1** | Do the whole effort on Linux; or ship it feature-gated OFF on macOS |
| R-2 | Rocky terrain may assume an on-screen `vsg::Window`/swapchain and not render under our off-screen VkImage path | **Spike S2** | Section 8: render Rocky in its own on-screen window (degraded), or fall back to a static geo-referenced terrain mesh built from a queried elevation grid (reuses the LIDAR `createTerrainMeshFromHeightfield` path — no live Rocky rendering, physics still georeferenced) |
| R-3 | Rocky's pinned Vulkan/VSG version may not match omnetpp-dev's VSG | **Spike S1** (build against our VSG) | Pin/patch Rocky to our VSG version, or bump our VSG |
| R-4 | Render-on-demand (Qtenv refresh) vs Rocky's async tile paging — tiles may never finish loading without continuous frames | **Spike S2** | Set `renderContinuously`, or force-load a fixed LOD, or use the static-mesh fallback (R-2) |

**Spike S1 — Rocky builds and elevation query works, standalone (no INET).**
1. Clone `pelicanmapping/rocky`, build it with vcpkg against **the same VSG that omnetpp-dev uses** (find omnetpp-dev's VSG install; point Rocky's `vsg` dependency at it or verify versions match). On macOS this is the moment of truth for R-1/R-3.
2. Build and run `src/apps/rocky_simple` (or the `no_app()` path). GO if a map renders in its own window on this platform.
3. Write a 30-line standalone program: create a `MapNode`, add a **local GeoTIFF** `GDALElevationLayer` (offline — no network), and use `rocky::ElevationSampler` to print the elevation at a few lat/lons. GO if the heights are sane. **This proves the physics data path independent of all rendering.**
- **GATE G1:** Rocky builds on the target platform against our VSG, and `ElevationSampler` returns correct heights offline. If macOS fails, decide with the user: Linux-only, or proceed to physics-only (R-2 static fallback needs no Rocky rendering, only `ElevationSampler`).

**Spike S2 — a Rocky `MapNode` renders into an OFF-SCREEN VSG target.**
1. Take the off-screen render harness from `omnetpp-dev/src/qtenv/vsg/vsgoffscreen.h` (or the spike under `impl/spike-offscreen/` referenced in its header) — a viewer that renders a scene to a `VkImage` and reads it back, no window.
2. Put a `rocky::MapNode` (wrapping that viewer via `VSGContextFactory`) as the scene, pump `vsgcontext->update()` each frame, render several frames, read back the image.
3. GO if the terrain appears in the read-back image (may need several frames / a forced LOD for tiles to load).
- **GATE G2:** Rocky renders under off-screen VSG. If NO after reasonable effort, **switch to the static-mesh fallback (Section 8)** — the plan's physics phases (R2) are unchanged, only the render phase (R1) changes.

> Keep S1/S2 as throwaway code under a scratch dir, not in the INET tree. Their only output is the two GO/NO-GO decisions and the exact working CMake/vcpkg invocation, which Phase R0 then formalizes.

---

## 3. Phased implementation

Every INET module below is **opt-in** (selected by `typename`/feature) and compiled only when `WITH_ROCKY` is defined. No default behavior changes; existing fingerprints stay untouched (same discipline as the LIDAR work).

### Phase R0 — build integration (after G1)
Make INET able to compile+link against Rocky, gated, before writing any module.
1. **omnetpp-dev side:** add `WITH_ROCKY = no` to `Makefile.inc` and a `-DWITH_ROCKY` define path, mirroring `WITH_OSGEARTH` (see `Makefile.inc` lines around the existing `WITH_OSGEARTH`, and how it flows to `$(DEFINES_FILE)`). This is a separate small omnetpp-dev change (its own branch/PR).
2. **INET `src/makefrag`:** in the `VisualizationVsg` block (the one that already adds `VSG_CFLAGS`/`VSG_LIBS` and `-I$(OMNETPP_ROOT)/src`), add, guarded by `ifeq ($(WITH_ROCKY),yes)`: the Rocky include path and `-lrocky` (or the flags from `find_package(rocky)`; simplest is a `ROCKY_CFLAGS`/`ROCKY_LIBS` pair set in `Makefile.inc`). Mirror the `WITH_OSGEARTH` block verbatim in structure.
3. **`.oppfeatures`:** add a `VisualizationVsgRocky` feature (or extend `VisualizationVsg`) that, when enabled, defines `INET_WITH_ROCKY`. Copy the shape of the osgEarth-related feature entry.
4. Guard every new `.cc`/`.h` body with `#if defined(WITH_ROCKY) && defined(INET_WITH_VISUALIZATIONVSG)` — exactly like `OsgEarthGround.cc`'s `#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)`.
- **GATE G3:** a no-op file including a Rocky header compiles and links inside INET when `WITH_ROCKY=yes`, and INET still builds normally when `WITH_ROCKY=no`. Test BOTH.

### Phase R1 — render a map: `SceneRockyVisualizer` (after G2)
Get a Rocky map showing in the Qtenv VSG window.
1. Create `src/inet/visualizer/vsg/scene/SceneRockyVisualizer.{h,cc,ned}` extending `SceneVsgVisualizerBase` (mirror `SceneOsgEarthVisualizer` which extends `SceneOsgVisualizerBase`).
2. NED parameters (copy names/spirit from `SceneOsgEarthVisualizer.ned` and Rocky's layer model): `string coordinateSystemModule`, and a way to declare layers — start minimal with `string mapImageFile` (local GeoTIFF for a `GDALImageLayer`) and `string mapElevationFile` (local GeoTIFF for a `GDALElevationLayer`). Offline/local only for the first cut (no TMS/network).
3. In `initializeScene()`/`initializeSceneFloor()` (INITSTAGE_LAST, where `SceneVsgVisualizerBase` adds the floor): get the Qtenv VSG viewer (the scene visualizer already reaches the VSG scene via `TopLevelScene::getSimulationScene(...)`; obtaining the `vsg::Viewer` may require a small accessor added on the omnetpp-dev VSG backend — see note below), build `VSGContextFactory::create(viewer)`, `MapNode::create(vsgcontext)`, add the GDAL image+elevation layers to `mapNode->map`, and add `mapNode` to the scene under a transform (see Section 5 for the transform). Store `mapNode`, `vsgcontext`, and the elevation layer as members with `getMapNode()`/`getElevationLayer()` accessors.
4. Pump `vsgcontext->update()` each frame — add the call wherever `SceneVsgVisualizerBase`/the backend refreshes (this may need a hook; the offscreen `render()` in `vsgoffscreen.h` is the natural place, i.e. a small omnetpp-dev change, OR call it from the visualizer's `refreshDisplay()`).
- **GATE G4 (visual, user-verified):** run a demo with `SceneRockyVisualizer` + a local GeoTIFF and confirm the terrain renders in Qtenv/VSG. (User does the visual check, as with all VSG work.)

> **omnetpp-dev accessor note:** `SceneRockyVisualizer` needs the actual `vsg::ref_ptr<vsg::Viewer>` that the offscreen backend owns, plus a per-frame `update()` hook. Our `vsgoffscreen.h`/`vsgviewer` may not expose the viewer today. Budget a small omnetpp-dev change (a `getViewer()` accessor + an `onFrame` hook), on its own branch/PR, analogous to how the OSG path exposes its viewer.

### Phase R2 — physics: `RockyGround : IGround` (after G1; independent of R1)
This is the RF payoff and does **not** depend on rendering — it only needs `ElevationSampler` + the elevation layer, so it can proceed even if R1/R2-render is on the fallback path.
1. Create `src/inet/environment/ground/RockyGround.{h,cc,ned}` implementing `IGround` — **copy `OsgEarthGround.{h,cc,ned}` and swap the elevation call.**
2. `initialize()`: `coordinateSystem.reference(this, "coordinateSystemModule", true)`; get the elevation source. Two options for the source, prefer (a):
   - (a) **Own the elevation layer** (a `GDALElevationLayer` created from a `file` NED param) + a standalone `rocky::ElevationSampler` — no dependency on the visualizer at all. Cleanest; works headless (Cmdenv). Recommended.
   - (b) Get the layer from `SceneRockyVisualizer::getElevationLayer()` (mirrors `OsgEarthGround` getting the map from the visualizer) — couples physics to a GUI module; avoid unless (a) is impractical.
3. `computeGroundProjection(Coord p)`:
   ```cpp
   GeoCoord geo = coordinateSystem->computeGeographicCoordinate(p);
   auto s = sampler.sample(rocky::GeoPoint(rocky::SRS::WGS84, geo.longitude.get<deg>(), geo.latitude.get<deg>(), 0), io);
   if (s.ok()) geo.altitude = m(s.value().height); else /* outOfBounds policy like PointCloudGround */;
   return coordinateSystem->computeSceneCoordinate(geo);
   ```
4. `computeGroundNormal` — copy `OsgEarthGround::computeGroundNormal` verbatim (3-sample cross product).
5. **Performance:** RF sims call `computeGroundProjection` a lot (per tx×rx, and `TerrainObstacleLoss` samples along each ray). `ElevationSampler` hits source data (slow-ish). Use `ElevationSampler::session(io)` / `clampRange` for batched ray sampling, and/or cache into a local `Heightfield` at init over the scene bounds (this REUSES the existing `Heightfield` class — build it once from a grid of `ElevationSampler` queries, then all downstream RF code, including `TerrainObstacleLoss`, works unchanged because it already consumes a `Heightfield`). **Strongly recommended: `RockyGround` builds a `Heightfield` at init from sampled Rocky elevation, then delegates exactly like `PointCloudGround`.** This makes `TerrainObstacleLoss` (los/fresnel/diffraction) work over Rocky terrain for free.
- **GATE G5 (headless):** a Cmdenv config with `RockyGround` over a local GeoTIFF runs; node altitudes above the real terrain vary correctly; `TwoRayGroundReflection` and `TerrainObstacleLoss` behave as over `PointCloudGround`. Verify with a scalar/log check, no GUI needed.

### Phase R3 — place nodes at geospatial coordinates (optional polish)
So a scenario can be authored in lat/lon and the drones/gcs land on the real map.
1. This is just the `IGeographicCoordinateSystem` you already use: give the mobility/scenario lat/lon, convert to scene `Coord` via `computeSceneCoordinate`. No Rocky code needed for the physics side.
2. For **rendering** a node label/icon anchored geospatially (if desired), use `rocky::GeoTransform` (a `vsg::Group` accepting a `GeoPoint`) or the ECS `Transform` component — but the existing VSG node visualizers already place nodes at scene `Coord`, which R2's coordinate system maps onto the map, so this phase is optional eye-candy.

### Phase R4 — example + docs
1. Add `examples/visualizer/vsgrocky/` (mirror `examples/visualizer/vsglidar/`): a `.ned` network, an `omnetpp.ini` with a `[Config]` selecting `SceneRockyVisualizer` + `RockyGround` + a small **local** GeoTIFF checked into the example (or a script to fetch one), and a `SimpleGeographicCoordinateSystem`/`OsgGeographicCoordinateSystem` anchored at the tile's lat/lon.
2. A `README.md` documenting the WITH_ROCKY build, the vcpkg dependency install, and the macOS caveat.
3. Unit-testable piece: the coordinate round-trip (`computeSceneCoordinate(computeGeographicCoordinate(p)) ≈ p`) and, if `RockyGround` builds a `Heightfield`, a small `ElevationSampler`→`Heightfield` sanity test.

---

## 4. Build integration details (concrete)

- **Dependency acquisition:** document a vcpkg-based Rocky install (`vcpkg install rocky` if a port exists, else build-from-source per Rocky's README with its `vcpkg-ports/` overlay). Record the exact `find_package(rocky CONFIG REQUIRED)` → `rocky::rocky` flags and translate them into `ROCKY_CFLAGS`/`ROCKY_LIBS` variables in omnetpp-dev's `Makefile.inc` (INET builds via opp_makemake/makefrag, not CMake, so you extract the include/lib flags once and hardcode them into `Makefile.inc`, exactly as `OSG_LIBS`/`OSGEARTH_LIBS` are handled).
- **Feature gating (must):** `WITH_ROCKY` (omnetpp-dev build flag) **and** an INET `.oppfeatures` feature defining `INET_WITH_ROCKY`. New code compiles only when both are set. **Test the OFF path** — INET must build normally without Rocky (mirror the "test feature combinations separately" lesson already recorded from the OSG-only build regression).

## 5. Coordinate system design (the subtle part — get it right)

- The scene visualizer anchors the map. Pick the scene-origin geodetic point from `IGeographicCoordinateSystem::getScenePosition()` (lat/lon/alt), exactly as `SceneOsgEarthVisualizer` does (`geoTransform->setPosition(GeoPoint(...scenePosition...))`).
- INET nodes live in a **local ENU tangent plane** at that origin (small area, flat-earth-ish). Rocky renders in **ECEF** (round earth). The transform that hangs `mapNode` in the scene must map the ENU origin to the ECEF point and orient ENU axes to the local tangent frame. Use `rocky::Ellipsoid::topocentricToGeocentricMatrix(originECEF)` to get that local-frame matrix, invert/compose as needed so the map sits under the nodes.
- **Simplest first cut:** if the scene is small (km-scale, like the LIDAR tile), a `SimpleGeographicCoordinateSystem` (pure ENU tangent plane) is enough for physics; the render transform just needs the map positioned and oriented at the origin. Only reach for full geocentric fidelity if simulating over large/curved extents.
- **Sanity check to implement:** `computeSceneCoordinate(computeGeographicCoordinate(Coord::ZERO))` ≈ `Coord::ZERO`, and a point 1 km east in ENU maps to a geodetic point ~1 km east. Put this in the example's checks.

## 6. File-by-file work list (checklist)

omnetpp-dev (small, separate PR):
- [ ] `Makefile.inc`: `WITH_ROCKY`, `ROCKY_CFLAGS`, `ROCKY_LIBS`, define flow.
- [ ] `src/qtenv/vsg/vsgviewer.*` / `vsgoffscreen.h`: `getViewer()` accessor + per-frame `update()` hook for the map context.

INET (`vsg-rocky-geospatial` branch):
- [ ] `src/makefrag`: `WITH_ROCKY` block in the VisualizationVsg section.
- [ ] `.oppfeatures`: Rocky feature → `INET_WITH_ROCKY`.
- [ ] `src/inet/visualizer/vsg/scene/SceneRockyVisualizer.{h,cc,ned}` (mirror `SceneOsgEarthVisualizer`).
- [ ] `src/inet/environment/ground/RockyGround.{h,cc,ned}` (mirror `OsgEarthGround`; internally build a `Heightfield` for speed + `TerrainObstacleLoss` reuse).
- [ ] `examples/visualizer/vsgrocky/` (mirror `vsglidar`): `.ned`, `omnetpp.ini`, a small local GeoTIFF, `README.md`.
- [ ] Guard all bodies with `#if defined(WITH_ROCKY) && defined(INET_WITH_VISUALIZATIONVSG)`.

## 7. Verification

- Build both ways: `WITH_ROCKY=yes` and `=no` (both must succeed) — separately, per the feature-combination lesson.
- G1–G5 gates above.
- Headless Cmdenv run of the `vsgrocky` demo (physics only) — no crash, terrain-varying SNR.
- Visual (user): map renders; nodes sit on the terrain; if `TerrainObstacleLoss` is enabled, links occlude behind real ridges.
- Reuse the LIDAR verification discipline: no default NED/ini changes; existing fingerprints untouched.

## 8. Fallback if Rocky won't render off-screen (Gate G2 = NO)

If Spike S2 shows Rocky terrain won't render under our off-screen VSG path (plausible — untested upstream), **do not block the whole feature**:
- **Keep Phase R2 entirely** (physics via `ElevationSampler` → `Heightfield` → `IGround`), which needs NO Rocky rendering. You still get real georeferenced terrain driving RF.
- For the **visual**, build a **static terrain mesh** from the sampled `Heightfield` using the existing `createTerrainMeshFromHeightfield` (already implemented on `vsg-lidar-pathloss`) — the same shaded surface, now sourced from Rocky's elevation instead of a PLY. No live Rocky scene-graph in the VSG window, but the map's geometry is faithfully shown and it renders through our proven off-screen path.
- This fallback delivers ~80% of the value (real geospatial terrain + physics + a rendered surface) with far less risk, and can ship first with live-Rocky rendering as a later enhancement.

---

## 9. Sequencing summary (do this, in order)

1. **S1** (Rocky builds on platform + `ElevationSampler` works) → **G1**. If macOS fails, decide scope with the user.
2. **S2** (Rocky renders off-screen) → **G2**. If NO, adopt Section 8 fallback for rendering; physics phases unchanged.
3. **R0** build gating → **G3**.
4. **R2** `RockyGround` (physics; build a `Heightfield`) → **G5**. *This is the highest-value, lowest-risk deliverable — do it early.*
5. **R1** `SceneRockyVisualizer` (live map) OR Section 8 static mesh → **G4**.
6. **R3** geospatial placement (optional), **R4** example + docs.

**Ship order:** physics-first (`RockyGround` + a rendered static mesh) is a complete, low-risk first release. Live Rocky map rendering is a second release gated on S2.
