//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINEMANAGER_H
#define __INET_LINEMANAGER_H

#include "inet/common/geometry/common/Coord.h"

namespace inet {

namespace visualizer {

class INET_API LineManager
{
  public:
    class INET_API ModuleLine {
      public:
        const int sourceModuleId;
        const int destinationModuleId;
        mutable double shiftPriority;
        mutable int shiftOffset;

      public:
        ModuleLine(int sourceModuleId, int destinationModuleId, double shiftPriority = 0) : sourceModuleId(sourceModuleId), destinationModuleId(destinationModuleId), shiftPriority(shiftPriority), shiftOffset(0) {}
    };

    class INET_API ModulePath {
      public:
        const std::vector<int> moduleIds;
        mutable double shiftPriority;
        mutable std::vector<int> shiftOffsets;

        ModulePath(const std::vector<int>& moduleIds, double shiftPriority = 0) : moduleIds(moduleIds), shiftPriority(shiftPriority) { shiftOffsets.resize(moduleIds.size() - 1); }
    };

    class INET_API CacheEntry {
      public:
        std::vector<const ModuleLine *> moduleLines;
        std::vector<std::pair<const ModulePath *, int>> modulePaths;
    };

  protected:
    std::map<std::pair<int, int>, CacheEntry> cacheEntries;

  protected:
    std::pair<int, int> getKey(int fromModuleId, int toModuleId) {
        return fromModuleId < toModuleId ? std::pair<int, int>(fromModuleId, toModuleId) : std::pair<int, int>(toModuleId, fromModuleId);
    }

    void updateOffsets(int fromModuleId, int toModuleId);
    void updateOffsets(const ModuleLine *moduleLine);
    void updateOffsets(const ModulePath *modulePath);

    static bool compareModuleLines(const ModuleLine *moduleLine1, const ModuleLine *moduleLine2);
    static bool compareModulePaths(std::pair<const ModulePath *, int> element1, std::pair<const ModulePath *, int> element2);

  public:
    void addModuleLine(const ModuleLine *moduleLine);
    void removeModuleLine(const ModuleLine *moduleLine);

    void addModulePath(const ModulePath *modulePath);
    void removeModulePath(const ModulePath *modulePath);

    Coord getLineShift(int sourceModuleId, int destinationModuleId, const Coord& sourcePosition, const Coord& destinationPosition, const char *shiftMode, int shiftOffset);

    static LineManager *getCanvasLineManager(const cCanvas *canvas);
    static LineManager *getOsgLineManager(const cCanvas *canvas);
};

} // namespace visualizer

} // namespace inet

#endif

