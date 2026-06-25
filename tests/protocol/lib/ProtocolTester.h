//
// Protocol Test Framework for INET -- Phase 0: observer / event normaliser.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PROTOCOLTESTER_H
#define __INET_PROTOCOLTEST_PROTOCOLTESTER_H

#include <map>

#include "inet/common/SimpleModule.h"
#include "PacketEvent.h"

namespace inet {
namespace protocoltest {

//
// Phase 0 of the protocol test framework. Subscribes (PcapRecorder-style) to the
// standard INET packet signals across the whole network, normalises each emission
// into a PacketEvent, and -- in "logEvents" mode -- prints a one-line trace.
//
// Later phases turn this into the matching-engine + injector runtime described in
// plan/pending/protocol-test-framework.md.
//
class INET_API ProtocolTester : public SimpleModule, protected cListener
{
  protected:
    std::map<simsignal_t, EventKind> signalKinds; // signal -> normalised kind
    bool logEvents = true;
    cModule *subscriptionModule = nullptr;        // where we attached the listener
    int numObserved = 0;

  protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // Build a normalised event from a raw signal emission.
    PacketEvent normalize(cComponent *source, EventKind kind, const Packet *packet);
    // Phase 0 sink: print a parseable one-line trace.
    void logEvent(const PacketEvent& event);

    static Layer inferLayer(const cComponent *source);

  public:
    virtual ~ProtocolTester();
};

} // namespace protocoltest
} // namespace inet

#endif
