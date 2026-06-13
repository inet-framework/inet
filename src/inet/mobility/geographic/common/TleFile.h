//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TLEFILE_H
#define __INET_TLEFILE_H

#include "inet/common/INETDefs.h"


#include <string>
#include <vector>

#include "inet/mobility/geographic/sgp4/SGP4.h"

namespace inet {

/**
 * One satellite parsed from a TLE (two-line element) file: its catalog name (the
 * optional line preceding the element set) and the SGP4 orbital element record
 * initialized by Vallado's twoline2rv().
 */
class INET_API TleRecord
{
  public:
    std::string name;
    elsetrec satrec;
};

/**
 * Loads a TLE file (optionally with a leading name line per satellite, the common
 * "3-line" format) and initializes an SGP4 element record for each satellite using
 * the embedded Vallado reference implementation. Satellites can be looked up by
 * index, by name, or by NORAD catalog number.
 */
class INET_API TleFile
{
  protected:
    std::vector<TleRecord> records;

  public:
    /** Reads and parses the given TLE file. Throws on I/O or format errors. */
    void load(const char *fileName, gravconsttype gravityConstants = wgs84);

    int getNumSatellites() const { return (int)records.size(); }
    const TleRecord& getRecord(int index) const { return records.at(index); }

    /** Returns the index of the satellite with the given name, or -1 if not found. */
    int findByName(const char *name) const;
    /** Returns the index of the satellite with the given NORAD catalog number, or -1. */
    int findByCatalogNumber(const char *catalogNumber) const;
};

} // namespace inet


#endif

