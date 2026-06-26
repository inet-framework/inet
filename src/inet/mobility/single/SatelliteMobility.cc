//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/SatelliteMobility.h"


#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "inet/common/geometry/common/Wgs84.h"

namespace {

// One satellite parsed from TLE data: its catalog name (the optional line preceding
// the element set) and the SGP4 element record. Kept at file scope so the unqualified
// gravity-constant value 'wgs84' below names the SGP4 gravconsttype enum member, not
// the inet::wgs84 helper namespace pulled in by Wgs84.h.
struct TleEntry
{
    std::string name;
    elsetrec satrec;
};

std::string trim(const std::string& s)
{
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos)
        return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

bool isElementLine(const std::string& line, char which)
{
    // a TLE element line starts with the digit '1' or '2' followed by a space
    return line.size() >= 2 && line[0] == which && line[1] == ' ';
}

// Parses TLE data (optionally with a leading name line per satellite, the common
// "3-line" format) and initializes an SGP4 element record for each satellite using
// the embedded Vallado reference implementation. Throws on format errors.
std::vector<TleEntry> parseTleData(const char *tleData, gravconsttype gravityConstants = wgs84)
{
    std::istringstream in(tleData);

    std::vector<std::string> lines;
    std::string raw;
    while (std::getline(in, raw))
        lines.push_back(raw);

    std::vector<TleEntry> entries;
    std::string pendingName;
    for (size_t i = 0; i < lines.size(); i++) {
        std::string trimmed = trim(lines[i]);
        if (trimmed.empty())
            continue;
        if (isElementLine(trimmed, '1') && i + 1 < lines.size() && isElementLine(trim(lines[i + 1]), '2')) {
            // twoline2rv() writes into the line buffers, so use mutable copies
            char longstr1[130], longstr2[130];
            strncpy(longstr1, lines[i].c_str(), sizeof(longstr1) - 1);
            longstr1[sizeof(longstr1) - 1] = '\0';
            strncpy(longstr2, lines[i + 1].c_str(), sizeof(longstr2) - 1);
            longstr2[sizeof(longstr2) - 1] = '\0';

            TleEntry entry;
            double startmfe, stopmfe, deltamin;
            // typerun='c' (catalog) does not read from stdin; opsmode='i' is the improved mode
            SGP4Funcs::twoline2rv(longstr1, longstr2, 'c', 'c', 'i', gravityConstants,
                    startmfe, stopmfe, deltamin, entry.satrec);
            if (entry.satrec.error != 0)
                throw omnetpp::cRuntimeError("SGP4 initialization failed for satellite '%s' in the TLE data (error code %d)",
                        pendingName.empty() ? entry.satrec.satnum : pendingName.c_str(), entry.satrec.error);
            entry.name = !pendingName.empty() ? pendingName : std::string(entry.satrec.satnum);
            entries.push_back(entry);
            pendingName.clear();
            i++; // also consumed line i+1
        }
        else if (!isElementLine(trimmed, '1') && !isElementLine(trimmed, '2'))
            // a non-element line preceding an element set is the satellite name
            pendingName = trimmed;
    }

    if (entries.empty())
        throw omnetpp::cRuntimeError("No valid two-line element sets found in the 'tleData' parameter");
    return entries;
}

} // namespace

namespace inet {

Define_Module(SatelliteMobility);

void SatelliteMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    EV_TRACE << "initializing SatelliteMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);

        // load the TLE file and select the satellite into satrec
        loadTleRecord();

        // determine the UTC epoch (simulation time 0); default to the TLE epoch
        const char *epoch = par("epoch");
        if (*epoch) {
            int year, month, day, hour, minute;
            double second;
            if (sscanf(epoch, "%d-%d-%dT%d:%d:%lf", &year, &month, &day, &hour, &minute, &second) != 6)
                throw cRuntimeError("Cannot parse epoch '%s', expected ISO-8601 e.g. 2026-06-02T00:00:00Z", epoch);
            epochJulianDate = wgs84::julianDateFromUtc(year, month, day, hour, minute, second);
        }
        else
            epochJulianDate = satrec.jdsatepoch + satrec.jdsatepochF;

        const char *attitudeModeString = par("attitudeMode");
        if (!strcmp(attitudeModeString, "earth_fixed"))
            attitudeMode = ATTITUDE_EARTH_FIXED;
        else if (!strcmp(attitudeModeString, "star_fixed"))
            attitudeMode = ATTITUDE_STAR_FIXED;
        else if (!strcmp(attitudeModeString, "nadir"))
            attitudeMode = ATTITUDE_NADIR;
        else if (!strcmp(attitudeModeString, "zenith"))
            attitudeMode = ATTITUDE_ZENITH;
        else if (!strcmp(attitudeModeString, "velocity"))
            attitudeMode = ATTITUDE_VELOCITY;
        else
            throw cRuntimeError("Unknown attitude mode '%s'", attitudeModeString);

        WATCH(epochJulianDate);
        WATCH(lastGeoPosition.latitude);
        WATCH(lastGeoPosition.longitude);
        WATCH(lastGeoPosition.altitude);
    }
}

