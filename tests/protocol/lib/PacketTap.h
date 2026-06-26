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
// intercept(...) rule from the test program (this is how the "mutate" action, whose
// mutator is a C++ lambda, is supplied).
//
class INET_API PacketTap : public SimpleModule
{
  protected:
    std::string matchExpression;   // "" = match every frame
    long minPacketBytes = 0;       // also require the inner frame to be at least this big
    std::string action;            // "drop" | "delay" | "mutate" | "pass"
    int occurrence = 0;            // act on the Nth selected frame (1-based); 0 = every
    simtime_t delayTime = 0;
    std::function<void(Packet *)> mutator; // applied to the inner frame for action == "mutate"

    PacketFilter filter;           // compiled from matchExpression
    bool hasFilter = false;
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
    // True if the frame matches the expression and is the targeted occurrence.
    bool isSelected(Packet *packet);
    // Enqueue toward the given output side and (re)start draining it.
    void enqueueForward(cPacket *packet, bool towardB);
    // Transmit the head of a side's queue if its channel is free, else arm the timer.
    void pump(const char *outGateName, cPacketQueue& queue, cMessage *timer);
    // Compile the match expression into the filter.
    void compileFilter();

  public:
    virtual ~PacketTap();

    // Install an interception rule programmatically (used by the ProtocolTester to apply a
    // test program's intercept(...) clause). Wins over the NED/ini parameters regardless of
    // module initialization order.
    void configure(const std::string& matchExpr, long minBytes, int occ, const std::string& act,
                   simtime_t delay, std::function<void(Packet *)> mut);
};

} // namespace protocoltest
} // namespace inet

#endif
