//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispRlocator.h"

#include <sstream>

namespace inet {
namespace lisp {

LispRlocator::LispRlocator() :
    rloc(Ipv4Address::UNSPECIFIED_ADDRESS)
{
}

LispRlocator::LispRlocator(const char *addr) :
    rloc(addr)
{
}

LispRlocator::LispRlocator(const char *addr, const char *prio, const char *wei, bool loca) :
    rloc(L3Address(addr)),
    priority((unsigned char)atoi(prio)),
    weight((unsigned char)atoi(wei)),
    local(loca)
{
}

unsigned char LispRlocator::getPriority() const { return priority; }
void LispRlocator::setPriority(unsigned char priority) { this->priority = priority; }
const L3Address& LispRlocator::getRlocAddr() const { return rloc; }
void LispRlocator::setRlocAddr(const L3Address& rloc) { this->rloc = rloc; }
LispRlocator::LocatorState LispRlocator::getState() const { return state; }
void LispRlocator::setState(LocatorState state) { this->state = state; }
unsigned char LispRlocator::getWeight() const { return weight; }
void LispRlocator::setWeight(unsigned char weight) { this->weight = weight; }
unsigned char LispRlocator::getMpriority() const { return mpriority; }
void LispRlocator::setMpriority(unsigned char mpriority) { this->mpriority = mpriority; }
unsigned char LispRlocator::getMweight() const { return mweight; }
void LispRlocator::setMweight(unsigned char mweight) { this->mweight = mweight; }
bool LispRlocator::isLocal() const { return local; }
void LispRlocator::setLocal(bool local) { this->local = local; }

LispCommon::Afi LispRlocator::getRlocAfi() const
{
    return rloc.getType() == L3Address::IPv6 ? LispCommon::AFI_IPV6 : LispCommon::AFI_IPV4;
}

std::string LispRlocator::getStateString() const
{
    switch (state) {
        case UP: return "up";
        case ADMIN_DOWN: return "admin-down";
        case DOWN:
        default: return "down";
    }
}

std::string LispRlocator::str() const
{
    std::stringstream os;
    os << rloc << "\t(" << getStateString() << ")\tpri/wei=" << short(priority) << "/" << short(weight);
    if (local)
        os << "\tLocal";
    if (mpriority != DEFAULT_MPRIORITY_VAL && mweight != DEFAULT_MWEIGHT_VAL)
        os << " mpri/mwei=" << short(mpriority) << "/" << short(mweight);
    return os.str();
}

bool LispRlocator::operator==(const LispRlocator& other) const
{
    return rloc == other.rloc && priority == other.priority && weight == other.weight
           && mpriority == other.mpriority && mweight == other.mweight && state == other.state;
}

bool LispRlocator::operator<(const LispRlocator& other) const
{
    if (rloc.getType() != L3Address::IPv6 && other.rloc.getType() == L3Address::IPv6)
        return true;
    else if (rloc.getType() == L3Address::IPv6 && other.rloc.getType() != L3Address::IPv6)
        return false;
    if (rloc < other.rloc) return true;
    if (state < other.state) return true;
    if (priority < other.priority) return true;
    if (weight < other.weight) return true;
    if (mpriority < other.mpriority) return true;
    if (mweight < other.mweight) return true;
    return false;
}

void LispRlocator::updateRlocator(const LispRlocator& rloc)
{
    state = rloc.getState();
    priority = rloc.getPriority();
    weight = rloc.getWeight();
    mpriority = rloc.getMpriority();
    mweight = rloc.getMweight();
    local = rloc.isLocal();
}

std::ostream& operator<<(std::ostream& os, const LispRlocator& locator)
{
    return os << locator.str();
}

} // namespace lisp
} // namespace inet
