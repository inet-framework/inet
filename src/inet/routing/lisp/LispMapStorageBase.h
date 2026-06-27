//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPMAPSTORAGEBASE_H
#define __INET_LISPMAPSTORAGEBASE_H

#include <list>

#include "inet/routing/lisp/LispCommon.h"
#include "inet/routing/lisp/LispEidPrefix.h"
#include "inet/routing/lisp/LispMapEntry.h"
#include "inet/routing/lisp/LispMessages_m.h"

namespace inet {
namespace lisp {

typedef std::list<LispMapEntry> MapStorage;
typedef MapStorage::iterator MapStorageItem;
typedef MapStorage::const_iterator MapStorageCItem;

/**
 * Common storage for the LISP map-cache and map-database: a list of
 * LispMapEntry with lookup-by-EID (longest-prefix-match) and update from
 * received Map-Reply/Register records.
 */
class INET_API LispMapStorageBase
{
  protected:
    MapStorage MappingStorage; ///< the EID-to-RLOC mappings (cache or database)

  public:
    LispMapStorageBase() {}
    virtual ~LispMapStorageBase() { clearMappingStorage(); }

    MapStorage& getMappingStorage() { return MappingStorage; }
    void clearMappingStorage() { MappingStorage.clear(); }

    std::string str() const;

    /** Loads static mappings from XML (<EID address="a/n"><RLOC .../></EID>...). */
    void parseMapEntry(cXMLElement *config);
    void addMapEntry(LispMapEntry& entry);
    /** Creates/updates a mapping from a received Map-Reply/Register record. */
    bool updateMapEntry(const LispMapRecord& record);
    bool syncMapEntry(LispMapEntry& entry);
    void removeMapEntry(const LispMapEntry& entry);

    LispMapEntry *findMapEntryByEidPrefix(const LispEidPrefix& eidpref);
    LispMapEntry *findMapEntryFromByLocator(const L3Address& rloc, const LispEidPrefix& eidPref);
    MapStorage findMapEntriesByLocator(const L3Address& rloc);

    /** Longest-prefix-match lookup of the EID address. */
    LispMapEntry *lookupMapEntry(L3Address address);
};

std::ostream& operator<<(std::ostream& os, const LispMapStorageBase& msb);
std::ostream& operator<<(std::ostream& os, const MapStorage& mapstor);

} // namespace lisp
} // namespace inet

#endif
