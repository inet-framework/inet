//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/geographic/SatelliteMobility.h"


#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

Define_Module(SatelliteMobility);

void SatelliteMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    EV_TRACE << "initializing SatelliteMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);

        // load the TLE file and select the satellite
        const char *tleFileName = par("tleFile").stringValue();
        if (!*tleFileName)
            throw cRuntimeError("The 'tleFile' parameter is not set (set it directly, or let a SatelliteController inject it)");
        tleFile.load(tleFileName);
        int index = par("satelliteIndex");
        const char *name = par("satelliteName");
        const char *catalogNumber = par("satelliteCatalogNumber");
        if (index < 0 && *name)
            index = tleFile.findByName(name);
        if (index < 0 && *catalogNumber)
            index = tleFile.findByCatalogNumber(catalogNumber);
        if (index < 0)
            index = 0;
        if (index >= tleFile.getNumSatellites())
            throw cRuntimeError("Satellite index %d out of range (file has %d satellites)", index, tleFile.getNumSatellites());
        satrec = tleFile.getRecord(index).satrec;
        // expose the resolved TLE name (so map visualizers can display it)
        par("satelliteName").setStringValue(tleFile.getRecord(index).name.c_str());

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

        const char *pointingString = par("pointing");
        if (!strcmp(pointingString, "none"))
            pointing = POINTING_NONE;
        else if (!strcmp(pointingString, "nadir"))
            pointing = POINTING_NADIR;
        else if (!strcmp(pointingString, "velocity"))
            pointing = POINTING_VELOCITY;
        else
            throw cRuntimeError("Unknown pointing mode '%s'", pointingString);

        WATCH(epochJulianDate);
        WATCH(lastGeoPosition.latitude);
        WATCH(lastGeoPosition.longitude);
        WATCH(lastGeoPosition.altitude);
    }
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
    lastPosition = computeScenePosition(julianDate, lastGeoPosition);
    GeoCoord ignored = GeoCoord::NIL;
    Coord nextPosition = computeScenePosition(julianDate + 1.0 / 86400.0, ignored);
    lastVelocity = nextPosition - lastPosition; // per second
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
    lastPosition = computeScenePosition(julianDate, lastGeoPosition);
    GeoCoord ignored = GeoCoord::NIL;
    Coord nextPosition = computeScenePosition(julianDate + 1.0 / 86400.0, ignored);
    lastVelocity = nextPosition - lastPosition; // per second
}

void SatelliteMobility::orient()
{
    switch (pointing) {
        case POINTING_NONE:
            lastOrientation = Quaternion::IDENTITY;
            break;
        case POINTING_NADIR:
            // point the body +Z axis toward the Earth center (the scene origin in a geocentric frame)
            if (lastPosition != Coord::ZERO)
                lastOrientation = Quaternion::rotationFromTo(Coord::Z_AXIS, (-lastPosition).getNormalized());
            break;
        case POINTING_VELOCITY:
            // point the body +X axis along the velocity vector
            if (lastVelocity != Coord::ZERO)
                lastOrientation = Quaternion::rotationFromTo(Coord::X_AXIS, lastVelocity.getNormalized());
            break;
    }
}

} // namespace inet


