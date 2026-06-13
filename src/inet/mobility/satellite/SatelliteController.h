//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SATELLITECONTROLLER_H
#define __INET_SATELLITECONTROLLER_H

#include "inet/common/INETDefs.h"

#ifdef INET_WITH_SATELLITE_MOBILITY

#include <string>
#include <vector>

#include "inet/common/SimpleModule.h"
#include "inet/mobility/satellite/common/TleFile.h"

namespace inet {

/**
 * Controls a set of satellite nodes propagated from one or more TLE files. The
 * controller is the authority for the absolute UTC epoch (mapped to simulation
 * time 0), which it distributes to the satellite mobilities it creates. It inserts
 * and removes satellite nodes on a configured schedule (at the start and/or at
 * explicit times).
 *
 * For each satellite it dynamically creates a node of a user-configurable type
 * (the ~ScenarioManager creation pattern) and injects the TLE selection and epoch
 * into the node's ~SatelliteMobility submodule. It keeps a registry of the live
 * satellite modules and emits signals on insertion and removal, so a visualizer
 * can enumerate and track them.
 */
class INET_API SatelliteController : public SimpleModule
{
  public:
    static simsignal_t satelliteInsertedSignal;
    static simsignal_t satelliteRemovedSignal;

  protected:
    class SatelliteEntry {
      public:
        std::string tleFile; // TLE file this satellite was read from
        int index = -1; // index within that file
        std::string name; // satellite name
        cModule *module = nullptr; // the live node, or nullptr when not inserted
    };

    class Event {
      public:
        simtime_t time;
        bool insert; // true: insert, false: remove
        int entry; // index into 'entries'
    };

    std::string epoch;
    std::string nodeTypeName;
    std::string nodeName;
    std::string coordinateSystemModule;
    std::string nodeIcon;
    double nodeIconRowX = 0;
    double nodeIconRowY = 0;
    double nodeIconSpacing = 0;

    std::vector<SatelliteEntry> entries;
    std::vector<Event> events; // sorted by time
    size_t nextEvent = 0;
    cMessage *timer = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    /** Reads the configured TLE files and builds the satellite entry list. */
    virtual void loadTleFiles();
    /** Builds and schedules the insertion/removal events. */
    virtual void scheduleEvents();
    virtual void scheduleNextEvent();

    virtual void insertSatellite(int entryIndex);
    virtual void removeSatellite(int entryIndex);

  public:
    virtual ~SatelliteController();

    /** The absolute UTC epoch (ISO-8601) mapped to simulation time 0. */
    virtual const char *getEpoch() const { return epoch.c_str(); }
    /** The currently live satellite modules (entries not yet inserted/already removed are null). */
    virtual std::vector<cModule *> getManagedSatellites() const;
};

} // namespace inet

#endif // INET_WITH_SATELLITE_MOBILITY

#endif

