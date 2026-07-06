//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/scene/SceneRockyVisualizer.h"

#if defined(WITH_ROCKY)

#include <algorithm> // std::remove
#include <cstdlib> // setenv
#include <string>

#include <glm/glm.hpp>
#include <rocky/GDALElevationLayer.h>
#include <rocky/GDALImageLayer.h>
#include <rocky/GeoPoint.h>
#include <rocky/SRS.h>
#include <rocky/TMSElevationLayer.h>
#include <rocky/TMSImageLayer.h>

#include "inet/common/InitStages.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "qtenv/vsg/vsgscenehandle.h"   // omnetpp::VsgScene3DNode (the render hook)

namespace inet {

namespace visualizer {

Define_Module(SceneRockyVisualizer);

SceneRockyVisualizer::~SceneRockyVisualizer()
{
    // The VsgScene3DNode (owned by the parent module, which outlives this submodule) still holds
    // callbacks that capture `this`; flip the shared flag so they stop touching freed memory.
    if (alive)
        *alive = false;
}

void SceneRockyVisualizer::initialize(int stage)
{
    SceneVsgVisualizer::initialize(stage);
    if (!hasGUI())
        return;
    if (stage == INITSTAGE_LOCAL) {
        alive = std::make_shared<bool>(true);
        coordinateSystem.reference(this, "coordinateSystemModule", true);
        // Rocky loads its terrain shaders/data at runtime via vsg::findFile; point it at the
        // Rocky install's share dir (ROCKY_FILE_PATH), or it asserts "may not find its shaders".
        const char *resourcePath = par("rockyResourcePath");
        if (resourcePath != nullptr && *resourcePath != '\0')
            setenv("ROCKY_FILE_PATH", resourcePath, 1);
        // A map fills the ground but not the sky; give the empty area above the horizon a sky
        // colour instead of the default backdrop so the map doesn't look cut off at the horizon.
        visualizationTargetModule->getOsgCanvas()->setClearColor(cFigure::Color(135, 206, 235));
        // A map is best seen from above: start the camera looking straight down. Wheel zoom only
        // changes distance (not tilt), so the view stays top-down — you always look at the global
        // map instead of across it at the horizon, which avoids the map appearing to clip away.
        visualizationTargetModule->getOsgCanvas()->setCameraManipulatorType(cOsgCanvas::CAM_OVERVIEW);
    }
    else if (stage == INITSTAGE_LAST) {
        // Attach to the backend's render hook: build the map when the viewer becomes available
        // (and rebuild if it is recreated, e.g. on resize), and pump Rocky's per-frame update().
        auto sceneNode = visualizationTargetModule->getOsgCanvas()->getScene();
        if (sceneNode == nullptr)
            throw cRuntimeError("SceneRockyVisualizer: no 3D scene on the canvas (is the VSG 3D backend active?)");
        if (sceneNode->getBackendType() == omnetpp::cScene3DNode::BACKEND_VSG) {
            auto vsgSceneNode = static_cast<omnetpp::VsgScene3DNode *>(sceneNode);
            // capture the shared 'alive' flag so the callbacks no-op after this module is destroyed
            auto flag = alive;
            vsgSceneNode->onViewerChanged = [this, flag](vsg::ref_ptr<vsg::Viewer> viewer) { if (*flag) setupRockyMap(viewer); };
            vsgSceneNode->perFrameCallbacks.push_back([this, flag]() { if (*flag && context != nullptr) context->update(); });
        }
        else
            throw cRuntimeError("SceneRockyVisualizer requires the VSG 3D backend");
    }
}

void SceneRockyVisualizer::setupRockyMap(const vsg::ref_ptr<vsg::Viewer>& viewer)
{
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);

    // Drop any previously-built map (the viewer was recreated, e.g. on resize). Release the old
    // MapNode BEFORE recreating the context: MapNode/terrain hold GPU resources built against the
    // old context's device, so the context must outlive them (the reverse of member-destruction
    // order would destroy the device while the old map is still alive -> Vulkan validation error).
    if (mapTransform != nullptr) {
        auto& children = scene->children;
        children.erase(std::remove(children.begin(), children.end(), vsg::ref_ptr<vsg::Node>(mapTransform)), children.end());
        mapTransform = nullptr;
        mapNode = nullptr;
    }

