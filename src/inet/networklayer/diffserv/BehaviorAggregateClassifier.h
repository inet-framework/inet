//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_BEHAVIORAGGREGATECLASSIFIER_H
#define __INET_BEHAVIORAGGREGATECLASSIFIER_H

#include "inet/common/INETDefs.h"
#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/Packet.h"

namespace inet {

/**
 * Behavior Aggregate Classifier.
 */
class INET_API BehaviorAggregateClassifier : public queueing::PacketClassifierBase
{
  protected:
    class INET_API PacketDissectorCallback : public PacketDissector::ICallback
    {
      protected:
        bool matches_ = false;
        bool dissect = true;

      public:
        int dscp = -1;

      public:
        PacketDissectorCallback() {}

        bool matches(const Packet *packet);

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return dissect; }
        virtual void startProtocolDataUnit(const Protocol *protocol) override {}
        virtual void endProtocolDataUnit(const Protocol *protocol) override {}
        virtual void markIncorrect() override {}
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

    int numOutGates = 0;
    std::map<int, int> dscpToGateIndexMap;

    int numRcvd = 0;

    static simsignal_t pkClassSignal;

  public:
    BehaviorAggregateClassifier() {}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif // ifndef __INET_BEHAVIORAGGREGATECLASSIFIER_H

