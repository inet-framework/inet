//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelSourceTable.h"

#include <sstream>

namespace inet {
namespace babel {

BabelSource::~BabelSource()
{
    deleteGCTimer();
}

std::string BabelSource::str() const
{
    std::stringstream out;
    out << prefix << " orig:" << originator << " FD:" << feasibleDistance;
    return out.str();
}

void BabelSource::resetGCTimer() { resetTimer(gcTimer, defval::SOURCE_GC_INTERVAL); }
void BabelSource::resetGCTimer(double delay) { resetTimer(gcTimer, delay); }
void BabelSource::deleteGCTimer() { deleteTimer(&gcTimer); }

BabelSourceTable::~BabelSourceTable()
{
    removeSources();
}

void BabelSourceTable::removeSources()
{
    for (auto source : sources)
        delete source;
    sources.clear();
}

BabelSource *BabelSourceTable::findSource(const netPrefix<L3Address>& p, const rid& orig)
{
    for (auto source : sources)
        if (source->getPrefix() == p && source->getOriginator() == orig)
            return source;
    return nullptr;
}

BabelSource *BabelSourceTable::addSource(BabelSource *source)
{
    ASSERT(source != nullptr);
    BabelSource *intable = findSource(source->getPrefix(), source->getOriginator());
    if (intable != nullptr)
        return intable;
    sources.push_back(source);
    return source;
}

void BabelSourceTable::removeSource(BabelSource *source)
{
    for (auto it = sources.begin(); it != sources.end(); ++it) {
        if (*it == source) {
            delete *it;
            sources.erase(it);
            return;
        }
    }
}

} // namespace babel
} // namespace inet
