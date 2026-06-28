//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPSITE_H
#define __INET_LISPSITE_H

#include "inet/routing/lisp/LispCommon.h"
#include "inet/routing/lisp/LispMapStorageBase.h"
#include "inet/routing/lisp/LispSiteRecord.h"

namespace inet {
namespace lisp {

/**
 * A LISP site as known by a Map-Server: its name, authentication key, the set of
 * EID prefixes it maintains (inherited storage) and the list of ETRs that have
 * registered for it.
 */
class INET_API LispSite : public LispMapStorageBase
{
  protected:
    std::string name;
    std::string key;
    Etrs ETRs;

  public:
    LispSite() {}
    LispSite(std::string nam, std::string ke) : name(nam), key(ke) {}
    virtual ~LispSite() {}

    bool operator==(const LispSite& other) const;

    std::string str() const;

    const std::string& getKey() const { return key; }
    void setKey(const std::string& key) { this->key = key; }
    const std::string& getSiteName() const { return name; }
    void setSiteName(const std::string& name) { this->name = name; }
    const Etrs& getETRs() const { return ETRs; }
    void setETRs(const Etrs& etrs) { ETRs = etrs; }
    void clearETRs() { ETRs.clear(); }

    void addRecord(LispSiteRecord& srec) { ETRs.push_back(srec); }
    void removeRecord(LispSiteRecord& srec) { ETRs.remove(srec); }
    LispSiteRecord *findRecordByAddress(L3Address& address);
    Etrs findAllRecordsByEid(const L3Address& address);
    bool isEidMaintained(const L3Address& address);
};

std::ostream& operator<<(std::ostream& os, const LispSite& si);

} // namespace lisp
} // namespace inet

#endif
