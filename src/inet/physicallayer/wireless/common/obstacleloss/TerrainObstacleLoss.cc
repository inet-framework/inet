//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/obstacleloss/TerrainObstacleLoss.h"

#include <cmath>
#include <cstring>

#include "inet/environment/ground/PointCloudGround.h"

namespace inet {

namespace physicallayer {

Define_Module(TerrainObstacleLoss);

void TerrainObstacleLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physicalEnvironment.reference(this, "physicalEnvironmentModule", true);
        const char *modeString = par("mode");
        if (!strcmp(modeString, "los"))
            mode = LOS;
        else if (!strcmp(modeString, "fresnel"))
            mode = FRESNEL;
        else
            throw cRuntimeError("Unknown mode '%s'", modeString);
        sampleStep = par("sampleStep");
        logLinkEvents = par("logLinkEvents");
    }
}

double TerrainObstacleLoss::computeKnifeEdgeLoss(double v)
{
    // ITU-R P.526 approximate single knife-edge diffraction loss, valid for
    // v > -0.78; below that the obstruction is far enough outside the first
    // Fresnel zone to have no effect (the curve is ~0 dB at the cutoff).
    if (v <= -0.78)
        return 0;
    return 6.9 + 20 * std::log10(std::sqrt((v - 0.1) * (v - 0.1) + 1) + v - 0.1);
}

void TerrainObstacleLoss::emitObstaclePenetrated(const Coord& rayPoint, const Coord& groundPoint, double lossFactor) const
{
    // No physical object is associated with terrain: coordinates are world-frame
    // (see ObstaclePenetratedEvent). The intersection segment connects the direct
    // ray to the terrain surface at the critical point, making the obstruction
    // (or near-grazing pinch) visible in the tracing obstacle loss visualizers.
    ObstaclePenetratedEvent event(nullptr, rayPoint, groundPoint, getHeightfield()->getNormal(groundPoint.x, groundPoint.y), Coord(0, 0, 1), lossFactor);
    const_cast<TerrainObstacleLoss *>(this)->emit(obstaclePenetratedSignal, &event);
}

const cModule *TerrainObstacleLoss::findNodeAt(const Coord& position) const
{
    if (nodes.empty()) {
        // collect the network nodes that have a mobility submodule (built lazily, after the network is up)
        auto networkModule = getSimulation()->getSystemModule();
        for (cModule::SubmoduleIterator it(networkModule); !it.end(); ++it) {
            auto mobilityModule = (*it)->getSubmodule("mobility");
            auto mobility = dynamic_cast<IMobility *>(mobilityModule);
            if (mobility != nullptr)
                nodes.emplace_back(*it, mobility);
        }
    }
    const cModule *best = nullptr;
    double bestDistance = 10; // best-effort labeling: ignore matches further than 10m from any node
    for (auto& node : nodes) {
        double distance = const_cast<IMobility *>(node.second)->getCurrentPosition().distance(position);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = node.first;
        }
    }
    return best;
}

void TerrainObstacleLoss::logLineOfSightChange(const Coord& transmissionPosition, const Coord& receptionPosition, bool blocked) const
{
    auto transmitterNode = findNodeAt(transmissionPosition);
    auto receiverNode = findNodeAt(receptionPosition);
    if (transmitterNode == nullptr || receiverNode == nullptr)
        return; // could not attribute the positions to nodes
    auto key = std::make_pair(transmitterNode, receiverNode);
    auto it = pairBlocked.find(key);
    if (it == pairBlocked.end()) {
        if (blocked)
            EV_WARN << "Terrain occlusion: NO line of sight between " << transmitterNode->getFullName() << " and " << receiverNode->getFullName()
                    << " — link cannot be established (terrain blocks the direct ray)\n";
        else
            EV_INFO << "Terrain occlusion: line of sight established between " << transmitterNode->getFullName() << " and " << receiverNode->getFullName() << "\n";
        pairBlocked[key] = blocked;
    }
    else if (it->second != blocked) {
        if (blocked)
            EV_WARN << "Terrain occlusion: line of sight LOST between " << transmitterNode->getFullName() << " and " << receiverNode->getFullName()
                    << " — link lost (terrain blocks the direct ray)\n";
        else
            EV_INFO << "Terrain occlusion: line of sight RE-ESTABLISHED between " << transmitterNode->getFullName() << " and " << receiverNode->getFullName() << "\n";
        it->second = blocked;
    }
}

