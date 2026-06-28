//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELSOURCETABLE_H
#define __INET_BABELSOURCETABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

/**
 * A "source" records the feasibility distance (RFC 6126, section 3.2.6): the
 * best (seqno, metric) this node has ever advertised for a (prefix, originator)
 * pair. The feasibility condition compares an incoming route's distance against
 * it to guarantee loop freedom.
 */
class INET_API BabelSource : public cObject
{
  protected:
    netPrefix<L3Address> prefix;
    rid originator;
    routeDistance feasibleDistance;
    BabelTimer *gcTimer = nullptr;

  public:
    BabelSource() {}
    BabelSource(const netPrefix<L3Address>& pre, const rid& orig, const routeDistance& fd, BabelTimer *gct) :
        prefix(pre), originator(orig), feasibleDistance(fd), gcTimer(gct)
    {
        if (gcTimer != nullptr)
            gcTimer->setContextPointer(this);
    }

    virtual ~BabelSource();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelSource& bs) { return os << bs.str(); }

    const netPrefix<L3Address>& getPrefix() const { return prefix; }
    void setPrefix(const netPrefix<L3Address>& p) { prefix = p; }

    const rid& getOriginator() const { return originator; }
    void setOriginator(const rid& o) { originator = o; }

    routeDistance& getFDistance() { return feasibleDistance; }
    const routeDistance& getFDistance() const { return feasibleDistance; }
    void setFDistance(const routeDistance& fd) { feasibleDistance = fd; }

    BabelTimer *getGCTimer() const { return gcTimer; }
    void setGCTimer(BabelTimer *gct) { gcTimer = gct; }

    void resetGCTimer();
    void resetGCTimer(double delay);
    void deleteGCTimer();
};

class INET_API BabelSourceTable
{
  protected:
    std::vector<BabelSource *> sources;

  public:
    virtual ~BabelSourceTable();

    std::vector<BabelSource *>& getSources() { return sources; }

    BabelSource *findSource(const netPrefix<L3Address>& p, const rid& orig);
    BabelSource *addSource(BabelSource *source);
    void removeSource(BabelSource *source);
    void removeSources();
};

} // namespace babel
} // namespace inet

#endif
