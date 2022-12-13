//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/LineManager.h"

#include <algorithm>

namespace inet {

namespace visualizer {


LineManager *LineManager::getCanvasLineManager(const cCanvas *canvas)
{
    static int canvasLineManagersHandle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::canvasLineManagers");
    auto& canvasLineManagers = getSimulationOrSharedDataManager()->getSharedVariable<std::map<const cCanvas *, LineManager>>(canvasLineManagersHandle);
    return &canvasLineManagers[canvas];  // inserts element if not yet exists
}

LineManager *LineManager::getOsgLineManager(const cCanvas *canvas)
{
    static int osgLineManagersHandle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::osgLineManagers");
    auto& osgLineManagers = getSimulationOrSharedDataManager()->getSharedVariable<std::map<const cCanvas *, LineManager>>(osgLineManagersHandle);
    return &osgLineManagers[canvas];  // inserts element if not yet exists
}

bool LineManager::compareModuleLines(const ModuleLine *moduleLine1, const ModuleLine *moduleLine2)
{
    auto v1 = moduleLine1->sourceModuleId < moduleLine1->destinationModuleId ? moduleLine1->shiftPriority : -moduleLine1->shiftPriority;
    auto v2 = moduleLine2->sourceModuleId < moduleLine2->destinationModuleId ? moduleLine2->shiftPriority : -moduleLine2->shiftPriority;
    return v1 < v2;
}

bool LineManager::compareModulePaths(std::pair<const ModulePath *, int> element1, std::pair<const ModulePath *, int> element2)
{
    auto v1 = element1.first->moduleIds[element1.second] < element1.first->moduleIds[element1.second + 1] ? element1.first->shiftPriority : -element1.first->shiftPriority;
    auto v2 = element2.first->moduleIds[element2.second] < element2.first->moduleIds[element2.second + 1] ? element2.first->shiftPriority : -element2.first->shiftPriority;
    return v1 < v2;
}

void LineManager::updateOffsets(int fromModuleId, int toModuleId)
{
    auto key = getKey(fromModuleId, toModuleId);
    auto& cacheEntry = cacheEntries[key];
    std::sort(cacheEntry.moduleLines.begin(), cacheEntry.moduleLines.end(), compareModuleLines);
    std::sort(cacheEntry.modulePaths.begin(), cacheEntry.modulePaths.end(), compareModulePaths);
    int shiftOffset = 0;
    for (auto& moduleLine : cacheEntry.moduleLines)
        moduleLine->shiftOffset = shiftOffset++;
    for (auto& element : cacheEntry.modulePaths)
        element.first->shiftOffsets[element.second] = shiftOffset++;
}

void LineManager::updateOffsets(const ModuleLine *moduleLine)
{
    updateOffsets(moduleLine->sourceModuleId, moduleLine->destinationModuleId);
}

void LineManager::updateOffsets(const ModulePath *modulePath)
{
    for (size_t index = 1; index < modulePath->moduleIds.size(); index++)
        updateOffsets(modulePath->moduleIds[index - 1], modulePath->moduleIds[index]);
}

void LineManager::addModuleLine(const ModuleLine *moduleLine)
{
    auto key = getKey(moduleLine->sourceModuleId, moduleLine->destinationModuleId);
    auto& cacheEntry = cacheEntries[key];
    cacheEntry.moduleLines.push_back(moduleLine);
    updateOffsets(moduleLine);
}

void LineManager::removeModuleLine(const ModuleLine *moduleLine)
{
    auto key = getKey(moduleLine->sourceModuleId, moduleLine->destinationModuleId);
    auto& cacheEntry = cacheEntries[key];
    cacheEntry.moduleLines.erase(std::remove(cacheEntry.moduleLines.begin(), cacheEntry.moduleLines.end(), moduleLine), cacheEntry.moduleLines.end());
    updateOffsets(moduleLine);
}

void LineManager::addModulePath(const ModulePath *modulePath)
{
    for (size_t index = 1; index < modulePath->moduleIds.size(); index++) {
        auto key = getKey(modulePath->moduleIds[index - 1], modulePath->moduleIds[index]);
        auto value = std::pair<const ModulePath *, int>(modulePath, index - 1);
        auto& cacheEntry = cacheEntries[key];
        cacheEntry.modulePaths.push_back(value);
    }
    updateOffsets(modulePath);
}

void LineManager::removeModulePath(const ModulePath *modulePath)
{
    for (size_t index = 1; index < modulePath->moduleIds.size(); index++) {
        auto key = getKey(modulePath->moduleIds[index - 1], modulePath->moduleIds[index]);
        auto value = std::pair<const ModulePath *, int>(modulePath, index - 1);
        auto& cacheEntry = cacheEntries[key];
        cacheEntry.modulePaths.erase(std::remove(cacheEntry.modulePaths.begin(), cacheEntry.modulePaths.end(), value), cacheEntry.modulePaths.end());
    }
    updateOffsets(modulePath);
}

Coord LineManager::getLineShift(int sourceModuleId, int destinationModuleId, const Coord& sourcePosition, const Coord& destinationPosition, const char *shiftMode, int shiftOffset)
{
    auto sign = *shiftMode;
    if (sign == '+' || sign == '-')
        shiftMode++;
    Coord shift;
    if (!strcmp("none", shiftMode))
        return Coord::ZERO;
    else if (!strcmp("normal", shiftMode)) {
        auto direction = sourceModuleId < destinationModuleId ? destinationPosition - sourcePosition : sourcePosition - destinationPosition;
        direction.normalize();
        shift = Coord(-direction.y, direction.x, 0);
    }
    else if (!strcmp("x", shiftMode))
        shift = Coord::X_AXIS;
    else if (!strcmp("y", shiftMode))
        shift = Coord::Y_AXIS;
    else if (!strcmp("z", shiftMode))
        shift = Coord::Z_AXIS;
    else
        throw cRuntimeError("Unknown shift mode: %s", shiftMode);
    if (sign == '+')
        shift *= shiftOffset;
    else if (sign == '-')
        shift *= -shiftOffset;
    else {
        auto& cacheEntry = cacheEntries[getKey(sourceModuleId, destinationModuleId)];
        auto count = cacheEntry.moduleLines.size() + cacheEntry.modulePaths.size();
        shift *= shiftOffset - ((double)count - 1) / 2.0;
    }

    double zoomLevel = cSimulation::getActiveEnvir()->getZoomLevel(cSimulation::getActiveSimulation()->getModule(sourceModuleId)->getParentModule());
    if (!std::isnan(zoomLevel))
        shift /= zoomLevel;

    return shift;
}

} // namespace visualizer

} // namespace inet

