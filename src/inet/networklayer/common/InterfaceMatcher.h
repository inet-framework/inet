//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACEMATCHER_H
#define __INET_INTERFACEMATCHER_H

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class PatternMatcher;

/**
 * Utility class for configuring interfaces.
 *
 * It is assumed that the configuration is described by an xml document.
 * Each element in the configuration applies parameters to a set of interfaces.
 * These interfaces are selected by a set of selector attributes: @hosts, @names,
 * @towards, and @among. The value of these attributes are space separated name
 * patterns. ...
 *
 * - @hosts: specifies the names of the host module of the interface.
 *           Qualified names are accepted, but not required.
 * - @names: specifies the names of the interfaces
 * - @towards: specifies the names of the hosts which the interface is connected to
 * - @among: specifies the names of hosts, and select the interfaces of those
 *           hosts that are connected to that hosts
 *           among="X Y Z" is the same as hosts="X Y Z" towards="X Y Z"
 *
 * If there are more selector attributes in an element, all of them are required to match.
 */
class INET_API InterfaceMatcher
{
  private:
    class Matcher {
      private:
        bool matchesany;
        std::vector<inet::PatternMatcher *> matchers; // TODO replace with a MatchExpression once it becomes available in OMNeT++

      public:
        Matcher(const char *pattern);
        ~Matcher();
        bool matches(const char *s) const;
        bool matchesAny() const { return matchesany; }
    };
    struct Selector {
        Matcher hostMatcher;
        Matcher nameMatcher;
        Matcher towardsMatcher;
        const InterfaceMatcher *parent;
        Selector(const char *hostPattern, const char *namePattern, const char *towardsPattern, const InterfaceMatcher *parent);
        bool matches(const NetworkInterface *ie);
    };

  private:
    std::vector<Selector *> selectors;

  public:
    InterfaceMatcher(const cXMLElementList& selectors);
    ~InterfaceMatcher();
    int findMatchingSelector(const NetworkInterface *ie);

  private:
    bool linkContainsMatchingHost(const NetworkInterface *ie, const Matcher& hostMatcher) const;
    void collectNeighbors(cGate *outGate, std::vector<cModule *>& hostNodes, std::vector<cModule *>& deviceNodes, cModule *exludedNode) const;
};

} // namespace inet

#endif

