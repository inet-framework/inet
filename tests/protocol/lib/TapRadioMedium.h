//
// Protocol Test Framework for INET -- a radio medium that is a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_TAPRADIOMEDIUM_H
#define __INET_PROTOCOLTEST_TAPRADIOMEDIUM_H

#include <map>
#include <set>
#include <string>

#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"

#include "ITap.h"
#include "InterceptionRules.h"

namespace inet {
namespace protocoltest {

//
// A radio medium that applies the interception rules of a test program, as a PacketTap
// does on a wired link. Every radio medium NED type uses the C++ class RadioMedium, so a
// NED type that extends one of them with @class(TapRadioMedium) makes it a tap.
//
// The medium decides once for each pair of a frame and a receiver radio, when the frame
// starts (addTransmission), before it sends a signal or computes energy. The rules see the
// frame that the transmitter sends, from the node of the transmitter to the node of the
// receiver. A rule counts frames, not receptions. The decision then acts at each place
// where the frame reaches that receiver:
//
//   drop      -- the frame does not exist for the receiver: no signal, no energy
//   corrupt   -- the receiver receives the frame, and the data part fails to decode
//   mutate    -- the receiver gets the frame changed by the mutator
//   duplicate -- the receiver gets the frame unchanged; the medium also transmits copies
//                of it from the same transmitter, each spacing after the one before. Each
//                receiver in range gets a copy, unless the rule names to(node)
//   pass      -- the receiver gets the frame unchanged
//
// A radio signal cannot arrive late, so the medium refuses delay(). It also refuses a
// spacing of 0, because the medium refuses two transmissions that start at the same time.
//
class INET_API TapRadioMedium : public physicallayer::RadioMedium, public ITap
{
  protected:
    enum class Effect { NONE, DROP, CORRUPT, MUTATE };
    struct Decision {
        Effect effect = Effect::NONE;
        const InterceptionRules::Rule *rule = nullptr;
    };
    // A copy that waits for its start time.
    struct PendingCopy {
        const physicallayer::IRadio *transmitter = nullptr;
        Packet *packet = nullptr;
        const InterceptionRules::Rule *rule = nullptr;
    };

    InterceptionRules rules;
    bool rulesChecked = false;          // the from/to nodes of the rules are checked on the first frame
    // The decision for each pair of a transmission id and a receiver radio id. A pair
    // without an entry is NONE.
    std::map<std::pair<int, int>, Decision> decisions;
    std::map<cMessage *, PendingCopy> pendingCopies;
    const InterceptionRules::Rule *copyInTransmission = nullptr; // the rule whose copy transmitPacket() sends now
    long numDropped = 0;
    long numCorrupted = 0;
    long numMutated = 0;
    long numCopied = 0;

  protected:
    virtual void handleMessage(cMessage *message) override;
    virtual void finish() override;

    virtual void addTransmission(const physicallayer::IRadio *transmitter, const physicallayer::ITransmission *transmission) override;
    virtual bool isPotentialReceiver(const physicallayer::IRadio *receiver, const physicallayer::ITransmission *transmission) const override;
    virtual bool isInterferingTransmission(const physicallayer::ITransmission *transmission, const physicallayer::IListening *listening) const override;
    virtual bool isInterferingTransmission(const physicallayer::ITransmission *transmission, const physicallayer::IReception *reception) const override;
    virtual const physicallayer::IReceptionDecision *computeReceptionDecision(const physicallayer::IRadio *receiver, const physicallayer::IListening *listening, const physicallayer::ITransmission *transmission, physicallayer::IRadioSignal::SignalPart part) const override;

    // Applies the rules to each receiver of a frame that starts now.
    void decide(const physicallayer::IRadio *transmitter, const physicallayer::ITransmission *transmission);
    Decision getDecision(const physicallayer::IRadio *receiver, const physicallayer::ITransmission *transmission) const;
    bool isDropped(const physicallayer::IRadio *receiver, const physicallayer::ITransmission *transmission) const;
    // The name of the network node that holds a radio.
    static std::string getNodeName(const physicallayer::IRadio *radio);
    // Throws for a rule whose from or to names a node without a radio on this medium.
    void checkRuleNodes();

  public:
    virtual ~TapRadioMedium();

    virtual Packet *receivePacket(const physicallayer::IRadio *receiver, physicallayer::IWirelessSignal *signal) override;

    virtual int addInterception(const Interception& interception, const CaptureStore *captures) override;
    virtual void startInterception(int handle) override;
};

} // namespace protocoltest
} // namespace inet

#endif