void SatelliteMobility::loadTleRecord()
{
    const char *tleData = par("tleData").stringValue();
    if (!*tleData)
        throw cRuntimeError("The 'tleData' parameter is not set");
    std::vector<TleEntry> entries = parseTleData(tleData);

    int index = par("satelliteIndex");
    const char *name = par("satelliteName");
    const char *catalogNumber = par("satelliteCatalogNumber");
    if (index < 0 && *name) {
        for (size_t i = 0; i < entries.size(); i++)
            if (entries[i].name == name) { index = (int)i; break; }
    }
    if (index < 0 && *catalogNumber) {
        for (size_t i = 0; i < entries.size(); i++)
            if (!strcmp(entries[i].satrec.satnum, catalogNumber)) { index = (int)i; break; }
    }
    if (index < 0)
        index = 0;
    if (index >= (int)entries.size())
        throw cRuntimeError("Satellite index %d out of range (the 'tleData' parameter has %d satellites)", index, (int)entries.size());
    satrec = entries[index].satrec;
    // expose the resolved name (the line preceding the element set) so the map/sky-view visualizers can
    // label the satellite even when it was selected by index or catalog number rather than by name
    if (!*name && !entries[index].name.empty())
        par("satelliteName").setStringValue(entries[index].name.c_str());
}

Coord SatelliteMobility::computeScenePosition(double julianDate, GeoCoord& geographicPosition)
{
    double tsince = (julianDate - (satrec.jdsatepoch + satrec.jdsatepochF)) * 1440.0; // minutes since TLE epoch
    double r[3], v[3];
    SGP4Funcs::sgp4(satrec, tsince, r, v);
    if (satrec.error != 0)
        EV_WARN << "SGP4 propagation error " << satrec.error << " at t=" << simTime() << " (e.g. decayed orbit)" << endl;
    Coord teme(r[0] * 1000, r[1] * 1000, r[2] * 1000); // km -> m, TEME (ECI) frame
    Coord ecef = wgs84::eciTemeToEcef(teme, wgs84::gmst(julianDate));
    geographicPosition = wgs84::ecefToGeodetic(ecef);
    return coordinateSystem != nullptr ? coordinateSystem->computeSceneCoordinate(geographicPosition) : ecef;
}

void SatelliteMobility::setInitialPosition()
{
    double julianDate = epochJulianDate; // simulation time 0
    currentJulianDate = julianDate;
    lastPosition = computeScenePosition(julianDate, lastGeoPosition);
    // velocity from a small centered finite difference; a 1 s forward difference lagged the true
    // instantaneous velocity by half a step. The difference stays in the rotating ECEF scene frame.
    const double halfStepDay = 0.05 / 86400.0; // +/- 0.05 s around the sample, expressed in days
    GeoCoord ignored = GeoCoord::NIL;
    Coord aheadPosition = computeScenePosition(julianDate + halfStepDay, ignored);
    Coord behindPosition = computeScenePosition(julianDate - halfStepDay, ignored);
    lastVelocity = (aheadPosition - behindPosition) / 0.1; // m/s over the 0.1 s centered step
    orient();
    EV_INFO << "Satellite initial state: lat=" << lastGeoPosition.latitude.get<deg>()
            << "deg lon=" << lastGeoPosition.longitude.get<deg>()
            << "deg alt=" << lastGeoPosition.altitude.get<m>() / 1000 << "km"
            << " geocentric radius=" << lastPosition.length() / 1000 << "km"
            << " speed=" << lastVelocity.length() << "m/s" << endl;
}

void SatelliteMobility::move()
{
    double julianDate = epochJulianDate + simTime().dbl() / 86400.0;
    currentJulianDate = julianDate;
    lastPosition = computeScenePosition(julianDate, lastGeoPosition);
    // centered finite difference for velocity (see setInitialPosition), stays in the ECEF scene frame
    const double halfStepDay = 0.05 / 86400.0; // +/- 0.05 s around the sample, expressed in days
    GeoCoord ignored = GeoCoord::NIL;
    Coord aheadPosition = computeScenePosition(julianDate + halfStepDay, ignored);
    Coord behindPosition = computeScenePosition(julianDate - halfStepDay, ignored);
    lastVelocity = (aheadPosition - behindPosition) / 0.1; // m/s over the 0.1 s centered step
}

void SatelliteMobility::orient()
{
    // Every mode aims the body +X axis (the forward/boresight axis the orientation visualizer draws).
    // rotationFromTo() only pins that axis, leaving roll free, which is what these attitudes require.
    switch (attitudeMode) {
        case ATTITUDE_EARTH_FIXED:
            // body axes held fixed in the ECEF scene frame (so they rotate with the Earth)
            lastOrientation = Quaternion::IDENTITY;
            break;
        case ATTITUDE_STAR_FIXED:
            // body axes held fixed in the inertial TEME (ECI) frame, i.e. pointing at the "fixed stars";
            // expressed in ECEF this is exactly the GMST rotation that maps ECI to ECEF position vectors
            lastOrientation = wgs84::eciTemeToEcefRotation(wgs84::gmst(currentJulianDate));
            break;
        case ATTITUDE_NADIR:
            // point +X toward the Earth center, i.e. the scene origin in the geocentric ECEF frame
            if (lastPosition != Coord::ZERO)
                lastOrientation = Quaternion::rotationFromTo(Coord::X_AXIS, (-lastPosition).getNormalized());
            break;
        case ATTITUDE_ZENITH:
            // point +X radially outward, away from the Earth center (the opposite of nadir)
            if (lastPosition != Coord::ZERO)
                lastOrientation = Quaternion::rotationFromTo(Coord::X_AXIS, lastPosition.getNormalized());
            break;
        case ATTITUDE_VELOCITY:
            // point +X along the velocity vector
            if (lastVelocity != Coord::ZERO)
                lastOrientation = Quaternion::rotationFromTo(Coord::X_AXIS, lastVelocity.getNormalized());
            break;
    }
}

} // namespace inet


