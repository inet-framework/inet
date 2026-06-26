//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GNSSTRACKMOBILITY_H
#define __INET_GNSSTRACKMOBILITY_H

#include <vector>

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * Mobility model that follows a recorded GNSS (WGS84) track read from a CSV, GPX, or
 * GeoJSON LineString source. The playback start time, looping (none / repeat /
 * ping-pong), and great-circle interpolation between waypoints are handled by the
 * nested ~GnssTrackMobility::Track helper, so the path stays on the Earth's surface
 * even between far-apart points (suitable for both cars and aircraft).
 *
 * Timing is auto-detected: if the source provides per-point timestamps they drive
 * the motion; otherwise the 'speed' parameter is used to traverse the track at a
 * constant ground speed. Geographic positions are mapped into the scene through the
 * geographic coordinate system; with faceForward the body +X axis is aligned with
 * the direction of travel.
 */
class INET_API GnssTrackMobility : public MovingMobilityBase
{
  public:
    /**
     * A geographic track (a polyline of WGS84 waypoints with a time for each) plus
     * the playback policy: a start time, a loop mode, and great-circle interpolation
     * between waypoints. Kept independent of OMNeT++ scheduling so the timing/looping/
     * interpolation can be unit tested headlessly.
     *
     * Waypoints far enough apart that a straight Cartesian line between them would cut
     * through the Earth are interpolated along the great circle (see
     * wgs84::interpolateGreatCircle), so the track follows the surface for both cars
     * and aircraft.
     */
    class INET_API Track
    {
      public:
        enum LoopMode { LOOP_NONE, LOOP_REPEAT, LOOP_PINGPONG };

        std::vector<GeoCoord> waypoints;
        std::vector<double> times; // seconds, ascending, normalized so times[0] == 0
        LoopMode loopMode = LOOP_NONE;
        double startTime = 0; // simulation time (s) at which playback of the first waypoint begins

      public:
        /** Normalizes the timestamps so the first is 0 (call after filling waypoints/times from a timed source). */
        void normalizeTimes();

        /** Derives times from a constant ground speed (m/s) using cumulative great-circle distance. */
        void computeTimesFromSpeed(double speed);

        /** Duration of one pass over the track in seconds. */
        double getDuration() const { return times.empty() ? 0 : times.back(); }

        /** Largest segment ground speed in m/s (0 for a degenerate track). */
        double getMaxSpeed() const;

        /** True when the node is moving at the given simulation time (false before start, and after the end in LOOP_NONE). */
        bool isMoving(double simTimeSec) const;

        /** Interpolated geographic position at the given simulation time. */
        GeoCoord positionAt(double simTimeSec) const;

        /** True if the phase advances by a full dt between simTimeSec and simTimeSec+dt, i.e. no loop
            wrap (LOOP_REPEAT) or fold (LOOP_PINGPONG) lies in between, so a forward finite difference
            is safe to estimate velocity. */
        bool phaseAdvancesForward(double simTimeSec, double dt) const;

      protected:
        // Maps elapsed time since startTime to a phase in [0, duration] according to the loop mode.
        double mapPhase(double localTime) const;
    };

  protected:
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;
    Track track;
    double maxSpeed = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;

    // Reads the track waypoints (and timestamps when present) from the configured source.
    // Returns true when the source provided per-point timestamps.
    virtual bool readTrack();
    virtual bool readCsv(const char *data);
    virtual bool readGpx(const char *data);
    virtual bool readGeoJson(cObject *json);

    // Updates lastPosition/lastVelocity (and the stationary flag) for the given simulation time.
    virtual void updateState(simtime_t time);

  public:
    virtual double getMaxSpeed() const override { return maxSpeed; }
};

} // namespace inet

#endif
