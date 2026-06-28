//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispSiteDatabase.h"

#include <sstream>

#include "inet/routing/lisp/LispCommon.h"

namespace inet {
namespace lisp {

void LispSiteDatabase::parseSites(cXMLElement *mapServerConfig)
{
    if (!mapServerConfig)
        return;
    for (cXMLElement *m : mapServerConfig->getChildrenByTagName(SITE_TAG)) {
        if (!m->getAttribute(NAME_ATTR) || !m->getAttribute(KEY_ATTR)) {
            EV_WARN << "LISP config: <Site> missing 'name'/'key'\n";
            continue;
        }
        std::string nam = m->getAttribute(NAME_ATTR);
        std::string ke = m->getAttribute(KEY_ATTR);
        if (!nam.empty() && !ke.empty()) {
            LispSite site(nam, ke);
            site.parseMapEntry(m); // the <EID> children become the site's maintained prefixes
            addSite(site);
        }
    }
}

LispSite *LispSiteDatabase::findSiteInfoBySiteName(const std::string& siteName)
{
    for (auto& site : SiteDatabase)
        if (site.getSiteName() == siteName)
            return &site;
    return nullptr;
}

LispSite *LispSiteDatabase::findSiteInfoByKey(const std::string& siteKey)
{
    for (auto& site : SiteDatabase)
        if (site.getKey() == siteKey)
            return &site;
    return nullptr;
}

LispSite *LispSiteDatabase::findSiteByAggregate(const L3Address& addr)
{
    LispSite *result = nullptr;
    int bestLen = 0;
    for (auto& site : SiteDatabase) {
        if (LispMapEntry *me = site.lookupMapEntry(addr)) {
            int len = LispCommon::doPrefixMatch(me->getEidPrefix().getEidAddr(), addr);
            if (len >= bestLen) {
                bestLen = len;
                result = &site;
            }
        }
    }
    return result;
}

LispSiteRecord *LispSiteDatabase::updateSiteEtr(LispSite *si, L3Address src, bool proxy)
{
    if (!si->findRecordByAddress(src)) {
        LispSiteRecord srec;
        srec.setServerEntry(LispServerEntry(src.str(), "", proxy, false, false));
        si->addRecord(srec);
    }
    LispSiteRecord *srec = si->findRecordByAddress(src);
    srec->getServerEntry().setLastTime(simTime());
    srec->getServerEntry().setProxyReply(proxy);
    return srec;
}

void LispSiteDatabase::updateEtrEntries(LispSiteRecord *siteRec, const LispMapRecord& record)
{
    siteRec->updateMapEntry(record);
}

std::string LispSiteDatabase::str() const
{
    std::stringstream os;
    for (const auto& site : SiteDatabase)
        os << site.str() << "\n";
    return os.str();
}

} // namespace lisp
} // namespace inet
