//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/GnssTrackMobility.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

Define_Module(GnssTrackMobility);

// --- small helpers for walking parsed GeoJSON (cValueMap/cValueArray) trees ---
static cValueMap *asMap(const cValue& value)
{
    return value.containsObject() ? dynamic_cast<cValueMap *>(value.objectValue()) : nullptr;
}

static cValueArray *asArray(const cValue& value)
{
    return value.containsObject() ? dynamic_cast<cValueArray *>(value.objectValue()) : nullptr;
}

static cValueMap *childMap(cValueMap *map, const char *key)
{
    return (map != nullptr && map->containsKey(key)) ? asMap(map->get(key)) : nullptr;
}

static cValueArray *childArray(cValueMap *map, const char *key)
{
    return (map != nullptr && map->containsKey(key)) ? asArray(map->get(key)) : nullptr;
}

static double coordinateNumber(cValueArray *array, int index)
{
    const cValue& value = array->get(index);
    if (value.getType() != cValue::DOUBLE && value.getType() != cValue::INT)
        throw cRuntimeError("GnssTrackMobility: GeoJSON coordinate components must be numbers");
    return value.doubleValue();
}

static const char *childString(cValueMap *map, const char *key, const char *defaultValue)
{
    if (map != nullptr && map->containsKey(key) && map->get(key).getType() == cValue::STRING)
        return map->get(key).stringValue();
    return defaultValue;
}

// Parses an ISO-8601 UTC timestamp (e.g. 2026-06-21T12:00:00Z) to seconds.
static double parseIsoToSeconds(const char *s)
{
    int year, month, day, hour, minute;
    double second;
    if (sscanf(s, "%d-%d-%dT%d:%d:%lf", &year, &month, &day, &hour, &minute, &second) != 6)
        throw cRuntimeError("GnssTrackMobility: cannot parse timestamp '%s', expected ISO-8601 e.g. 2026-06-21T12:00:00Z", s);
    return wgs84::julianDateFromUtc(year, month, day, hour, minute, second) * 86400.0;
}

void GnssTrackMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);

        bool hasTimestamps = readTrack();
        if (track.waypoints.empty())
            throw cRuntimeError("GnssTrackMobility: the track has no waypoints");

        track.startTime = par("trackStartTime").doubleValueInUnit("s");
        const char *loopMode = par("loopMode");
        if (!strcmp(loopMode, "repeat"))
            track.loopMode = Track::LOOP_REPEAT;
        else if (!strcmp(loopMode, "pingPong"))
            track.loopMode = Track::LOOP_PINGPONG;
        else
            track.loopMode = Track::LOOP_NONE;

        if (hasTimestamps)
            track.normalizeTimes();
        else {
            double speed = par("speed").doubleValueInUnit("mps");
            if (std::isnan(speed))
                throw cRuntimeError("GnssTrackMobility: the track has no timestamps; set the 'speed' parameter to traverse it at a constant ground speed");
            track.computeTimesFromSpeed(speed);
        }
        maxSpeed = track.getMaxSpeed();
        EV_INFO << "GnssTrackMobility: loaded " << track.waypoints.size() << " waypoints, duration "
                << track.getDuration() << "s, maxSpeed " << maxSpeed << "m/s" << endl;
    }
}

bool GnssTrackMobility::readTrack()
{
    const char *format = par("trackFormat");
    cObject *json = par("trackJson").objectValue(); // nullptr unless a GeoJSON object was given
    bool useGeoJson = !strcmp(format, "geojson") || (!strcmp(format, "auto") && json != nullptr);
    if (useGeoJson) {
        if (json == nullptr)
            throw cRuntimeError("GnssTrackMobility: trackFormat is 'geojson' but the 'trackJson' parameter is not set (use readJSON(\"...\"))");
        return readGeoJson(json);
    }
    const char *trackData = par("trackData").stringValue();
    if (!*trackData)
        throw cRuntimeError("GnssTrackMobility: the 'trackData' parameter is not set");
    // auto-detect GPX (XML) by its leading '<'; otherwise treat the data as CSV
    size_t start = std::string(trackData).find_first_not_of(" \t\r\n");
    bool looksLikeXml = start != std::string::npos && trackData[start] == '<';
    bool useGpx = !strcmp(format, "gpx") || (!strcmp(format, "auto") && looksLikeXml);
    return useGpx ? readGpx(trackData) : readCsv(trackData);
}

