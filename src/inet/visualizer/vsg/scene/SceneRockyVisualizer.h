//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEROCKYVISUALIZER_H
#define __INET_SCENEROCKYVISUALIZER_H

#include "inet/visualizer/vsg/scene/SceneVsgVisualizer.h"

#if defined(WITH_ROCKY)

#include <memory>

#include <rocky/Context.h>
#include <rocky/vsg/MapNode.h>
#include <rocky/vsg/VSGContext.h>

#include <vsg/nodes/MatrixTransform.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

namespace inet {

namespace visualizer {

/**
 * Renders real geospatial imagery + terrain in the VSG 3D view using Rocky (the
 * VSG successor to osgEarth). Unlike the OSG plugin, which pages terrain from the
 * OSG viewer's own frame loop, Rocky needs the live vsg::Viewer (to build a
 * VSGContext) and an explicit per-frame update() to page/compile tiles; both are
 * obtained through the VsgScene3DNode render hook the backend publishes. Rocky's
 * geocentric (ECEF) map is transformed into the scene's local ENU frame, anchored
 * at the geographic origin of an IGeographicCoordinateSystem, so it sits under the
 * network nodes. See SceneRockyVisualizer.ned.
 */
class INET_API SceneRockyVisualizer : public SceneVsgVisualizer
{
#if defined(WITH_ROCKY)
  protected:
    ModuleRefByPar<IGeographicCoordinateSystem> coordinateSystem;
    rocky::VSGContextSingleton contextOwner;    // keeps the ContextImpl alive (Context is a raw pointer into it)
    rocky::VSGContext context = nullptr;
    vsg::ref_ptr<rocky::MapNode> mapNode;
    vsg::ref_ptr<vsg::MatrixTransform> mapTransform; // ECEF->ENU wrapper currently in the scene
    // The callbacks registered on the (parent-owned, longer-lived) VsgScene3DNode capture `this`;
    // this shared flag lets them no-op after the visualizer is destroyed instead of using freed
    // memory (submodules are destroyed before their parent, so the scene node outlives us).
    std::shared_ptr<bool> alive;

    virtual ~SceneRockyVisualizer();
    virtual void initialize(int stage) override;
    // (Re)build the map bound to the given viewer and place it in the scene (called when the
    // backend (re)creates its viewer, e.g. on resize).
    void setupRockyMap(const vsg::ref_ptr<vsg::Viewer>& viewer);
#endif
};

} // namespace visualizer

} // namespace inet

#endif // defined(WITH_ROCKY)

#endif