    // Bind a Rocky context to the live viewer and build the map + layers.
    contextOwner = rocky::VSGContextFactory::create(viewer);
    context = contextOwner.get();
    mapNode = rocky::MapNode::create(context);

    // Elevation: a local GDAL source (DEM GeoTIFF/...) and/or an online TMS elevation service.
    const char *elevationFile = par("elevationFile");
    if (elevationFile != nullptr && *elevationFile != '\0') {
        auto layer = rocky::GDALElevationLayer::create();
        layer->uri = rocky::URI(std::string(elevationFile));
        mapNode->map->add(layer);
    }
    const char *elevationUrl = par("elevationUrl");
    if (elevationUrl != nullptr && *elevationUrl != '\0') {
        auto layer = rocky::TMSElevationLayer::create();
        layer->uri = rocky::URI(std::string(elevationUrl));
        mapNode->map->add(layer);
    }
    // Imagery: a local GDAL source and/or an online TMS imagery service (drapes over the terrain).
    const char *imageFile = par("imageFile");
    if (imageFile != nullptr && *imageFile != '\0') {
        auto layer = rocky::GDALImageLayer::create();
        layer->uri = rocky::URI(std::string(imageFile));
        mapNode->map->add(layer);
    }
    const char *imageUrl = par("imageUrl");
    if (imageUrl != nullptr && *imageUrl != '\0') {
        auto layer = rocky::TMSImageLayer::create();
        layer->uri = rocky::URI(std::string(imageUrl));
        mapNode->map->add(layer);
    }

    // Rocky renders a geocentric (ECEF) globe; the scene is a local ENU tangent plane at the
    // coordinate system's geographic origin. Place the map so that origin sits at the scene
    // origin with ENU orientation: transform by the inverse of the ENU->ECEF tangent matrix.
    //
    // NOTE: this is an ellipsoidal (WGS84) tangent-plane placement. If nodes/physics use
    // SimpleGeographicCoordinateSystem (a flat equirectangular approximation), the two projections
    // agree exactly only at the origin and drift apart with distance (metres, growing to more at
    // tens of km). For exact agreement over large areas a coordinate system derived from the map's
    // SRS would be needed (cf. the OSG-era OsgGeographicCoordinateSystem).
    auto origin = coordinateSystem->getScenePosition();
    rocky::GeoPoint originGeo(rocky::SRS::WGS84, origin.longitude.get<deg>(), origin.latitude.get<deg>(), origin.altitude.get<m>());
    rocky::GeoPoint originEcef = originGeo.transform(rocky::SRS::ECEF);
    glm::dmat4 enuToEcef = mapNode->srs().ellipsoid().topocentricToGeocentricMatrix(glm::dvec3(originEcef.x, originEcef.y, originEcef.z));
    glm::dmat4 ecefToEnu = glm::inverse(enuToEcef);

    vsg::dmat4 matrix; // both glm and vsg are column-major: value[column][row]
    for (int column = 0; column < 4; column++)
        for (int row = 0; row < 4; row++)
            matrix[column][row] = ecefToEnu[column][row];

    mapTransform = vsg::MatrixTransform::create(matrix);
    mapTransform->addChild(mapNode);
    // Insert the (opaque) map at the FRONT of the scene so it is drawn before the transparent
    // node icons/labels. If it is drawn after them, the icons (already in the scene) write depth
    // that occludes the map within their billboard quad, so their transparent pixels show the
    // background clear colour instead of the map behind them.
    scene->children.insert(scene->children.begin(), vsg::ref_ptr<vsg::Node>(mapTransform));

    EV_INFO << "SceneRockyVisualizer: Rocky map built and placed at the scene origin ("
            << origin.latitude.get<deg>() << ", " << origin.longitude.get<deg>() << ")\n";
}

} // namespace visualizer

} // namespace inet

#endif // defined(WITH_ROCKY)
