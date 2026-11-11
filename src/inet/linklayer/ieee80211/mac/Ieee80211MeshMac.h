// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211MESHMAC_H
#define __INET_IEEE80211MESHMAC_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mgmt/IMeshManagement.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshDataMac.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPowerSave.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshRadioControl.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211MeshMac : public Ieee80211Mac, public IMeshDataMac, public IMeshRadioControl
{
  protected:
    ModuleRefByPar<IMeshManagement> meshManagement;
    ModuleRefByPar<IMeshPowerSave> meshPowerSave;
    cMessage *radioReview = nullptr;
    bool powerManaged = false;
    bool awakeIntent = true;
    void reviewRadioState();
    void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
    void receiveSignal(cComponent *source, simsignal_t signal, bool value, cObject *details) override;
    void handleSelfMessage(cMessage *message) override;
    void configureRadioMode(physicallayer::IRadio::RadioMode mode) override;
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void initialize(int stage) override;
    virtual void handleMgmtPacket(Packet *packet) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerMessage(cMessage *message) override;
    void unsupportedData(Packet *packet);

  public:
    ~Ieee80211MeshMac() override;
    virtual void sendUpFrame(Packet *packet) override;
    virtual void sendMeshData(Packet *packet) override;
    virtual void deliverMeshMsdu(Packet *packet, MacAddress source, MacAddress destination, int tid) override;
    virtual void prepareFrameTransmission(Packet *packet, simtime_t ifs,
            const physicallayer::IIeee80211Mode *mode) override;
    virtual void frameTransmissionComplete(const Packet *packet) override;
    void frameTransmissionComplete(const Packet *packet, simtime_t start) override;
    bool defersTransmissionStart() const override;
    bool isTransmissionReady() const override;
    bool isRadioReadyForTransmission(simtime_t start) const override;
    void requestTransmissionReadiness() override { requestRadioReview(); }
    void requestRadioReview() override;
    std::optional<physicallayer::IRadio::RadioModeState> getMeshRadioState() const override;
    std::optional<simtime_t> getMeshRadioSwitchingTime(physicallayer::IRadio::RadioMode from,
        physicallayer::IRadio::RadioMode to) const override;
    bool hasMeshRadioActivity() const override;
    bool isMeshPhyIdle() const override;
    void prepareFrameContent(Packet *packet, const std::set<int64_t>& deliveryUnit) override;
    void finalizeDataFrame(Packet *packet) override;
    void discardPendingMeshSources() override;
    virtual bool isFrameTransmissionEligible(const Packet *packet, simtime_t plannedStart) const override;
    virtual bool isDataReceptionSupported(const Ptr<const Ieee80211DataHeader>& header) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
