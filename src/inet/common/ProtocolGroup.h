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

    std::vector<const Protocol *> dynamicallyAddedProtocols; // the items in protocols[] that need to be deallocated in destructor

  public:
    typedef std::map<int, const Protocol *> Protocols;
    ProtocolGroup(const char *name, const Protocols& protocolNumberToProtocol);
    ~ProtocolGroup();

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
    // in alphabetic order
    static ProtocolGroup *getEthertypeProtocolGroup();
    static ProtocolGroup *getIeee8022ProtocolGroup();
    static ProtocolGroup *getIpProtocolGroup();
    static ProtocolGroup *getPppProtocolGroup();
    static ProtocolGroup *getSnapOuiProtocolGroup();
    static ProtocolGroup *getTcpProtocolGroup();
    static ProtocolGroup *getUdpProtocolGroup();
};

inline std::ostream& operator<<(std::ostream& o, const ProtocolGroup& t) { o << t.str(); return o; }

} // namespace inet

#endif

