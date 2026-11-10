// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211MESHMAC_H
#define __INET_IEEE80211MESHMAC_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mgmt/IMeshManagement.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshDataMac.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211MeshMac : public Ieee80211Mac, public IMeshDataMac
{
  protected:
    ModuleRefByPar<IMeshManagement> meshManagement;
    virtual void initialize(int stage) override;
    virtual void handleMgmtPacket(Packet *packet) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerMessage(cMessage *message) override;
    void unsupportedData(Packet *packet);

  public:
    virtual void sendUpFrame(Packet *packet) override;
    virtual void sendMeshData(Packet *packet) override;
    virtual void deliverMeshMsdu(Packet *packet, MacAddress source, MacAddress destination, int tid) override;
    virtual void prepareFrameTransmission(Packet *packet, simtime_t ifs,
            const physicallayer::IIeee80211Mode *mode) override;
    virtual void frameTransmissionComplete(const Packet *packet) override;
    void finalizeDataFrame(Packet *packet) override;
    void discardPendingMeshSources() override;
    virtual bool isFrameTransmissionEligible(const Packet *packet, simtime_t plannedStart) const override;
    virtual bool isDataReceptionSupported(const Ptr<const Ieee80211DataHeader>& header) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