bool GnssTrackMobility::readCsv(const char *data)
{
    std::istringstream in(data);
    std::string line;
    bool allTimed = true;
    while (std::getline(in, line)) {
        // skip blank lines, comments, and a possible header row (non-numeric first field)
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            continue;
        char first = line[start];
        if (!(isdigit((unsigned char)first) || first == '+' || first == '-' || first == '.'))
            continue;
        cStringTokenizer tokenizer(line.c_str(), ", \t\r");
        std::vector<double> values;
        while (tokenizer.hasMoreTokens())
            values.push_back(atof(tokenizer.nextToken()));
        if (values.size() < 2)
            continue;
        double altitude = values.size() >= 3 ? values[2] : 0;
        track.waypoints.push_back(GeoCoord(deg(values[0]), deg(values[1]), m(altitude)));
        if (values.size() >= 4)
            track.times.push_back(values[3]);
        else
            allTimed = false;
    }
    bool hasTimestamps = allTimed && track.times.size() == track.waypoints.size() && !track.waypoints.empty();
    if (!hasTimestamps)
        track.times.clear();
    return hasTimestamps;
}

bool GnssTrackMobility::readGpx(const char *data)
{
    cXMLElement *root = getEnvir()->getParsedXMLString(data, nullptr);
    if (root == nullptr)
        throw cRuntimeError("GnssTrackMobility: cannot parse the GPX 'trackData'");
    cXMLElementList points = root->getElementsByTagName("trkpt");
    if (points.empty())
        points = root->getElementsByTagName("rtept"); // route points as a fallback
    bool allTimed = true;
    for (auto point : points) {
        const char *latString = point->getAttribute("lat");
        const char *lonString = point->getAttribute("lon");
        if (latString == nullptr || lonString == nullptr)
            continue;
        double altitude = 0;
        if (auto ele = point->getFirstChildWithTag("ele"))
            if (ele->getNodeValue() != nullptr)
                altitude = atof(ele->getNodeValue());
        track.waypoints.push_back(GeoCoord(deg(atof(latString)), deg(atof(lonString)), m(altitude)));
        auto timeElement = point->getFirstChildWithTag("time");
        if (timeElement != nullptr && timeElement->getNodeValue() != nullptr)
            track.times.push_back(parseIsoToSeconds(timeElement->getNodeValue()));
        else
            allTimed = false;
    }
    bool hasTimestamps = allTimed && track.times.size() == track.waypoints.size() && !track.waypoints.empty();
    if (!hasTimestamps)
        track.times.clear();
    return hasTimestamps;
}

