//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2006 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef WORLD_OBSTACLE_OBSTACLECONTROL_H
#define WORLD_OBSTACLE_OBSTACLECONTROL_H

#include <list>

#include "INETDefs.h"

#include "ModuleAccess.h"
#include "Coord.h"
#include "world/obstacles/Obstacle.h"
#include "world/annotations/AnnotationManager.h"

/**
 * ObstacleControl models obstacles that block radio transmissions.
 *
 * Each Obstacle is a polygon.
 * Transmissions that cross one of the polygon's lines will have
 * their receive power set to zero.
 */
class INET_API ObstacleControl : public cSimpleModule
{
    public:
        ~ObstacleControl();
        void initialize(int stage);
        int numInitStages() const { return 2; }
        void finish();
        void handleMessage(cMessage *msg);
        void handleSelfMsg(cMessage *msg);

        void addFromXml(cXMLElement* xml);
        void add(Obstacle obstacle);
        void erase(const Obstacle* obstacle);

        /**
         * calculate additional attenuation by obstacles, return signal strength
         */
        double calculateReceivedPower(double pSend, double carrierFrequency, const Coord& senderPos, double senderAngle, const Coord& receiverPos, double receiverAngle) const;

    protected:
        struct CacheKey {
            const double pSend;
            const double carrierFrequency;
            const Coord senderPos;
            const double senderAngle;
            const Coord receiverPos;
            const double receiverAngle;

            CacheKey(double pSend, double carrierFrequency, const Coord& senderPos, double senderAngle, const Coord& receiverPos, double receiverAngle) :
                pSend(pSend),
                carrierFrequency(carrierFrequency),
                senderPos(senderPos),
                senderAngle(senderAngle),
                receiverPos(receiverPos),
                receiverAngle(receiverAngle) {
            }
            bool operator<(const CacheKey& o) const {
                if (senderPos.x < o.senderPos.x) return true;
                if (senderPos.x > o.senderPos.x) return false;
                if (senderPos.y < o.senderPos.y) return true;
                if (senderPos.y > o.senderPos.y) return false;
                if (receiverPos.x < o.receiverPos.x) return true;
                if (receiverPos.x > o.receiverPos.x) return false;
                if (receiverPos.y < o.receiverPos.y) return true;
                if (receiverPos.y > o.receiverPos.y) return false;
                if (pSend < o.pSend) return true;
                if (pSend > o.pSend) return false;
                if (senderAngle < o.senderAngle) return true;
                if (senderAngle > o.senderAngle) return false;
                if (receiverAngle < o.receiverAngle) return true;
                if (receiverAngle > o.receiverAngle) return false;
                if (carrierFrequency < o.carrierFrequency) return true;
                if (carrierFrequency > o.carrierFrequency) return false;
                return false;
            }
        };

        enum { GRIDCELL_SIZE = 1024 };

        typedef std::list<Obstacle*> ObstacleGridCell;
        typedef std::vector<ObstacleGridCell> ObstacleGridRow;
        typedef std::vector<ObstacleGridRow> Obstacles;
        typedef std::map<CacheKey, double> CacheEntries;

        bool debug; /**< whether to emit debug messages */
        cXMLElement* obstaclesXml; /**< obstacles to add at startup */

        Obstacles obstacles;
        AnnotationManager* annotations;
        AnnotationManager::Group* annotationGroup;
        mutable CacheEntries cacheEntries;
};

class ObstacleControlAccess
{
    public:
        ObstacleControlAccess() {
        }

        ObstacleControl* getIfExists() {
            return dynamic_cast<ObstacleControl*>(simulation.getModuleByPath("obstacles"));
        }
};

#endif

