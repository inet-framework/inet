//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPSITEDATABASE_H
#define __INET_LISPSITEDATABASE_H

#include <list>

#include "inet/routing/lisp/LispMessages_m.h"
#include "inet/routing/lisp/LispSite.h"

namespace inet {
namespace lisp {

typedef std::list<LispSite> SiteStorage;
typedef SiteStorage::iterator SiteStorageItem;
typedef SiteStorage::const_iterator SiteStorageCItem;

/**
 * The Map-Server's site database: the configured LISP sites (name, key, EID
 * prefixes) and, for each, the ETRs that have registered. Owned by the Lisp
 * module. (ETR registration timeouts are managed by the owning module.)
 */
class INET_API LispSiteDatabase
{
  protected:
    SiteStorage SiteDatabase;

  public:
    SiteStorage& getSiteStorage() { return SiteDatabase; }

    /** Loads the configured sites from the <MapServer> config element. */
    void parseSites(cXMLElement *mapServerConfig);
    void addSite(LispSite& si) { SiteDatabase.push_back(si); }

    LispSite *findSiteInfoBySiteName(const std::string& siteName);
    LispSite *findSiteInfoByKey(const std::string& siteKey);
    LispSite *findSiteByAggregate(const L3Address& addr);

    /** Creates/updates the registering ETR's record within a site. */
    LispSiteRecord *updateSiteEtr(LispSite *si, L3Address src, bool proxy);
    /** Refreshes an ETR's registered mappings from a received Map-Register record. */
    void updateEtrEntries(LispSiteRecord *siteRec, const LispMapRecord& record);

    std::string str() const;
};

} // namespace lisp
} // namespace inet

#endif
