//
// Protocol Test Framework for INET -- observer + matching engine.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PROTOCOLTESTER_H
#define __INET_PROTOCOLTEST_PROTOCOLTESTER_H

#include <map>
#include <optional>

#include "inet/common/SimpleModule.h"
#include "PacketEvent.h"
#include "ProtocolTest.h"

namespace inet {
namespace protocoltest {

//
// Runtime of the protocol test framework. Subscribes (PcapRecorder-style) to the
// standard INET packet signals across the whole network and normalises each emission
// into a PacketEvent.
//
//  - logEvents mode (no testName): prints a one-line trace per event (Phase 0).
//  - matching mode (testName set): walks the named ProtocolTest program against the
//    event stream as a sequential, timing-guarded matcher and emits a PASS/FAIL
//    verdict (Phase 1; expect-only).
//
class INET_API ProtocolTester : public SimpleModule, protected cListener
{
  protected:
    std::map<simsignal_t, EventKind> signalKinds; // signal -> normalised kind
    bool logEvents = true;
    cModule *subscriptionModule = nullptr;        // where we attached the listener
    int numObserved = 0;

    // matching engine state
    bool matchingMode = false;
    std::optional<ProtocolTest> program;
    size_t currentStep = 0;
    CaptureStore captureStore;                     // values bound by capture(...) as steps match
    simtime_t anchorTime = 0;                      // start time of the current step's window
    cMessage *deadlineMsg = nullptr;               // fires when the current expect step misses its deadline
    cMessage *injectMsg = nullptr;                 // fires when the current inject step is due
    cMessage *endMsg = nullptr;                    // ends the simulation once a verdict is reached
    bool decided = false;
    bool verdictPass = false;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // observation
    PacketEvent normalize(cComponent *source, EventKind kind, const Packet *packet);
    void logEvent(const PacketEvent& event);
    static Layer inferLayer(const cComponent *source);

    // matching engine
    void enterStep();              // begin the current step (arm an expect, or schedule an inject)
    void processMatch(const PacketEvent& event);
    void performInjection(const Injection& injection);
    void armCurrentDeadline();
    void cancelDeadline();
    void decide(bool pass, const std::string& reason);

  public:
    virtual ~ProtocolTester();
};

} // namespace protocoltest
} // namespace inet

#endif
