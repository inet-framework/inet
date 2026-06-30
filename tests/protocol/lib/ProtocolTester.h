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
    std::map<simsignal_t, std::string> stateSignalNames; // subscribed scalar signal -> name (state channel)
    bool logEvents = true;
    bool traceState = false;                      // dump every observed scalar (state) emission
    cModule *subscriptionModule = nullptr;        // where we attached the listener
    int numObserved = 0;

    // matching engine state
    bool matchingMode = false;
    std::optional<ProtocolTest> program;
    size_t currentStep = 0;
    CaptureStore captureStore;                     // values bound by capture(...) as steps match
    std::vector<char> groupMatched;                // per-pattern matched flags for an Unordered step
    int groupRemaining = 0;                        // unmatched patterns left in the Unordered group
    int repeatRemaining = 0;                       // occurrences left for an ExactlyTimes step
    int cardCount = 0;                             // matches seen so far for a Count step
    int deliveryStage = 0;                         // 0 = awaiting send, 1 = awaiting matching receive
    long deliveryTreeId = -1;                      // treeId of the sent packet to correlate
    simtime_t anchorTime = 0;                      // start time of the current step's window
    cMessage *deadlineMsg = nullptr;               // fires when the current expect step misses its deadline
    cMessage *injectMsg = nullptr;                 // fires when the current inject step is due
    cMessage *endMsg = nullptr;                    // ends the simulation once a verdict is reached
    bool decided = false;
    bool verdictPass = false;
    bool finishing = false;                        // true once finish() runs (no scheduling allowed)

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

    // observation (packet channel)
    PacketEvent normalize(cComponent *source, EventKind kind, const Packet *packet);
    void logEvent(const PacketEvent& event);
    static Layer inferLayer(const cComponent *source);

    // observation (state channel: scalar signals)
    void subscribeStateSignals();  // subscribe to the program's + the stateSignals param's signal names

    // matching engine
    void installInterceptions();   // push the program's intercept(...) rules onto their taps
    void enterStep();              // begin the current step (arm/schedule per step kind)
    void processMatch(const PacketEvent& event);
    void advance(simtime_t at);    // cancel deadline, set anchor, move to the next step
    void performInjection(const Injection& injection);
    bool patternMatches(const EventPattern& pattern, const PacketEvent& event) const; // selector + earliest gate
    void runCaptures(const EventPattern& pattern, const PacketEvent& event);
    void armDeadline(simtime_t window);
    void cancelDeadline();
    void decide(bool pass, const std::string& reason);

  public:
    virtual ~ProtocolTester();
};

} // namespace protocoltest
} // namespace inet

#endif