bool GnssTrackMobility::readGeoJson(cObject *json)
{
    auto rootMap = dynamic_cast<cValueMap *>(json);
    if (rootMap == nullptr)
        throw cRuntimeError("GnssTrackMobility: 'trackJson' must be a GeoJSON object (e.g. readJSON(\"track.geojson\"))");
    cValueMap *geometry = nullptr;
    cValueMap *properties = nullptr;
    const char *type = childString(rootMap, "type", "");
    if (!strcmp(type, "FeatureCollection")) {
        auto features = childArray(rootMap, "features");
        if (features != nullptr)
            for (int i = 0; i < features->size() && geometry == nullptr; i++) {
                auto feature = asMap(features->get(i));
                auto geom = childMap(feature, "geometry");
                const char *geomType = childString(geom, "type", "");
                if (!strcmp(geomType, "LineString") || !strcmp(geomType, "MultiLineString")) {
                    geometry = geom;
                    properties = childMap(feature, "properties");
                }
            }
    }
    else if (!strcmp(type, "Feature")) {
        geometry = childMap(rootMap, "geometry");
        properties = childMap(rootMap, "properties");
    }
    else
        geometry = rootMap; // a bare geometry object

    const char *geometryType = childString(geometry, "type", "");
    cValueArray *coordinates = childArray(geometry, "coordinates");
    cValueArray *lineCoordinates = nullptr;
    if (!strcmp(geometryType, "LineString"))
        lineCoordinates = coordinates;
    else if (!strcmp(geometryType, "MultiLineString"))
        lineCoordinates = (coordinates != nullptr && coordinates->size() > 0) ? asArray(coordinates->get(0)) : nullptr;
    else
        throw cRuntimeError("GnssTrackMobility: the GeoJSON track must be a LineString or MultiLineString geometry, got '%s'", geometryType);
    if (lineCoordinates == nullptr)
        throw cRuntimeError("GnssTrackMobility: the GeoJSON track has no coordinates");
    for (int i = 0; i < lineCoordinates->size(); i++) {
        auto position = asArray(lineCoordinates->get(i));
        if (position == nullptr || position->size() < 2)
            continue;
        double altitude = position->size() >= 3 ? coordinateNumber(position, 2) : 0;
        // GeoJSON positions are [longitude, latitude, altitude] (RFC 7946), so element 0 is the
        // longitude and element 1 is the latitude - GeoCoord takes (latitude, longitude, altitude).
        track.waypoints.push_back(GeoCoord(deg(coordinateNumber(position, 1)), deg(coordinateNumber(position, 0)), m(altitude)));
    }
    // optional per-point timestamps from properties.coordTimes (array of ISO-8601 strings, as some exporters emit)
    auto coordTimes = childArray(properties, "coordTimes");
    if (coordTimes != nullptr && coordTimes->size() == (int)track.waypoints.size()) {
        for (int i = 0; i < coordTimes->size(); i++) {
            const cValue& time = coordTimes->get(i);
            if (time.getType() != cValue::STRING)
                throw cRuntimeError("GnssTrackMobility: GeoJSON coordTimes entries must be ISO-8601 strings");
            track.times.push_back(parseIsoToSeconds(time.stringValue()));
        }
        return true;
    }
    return false;
}

void GnssTrackMobility::setInitialPosition()
{
    updateState(simTime());
}

void GnssTrackMobility::move()
{
    updateState(simTime());
}

void GnssTrackMobility::updateState(simtime_t time)
{
    double t = time.dbl();
    GeoCoord geo = track.positionAt(t);
    lastPosition = coordinateSystem != nullptr ? coordinateSystem->computeSceneCoordinate(geo) : wgs84::geodeticToEcef(geo);
    if (track.isMoving(t)) {
        const double dt = 0.05; // finite-difference step for the velocity
        // sample the neighbour on the same playback branch: a forward difference would otherwise be
        // taken across a loop wrap (LOOP_REPEAT) or fold (LOOP_PINGPONG), yielding a spurious huge or
        // reversed velocity; near such a seam fall back to a backward difference instead
        double sampleTime = track.phaseAdvancesForward(t, dt) ? t + dt : t - dt;
        GeoCoord neighbourGeo = track.positionAt(sampleTime);
        Coord neighbourPosition = coordinateSystem != nullptr ? coordinateSystem->computeSceneCoordinate(neighbourGeo) : wgs84::geodeticToEcef(neighbourGeo);
        lastVelocity = sampleTime > t ? (neighbourPosition - lastPosition) / dt : (lastPosition - neighbourPosition) / dt;
    }
    else
        lastVelocity = Coord::ZERO;
    // in non-looping mode, stop scheduling updates once the end of the track is reached
    if (track.loopMode == Track::LOOP_NONE && t >= track.startTime + track.getDuration() && track.getDuration() > 0)
        stationary = true;
}

