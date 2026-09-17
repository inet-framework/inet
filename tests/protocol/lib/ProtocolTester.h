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

    // A step started by meanwhile(...) keeps running while the ordered steps go on. Each
    // guard carries its own window and its own count, and an event is offered to every one
    // of them before it reaches the ordered step.
    struct Guard {
        size_t stepIndex = 0;
        simtime_t startTime = 0;
        int count = 0;
        cMessage *timer = nullptr;
    };
    std::vector<Guard> guards;
    cMessage *injectMsg = nullptr;                 // fires when the current inject step is due
    cMessage *endMsg = nullptr;                    // ends the simulation once a verdict is reached
    bool decided = false;
    bool verdictPass = false;
    bool finishing = false;                        // true once finish() runs (no scheduling allowed)

  protected:
    // The module's finish() would hide the listener's finish(cComponent *, simsignal_t).
    using cListener::finish;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, uintval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, const SimTime& value, cObject *details) override;
    // One normaliser behind the four overloads above.
    virtual void receiveStateValue(cComponent *source, simsignal_t signalID, double value);

    // observation (packet channel)
    PacketEvent normalize(cComponent *source, EventKind kind, const Packet *packet);
    void logEvent(const PacketEvent& event);
    static Layer inferLayer(const cComponent *source);

    // observation (state channel: scalar signals)
    void subscribeStateSignals();  // subscribe to the program's + the stateSignals param's signal names

    // matching engine
    void installInterceptions();   // push the program's tap(...) rules onto their taps
    void enterStep();              // begin the current step (arm/schedule per step kind)
    void processMatch(const PacketEvent& event);
    void advance(simtime_t at);    // cancel deadline, set anchor, move to the next step
    void performInjection(const Injection& injection);
    bool patternMatches(const EventPattern& pattern, const PacketEvent& event, simtime_t anchor = -1); // selector + earliest gate
    void runCaptures(const EventPattern& pattern, const PacketEvent& event);
    void startGuard(size_t stepIndex);             // begin a meanwhile(...) step
    void offerToGuards(const PacketEvent& event);  // every running guard sees the event
    void resolveGuard(Guard& guard);               // its window closed: judge it
    bool guardsOutstanding() const;                // a meanwhile(...) step is still running
    void armDeadline(simtime_t window);
    void cancelDeadline();
    void decide(bool pass, const std::string& reason);

  public:
    virtual ~ProtocolTester();
};

} // namespace protocoltest
} // namespace inet

#endif
