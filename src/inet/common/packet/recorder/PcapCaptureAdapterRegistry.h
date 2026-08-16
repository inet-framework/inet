//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCAPCAPTUREADAPTERREGISTRY_H
#define __INET_PCAPCAPTUREADAPTERREGISTRY_H

#include <map>
#include <tuple>

#include "inet/common/packet/recorder/IPcapCaptureAdapter.h"

namespace inet {

#define Register_Pcap_Capture_Adapter(PROTOCOL, CLASSNAME)    EXECUTE_PRE_NETWORK_SETUP(::inet::PcapCaptureAdapterRegistry::getInstance().registerProtocolAdapter(PROTOCOL, new CLASSNAME()));
#define Register_Pcap_Capture_Protocol_Resolver(OUTER_PROTOCOL, CAPTURE_PROTOCOL)    EXECUTE_PRE_NETWORK_SETUP(::inet::PcapCaptureAdapterRegistry::getInstance().registerProtocolResolver(OUTER_PROTOCOL, CAPTURE_PROTOCOL));
#define Register_Pcap_Capture_Observation_Adapter(KEY, CLASSNAME)    EXECUTE_PRE_NETWORK_SETUP(::inet::PcapCaptureAdapterRegistry::getInstance().registerObservationAdapter(KEY, new CLASSNAME()));

class INET_API PcapCaptureAdapterRegistry
{
  protected:
    std::map<const Protocol *, const IPcapCaptureAdapter *> protocolAdapters;
    std::map<const Protocol *, const Protocol *> protocolResolvers;
    std::map<std::string, const IPcapCaptureObservationAdapter *> observationAdapters;

  public:
    ~PcapCaptureAdapterRegistry();

    void registerProtocolAdapter(const Protocol *protocol, const IPcapCaptureAdapter *adapter);
    void registerProtocolResolver(const Protocol *outerProtocol, const Protocol *captureProtocol);
    void registerObservationAdapter(const char *key, const IPcapCaptureObservationAdapter *adapter);
    const IPcapCaptureAdapter *findProtocolAdapter(const Protocol *protocol) const;
    std::optional<std::tuple<const Protocol *, b, b>> tryResolveProtocol(const Protocol *outerProtocol, const Packet *packet, b frontOffset, b backOffset) const;
    std::optional<PcapCaptureObservation> tryCreateObservation(const cObject *object, Direction direction) const;

    static PcapCaptureAdapterRegistry& getInstance();
};

} // namespace inet

#endif
