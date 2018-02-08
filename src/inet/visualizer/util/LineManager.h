//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
        ModuleLine(int sourceModuleId, int destinationModuleId, double shiftPriority = 0) : sourceModuleId(sourceModuleId), destinationModuleId(destinationModuleId), shiftPriority(shiftPriority), shiftOffset(0) { }
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

    static std::map<const cCanvas *, LineManager> canvasLineManagers;
    static std::map<const cCanvas *, LineManager> osgLineManagers;

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

#endif // ifndef __INET_LINEMANAGER_H