// --- GnssTrackMobility::Track: headless playback policy (timing, looping, great-circle interpolation) ---

void GnssTrackMobility::Track::normalizeTimes()
{
    if (times.empty())
        return;
    double t0 = times.front();
    for (auto& t : times)
        t -= t0;
}

void GnssTrackMobility::Track::computeTimesFromSpeed(double speed)
{
    if (speed <= 0)
        throw cRuntimeError("GnssTrackMobility::Track: a positive speed is required when the track has no timestamps");
    times.resize(waypoints.size());
    if (waypoints.empty())
        return;
    times[0] = 0;
    for (size_t i = 1; i < waypoints.size(); i++) {
        double distance = wgs84::greatCircleDistance(waypoints[i - 1], waypoints[i]);
        times[i] = times[i - 1] + distance / speed;
    }
}

double GnssTrackMobility::Track::getMaxSpeed() const
{
    double maxSpeed = 0;
    for (size_t i = 1; i < waypoints.size(); i++) {
        double dt = times[i] - times[i - 1];
        if (dt > 0) {
            // 3D distance including altitude (consistent with the ECEF velocity the model reports),
            // not just the great-circle surface distance, so a climbing/descending leg is not under-reported
            double distance = wgs84::geodeticToEcef(waypoints[i]).distance(wgs84::geodeticToEcef(waypoints[i - 1]));
            double speed = distance / dt;
            maxSpeed = std::max(maxSpeed, speed);
        }
    }
    return maxSpeed;
}

double GnssTrackMobility::Track::mapPhase(double localTime) const
{
    double duration = getDuration();
    if (duration <= 0 || localTime <= 0)
        return 0;
    switch (loopMode) {
        case LOOP_REPEAT:
            return std::fmod(localTime, duration);
        case LOOP_PINGPONG: {
            double period = 2 * duration;
            double m = std::fmod(localTime, period);
            return m <= duration ? m : period - m;
        }
        case LOOP_NONE:
        default:
            return std::min(localTime, duration);
    }
}

bool GnssTrackMobility::Track::phaseAdvancesForward(double simTimeSec, double dt) const
{
    double p0 = mapPhase(simTimeSec - startTime);
    double p1 = mapPhase(simTimeSec + dt - startTime);
    // forward stays on one branch when the phase moves by a full dt (LOOP_REPEAT and PINGPONG-ascending
    // give +dt, PINGPONG-descending gives -dt); a wrap or fold makes |p1 - p0| differ from dt
    return std::abs(std::abs(p1 - p0) - dt) <= dt * 1e-3;
}

bool GnssTrackMobility::Track::isMoving(double simTimeSec) const
{
    if (waypoints.size() < 2 || getDuration() <= 0)
        return false;
    double localTime = simTimeSec - startTime;
    if (localTime <= 0)
        return false;
    if (loopMode == LOOP_NONE && localTime >= getDuration())
        return false;
    return true;
}

GeoCoord GnssTrackMobility::Track::positionAt(double simTimeSec) const
{
    if (waypoints.empty())
        return GeoCoord::NIL;
    if (waypoints.size() == 1)
        return waypoints[0];
    double phase = mapPhase(simTimeSec - startTime);
    // find the segment [i, i+1] containing phase
    auto upper = std::upper_bound(times.begin(), times.end(), phase);
    int i = (int)(upper - times.begin()) - 1;
    if (i < 0)
        i = 0;
    if (i > (int)waypoints.size() - 2)
        i = (int)waypoints.size() - 2;
    double segmentDuration = times[i + 1] - times[i];
    double frac = segmentDuration > 0 ? (phase - times[i]) / segmentDuration : 0;
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    return wgs84::interpolateGreatCircle(waypoints[i], waypoints[i + 1], frac);
}

} // namespace inet
