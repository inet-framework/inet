//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLGROUP_H
#define __INET_PROTOCOLGROUP_H

#include "inet/common/Protocol.h"

namespace inet {

class INET_API ProtocolGroup
{
  protected:
    const char *name;
    std::vector<const Protocol *> protocols;
    std::map<int, const Protocol *> protocolNumberToProtocol;
    std::map<const Protocol *, int> protocolToProtocolNumber;

  public:
    ProtocolGroup(const char *name, std::map<int, const Protocol *> protocolNumberToProtocol);

    const char *getName() const { return name; }
    int getNumElements() const { return protocols.size(); }
    const Protocol *getElement(int index) const { return protocols.at(index); }

    const Protocol *findProtocol(int protocolNumber) const;
    const Protocol *getProtocol(int protocolNumber) const;
    int findProtocolNumber(const Protocol *protocol) const;
    int getProtocolNumber(const Protocol *protocol) const;
    void addProtocol(int protocolId, const Protocol *protocol);

    std::string str() const { return name; }

  public:
    // in alphanumeric order
    static ProtocolGroup ethertype;
    static ProtocolGroup ieee8022protocol;
    static ProtocolGroup ipprotocol;
    static ProtocolGroup pppprotocol;
    static ProtocolGroup snapOui;
    static ProtocolGroup tcpprotocol;
    static ProtocolGroup udpprotocol;
};

inline std::ostream& operator<<(std::ostream& o, const ProtocolGroup& t) { o << t.str(); return o; }

} // namespace inet

#endif

