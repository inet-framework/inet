//
// Protocol Test Framework for INET -- Phase 6: inline MITM packet tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PACKETTAP_H
#define __INET_PROTOCOLTEST_PACKETTAP_H

#include <map>
#include <string>

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/Packet.h"

#include "ITap.h"
#include "InterceptionRules.h"

namespace inet {
namespace protocoltest {

//
// Inline man-in-the-middle tap (see PacketTap.ned). Forwards frames between its two
// inout gates `a` and `b`; frames selected by its rules get the action of the rule
// applied (drop / delay / mutate / corrupt / duplicate / pass). Pure observation and
// fault injection on a gate path, with no INET source changes.
//
// Because the link sides are Ethernet (datarate) channels, the tap is a store-and-forward
// relay: each direction has a queue, and the next frame is transmitted only once the
// outgoing channel's previous transmission has finished.
//
// It can be configured two ways: from its NED/ini parameters (self-contained), or
// programmatically through ITap -- which the ProtocolTester uses to install the
// tap(...) rules of the test program (this is how the "mutate" action, whose
// mutator is a C++ lambda, is supplied).
//
class INET_API PacketTap : public SimpleModule, public ITap
{
  protected:
    InterceptionRules rules;
    bool programmaticallyConfigured = false; // addInterception() called -> ignore params
    std::string nodeA, nodeB;      // the nodes behind side a and side b, found on first use
    // Frames waiting in a queue whose duplicate rule wants copies of them. The copies are
    // scheduled when the frame starts on the outgoing channel.
    std::map<cPacket *, const InterceptionRules::Rule *> pendingCopies;
    long numForwarded = 0;
    long numDropped = 0;
    long numMutated = 0;
    long numCorrupted = 0;
    long numCopied = 0;

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
    // The name of the node behind a side.
    const std::string& getSideNode(bool sideB);
    // Offer a frame to the rules and apply the action of the rule that selects it.
    void processFrame(cPacket *frame, bool towardB, const InterceptionRules::Rule *copyOf);
    // Schedule the copies that a duplicate rule wants of a frame that starts now.
    void scheduleCopies(cPacket *frame, bool towardB);
    // Enqueue toward the given output side and (re)start draining it.
    void enqueueForward(cPacket *packet, bool towardB);
    // Transmit the head of a side's queue if its channel is free, else arm the timer.
    void pump(const char *outGateName, cPacketQueue& queue, cMessage *timer);

  public:
    virtual ~PacketTap();

    // Wins over the NED/ini parameters regardless of module initialization order: the
    // first rule from a test program discards the rule that the parameters describe.
    virtual int addInterception(const Interception& interception, const CaptureStore *captures) override;
    virtual void startInterception(int handle) override;
};

} // namespace protocoltest
} // namespace inet

#endif
