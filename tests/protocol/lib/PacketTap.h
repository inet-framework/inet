//
// Protocol Test Framework for INET -- Phase 6: inline MITM packet tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PACKETTAP_H
#define __INET_PROTOCOLTEST_PACKETTAP_H

#include <functional>
#include <string>

#include "inet/common/SimpleModule.h"
#include <deque>
#include <memory>

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace protocoltest {

//
// Inline man-in-the-middle tap (see PacketTap.ned). Forwards frames between its two
// inout gates `a` and `b`; frames selected by the match expression + occurrence get the
// configured action (drop / delay / pass) applied. Pure observation/fault-injection on a
// gate path, with no INET source changes.
//
// Because the link sides are Ethernet (datarate) channels, the tap is a store-and-forward
// relay: each direction has a queue, and the next frame is transmitted only once the
// outgoing channel's previous transmission has finished.
//
// It can be configured two ways: from its NED/ini parameters (self-contained), or
// programmatically via configure() -- which the ProtocolTester uses to install an
// tap(...) rule from the test program (this is how the "mutate" action, whose
// mutator is a C++ lambda, is supplied).
//
class INET_API PacketTap : public SimpleModule
{
  protected:
    // A tap holds a list of rules, not one rule. Four standards passes worked around the
    // single-rule tap by putting two taps in series, and one of them had to write down the
    // arithmetic of which occurrence the second tap sees. A frame is offered to the rules in
    // order and the first that matches applies; a frame that matches none passes through.
    struct Rule {
        std::string matchExpression;   // "" = match every frame
        long minPacketBytes = 0;       // also require the inner frame to be at least this big
        std::string action;            // "drop" | "delay" | "mutate" | "pass"
        int occurrence = 0;            // act on the Nth selected frame (1-based); 0 = every
        int fromOccurrence = 0;        // act on the Nth and every one after it; 0 = unused
        simtime_t delayTime = 0;
        std::function<void(Packet *)> mutator; // for action == "mutate"

        // By pointer, and never by value: a PacketFilter does not survive a copy, and a
        // Rule is copied when it goes into the container. The pointer is made in place.
        std::shared_ptr<PacketFilter> filter;
        bool hasFilter = false;
        long numSelected = 0;          // frames this rule has selected, for its occurrence
    };
    std::deque<Rule> rules;
    bool programmaticallyConfigured = false; // configure() called -> ignore params
    long numSelected = 0;          // selected (matching) frames seen so far
    long numForwarded = 0;
    long numDropped = 0;
    long numMutated = 0;

    // store-and-forward, one queue + transmit timer per outgoing side
    cPacketQueue queueA, queueB;   // frames waiting for a$o / b$o
    cMessage *pumpA = nullptr;     // fires when a$o's channel is free again
    cMessage *pumpB = nullptr;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // The output gate on the side opposite the frame's arrival gate.
    cGate *forwardGate(const cGate *arrivalGate);
    // The first rule that matches the frame and is at its targeted occurrence, else null.
    Rule *selectRule(Packet *packet);
    // Enqueue toward the given output side and (re)start draining it.
    void enqueueForward(cPacket *packet, bool towardB);
    // Transmit the head of a side's queue if its channel is free, else arm the timer.
    void pump(const char *outGateName, cPacketQueue& queue, cMessage *timer);
    // Compile one rule's match expression into its own filter. A PacketFilter is set
    // once; setting it again on the same object is what a shared compile step would do.
    void compileRule(Rule& rule);

  public:
    virtual ~PacketTap();

    // Install an interception rule programmatically (used by the ProtocolTester to apply a
    // test program's tap(...) clause). Wins over the NED/ini parameters regardless of
    // module initialization order.
    void configure(const std::string& matchExpr, long minBytes, int occ, int fromOcc, const std::string& act,
                   simtime_t delay, std::function<void(Packet *)> mut);
};

} // namespace protocoltest
} // namespace inet

#endif