const Heightfield *TerrainObstacleLoss::getHeightfield() const
{
    if (heightfield == nullptr) {
        auto ground = physicalEnvironment->getGround();
        auto pointCloudGround = dynamic_cast<const physicalenvironment::PointCloudGround *>(ground);
        if (pointCloudGround == nullptr)
            throw cRuntimeError("TerrainObstacleLoss requires the physical environment's ground to be a PointCloudGround (found %s)",
                    ground == nullptr ? "no ground" : "another ground type");
        heightfield = &pointCloudGround->getHeightfield();
    }
    return heightfield;
}

std::ostream& TerrainObstacleLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "TerrainObstacleLoss";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(mode, mode == LOS ? "los" : "fresnel") << EV_FIELD(sampleStep);
    return stream;
}

double TerrainObstacleLoss::computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const
{
    const Heightfield *hf = getHeightfield();
    double dx = receptionPosition.x - transmissionPosition.x;
    double dy = receptionPosition.y - transmissionPosition.y;
    double dz = receptionPosition.z - transmissionPosition.z;
    double horizontalDistance = std::sqrt(dx * dx + dy * dy);
    double totalDistance = transmissionPosition.distance(receptionPosition);
    double step = sampleStep > 0 ? sampleStep : hf->getCellSize();
    int numSamples = std::min(4096, std::max(2, (int)(horizontalDistance / step) + 1));
    double waveLength = SPEED_OF_LIGHT / frequency.get<Hz>();
    // Walk interior samples only, so an antenna standing on the surface does not
    // occlude itself. LOS mode stops at the first obstruction; FRESNEL mode scans
    // the whole profile for the worst diffraction parameter v (the deepest
    // intrusion into the first Fresnel zone, in zone-normalized units), which the
    // single knife-edge curve J(v) then maps to a graded loss.
    bool blocked = false;
    double worstV = -INFINITY;
    double worstT = NaN;
    double worstGroundZ = NaN;
    for (int i = 1; i < numSamples - 1; i++) {
        double t = (double)i / (numSamples - 1);
        double groundZ = hf->getElevation(transmissionPosition.x + dx * t, transmissionPosition.y + dy * t);
        if (std::isnan(groundZ))
            continue; // outside the data extent: nothing to block there
        double rayZ = transmissionPosition.z + dz * t;
        if (mode == LOS) {
            if (groundZ > rayZ) {
                blocked = true; // terrain or building blocks the direct ray
                worstT = t;
                worstGroundZ = groundZ;
                break;
            }
        }
        else {
            double d1 = totalDistance * t;
            double d2 = totalDistance - d1;
            double v = (groundZ - rayZ) * std::sqrt(2 * totalDistance / (waveLength * d1 * d2));
            if (v > worstV) {
                worstV = v;
                worstT = t;
                worstGroundZ = groundZ;
            }
        }
    }
    double lossFactor;
    if (mode == LOS)
        lossFactor = blocked ? 0 : 1;
    else {
        double lossDb = std::isfinite(worstV) ? computeKnifeEdgeLoss(worstV) : 0;
        lossFactor = std::pow(10.0, -lossDb / 10.0);
        blocked = worstV > 0; // the direct ray itself is geometrically obstructed
    }
    if (lossFactor < 1) {
        Coord rayPoint(transmissionPosition.x + dx * worstT, transmissionPosition.y + dy * worstT, transmissionPosition.z + dz * worstT);
        Coord groundPoint(rayPoint.x, rayPoint.y, worstGroundZ);
        emitObstaclePenetrated(rayPoint, groundPoint, lossFactor);
    }
    if (logLinkEvents)
        logLineOfSightChange(transmissionPosition, receptionPosition, blocked);
    return lossFactor;
}

} // namespace physicallayer

} // namespace inet
