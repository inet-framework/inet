//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/obstacleloss/TerrainObstacleLoss.h"

#include <cmath>

#include "inet/environment/ground/PointCloudGround.h"

namespace inet {

namespace physicallayer {

Define_Module(TerrainObstacleLoss);

void TerrainObstacleLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physicalEnvironment.reference(this, "physicalEnvironmentModule", true);
        sampleStep = par("sampleStep");
        logLinkEvents = par("logLinkEvents");
    }
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
        stream << EV_FIELD(sampleStep);
    return stream;
}

double TerrainObstacleLoss::computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const
{
    const Heightfield *hf = getHeightfield();
    double dx = receptionPosition.x - transmissionPosition.x;
    double dy = receptionPosition.y - transmissionPosition.y;
    double dz = receptionPosition.z - transmissionPosition.z;
    double distance = std::sqrt(dx * dx + dy * dy);
    double step = sampleStep > 0 ? sampleStep : hf->getCellSize();
    int numSamples = std::min(4096, std::max(2, (int)(distance / step) + 1));
    // test interior samples only, so an antenna standing on the surface does not occlude itself
    bool blocked = false;
    for (int i = 1; i < numSamples - 1; i++) {
        double t = (double)i / (numSamples - 1);
        double groundZ = hf->getElevation(transmissionPosition.x + dx * t, transmissionPosition.y + dy * t);
        if (std::isnan(groundZ))
            continue; // outside the data extent: nothing to block there
        double rayZ = transmissionPosition.z + dz * t;
        if (groundZ > rayZ) {
            blocked = true; // terrain or building blocks the direct ray
            break;
        }
    }
    if (logLinkEvents)
        logLineOfSightChange(transmissionPosition, receptionPosition, blocked);
    return blocked ? 0 : 1;
}

} // namespace physicallayer

} // namespace inet
