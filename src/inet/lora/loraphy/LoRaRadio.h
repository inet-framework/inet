//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef LORA_LORARADIO_H_
#define LORA_LORARADIO_H_

#include "inet/physicallayer/base/packetlevel/PhysicalLayerBase.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/physicallayer/contract/packetlevel/IAntenna.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/base/packetlevel/FlatRadioBase.h"

namespace inet {

namespace lora {

using namespace physicallayer;

class INET_API LoRaRadio : public FlatRadioBase //: public PhysicalLayerBase, public virtual IRadio
{
  public:
    static simsignal_t minSNIRSignal;
    static simsignal_t packetErrorRateSignal;
    static simsignal_t bitErrorRateSignal;
    static simsignal_t symbolErrorRateSignal;
    static simsignal_t droppedPacket;

  protected:
    /**
     * An identifier which is globally unique for the whole lifetime of the
     * simulation among all radios.
     */
    const int id = nextId++;

    /** @name Parameters that determine the behavior of the radio. */
    //@{
    /**
     * The radio antenna model is never nullptr.
     */
    const IAntenna *antenna = nullptr;
    /**
     * The transmitter model is never nullptr.
     */
    const ITransmitter *transmitter = nullptr;
    /**
     * The receiver model is never nullptr.
     */
    const IReceiver *receiver = nullptr;
    /**
     * The radio medium model is never nullptr.
     */
    IRadioMedium *medium = nullptr;
    /**
     * The module id of the medim model.
     */
    int mediumModuleId = -1;
    /**
     * Simulation time required to switch from one radio mode to another.
     */
    simtime_t switchingTimes[RADIO_MODE_SWITCHING][RADIO_MODE_SWITCHING];
    /**
     * Determines whether the transmission of the preamble, header and data part
     * are simulated separately or not.
     */
    bool separateTransmissionParts = false;
    /**
     * Determines whether the reception of the preamble, header and data part
     * are simulated separately or not.
     */
    bool separateReceptionParts = false;
    /**
     * Displays a circle around the host submodule representing the communication range.
     */
    bool displayCommunicationRange = false;
    /**
     * Displays a circle around the host submodule representing the interference range.
     */
    bool displayInterferenceRange = false;
    //@}

    /** Gates */
    //@{
    cGate *upperLayerOut = nullptr;
    cGate *upperLayerIn = nullptr;
    cGate *radioIn = nullptr;
    //@}

    /** State */
    //@{
    /**
     * The current radio mode.
     */
    RadioMode radioMode = RADIO_MODE_OFF;
    /**
     * The radio is switching to this radio radio mode if a switch is in
     * progress, otherwise this is the same as the current radio mode.
     */
    RadioMode nextRadioMode = RADIO_MODE_OFF;
    /**
     * The radio is switching from this radio mode to another if a switch is
     * in progress, otherwise this is the same as the current radio mode.
     */
    RadioMode previousRadioMode = RADIO_MODE_OFF;
    /**
     * The current reception state.
     */
    ReceptionState receptionState = RECEPTION_STATE_UNDEFINED;
    /**
     * The current transmission state.
     */
    TransmissionState transmissionState = TRANSMISSION_STATE_UNDEFINED;
    /**
     * The current received signal part.
     */
    IRadioSignal::SignalPart receivedSignalPart = IRadioSignal::SIGNAL_PART_NONE;
    /**
     * The current transmitted signal part.
     */
    IRadioSignal::SignalPart transmittedSignalPart = IRadioSignal::SIGNAL_PART_NONE;
    //@}

    /** @name Timer */
    //@{
    /**
     * The timer that is scheduled to the end of the current transmission.
     * If this timer is not scheduled then no transmission is in progress.
     */
    cMessage *transmissionTimer = nullptr;
    /**
     * The timer that is scheduled to the end of the current reception.
     * If this timer is nullptr then no attempted reception is in progress but
     * there still may be incoming receptions which are not attempted.
     */
    cMessage *receptionTimer = nullptr;
    /**
     * The timer that is scheduled to the end of the radio mode switch.
     */
    cMessage *switchTimer = nullptr;
    //@}
    double currentTxPower;

  private:
    void parseRadioModeSwitchingTimes();
    void startRadioModeSwitch(RadioMode newRadioMode, simtime_t switchingTime);
    void completeRadioModeSwitch(RadioMode newRadioMode);

  protected:
    virtual void initialize(int stage) override;

    virtual void handleMessageWhenDown(cMessage *message) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message) override;
    virtual void handleSwitchTimer(cMessage *message) override ;
    virtual void handleTransmissionTimer(cMessage *message) override;
    virtual void handleReceptionTimer(cMessage *message) override;
    virtual void handleUpperCommand(cMessage *command) override;
    virtual void handleLowerCommand(cMessage *command) override;
    virtual void handleUpperPacket(Packet *packet) override;
    //virtual void handleLowerPacket(RadioFrame *packet) override;
    virtual void handleSignal(Signal *signal) override;
    //virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    //virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    //virtual void handleNodeCrash() override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;


    virtual void startTransmission(Packet *macFrame, IRadioSignal::SignalPart part) override;
    virtual void continueTransmission() override;
    virtual void endTransmission() override;
    virtual void abortTransmission() override;

    virtual Signal *createSignal(Packet *packet) const override;

    virtual void startReception(cMessage *timer, IRadioSignal::SignalPart part) override;
    virtual void continueReception(cMessage *timer) override;
    virtual void endReception(cMessage *timer) override;
    virtual void abortReception(cMessage *timer) override;
    virtual void captureReception(cMessage *timer) override;

    virtual void sendUp(Packet *macFrame) override;
    virtual cMessage *createReceptionTimer(Signal *radioFrame) const override;
    virtual bool isReceptionTimer(const cMessage *message) const override;

    virtual bool isReceiverMode(IRadio::RadioMode radioMode) const override;
    virtual bool isTransmitterMode(IRadio::RadioMode radioMode) const override;
    virtual bool isListeningPossible() const override;

    virtual void updateTransceiverState() override;
    virtual void updateTransceiverPart() override;

  public:
    LoRaRadio() { }
    virtual ~LoRaRadio();
    bool iAmGateway;
    double getCurrentTxPower();
    void setCurrentTxPower(double txPower);

    std::list<cMessage *>concurrentReceptions;

    virtual int getId() const override { return id; }

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IAntenna *getAntenna() const override { return antenna; }
    virtual const ITransmitter *getTransmitter() const override { return transmitter; }
    virtual const IReceiver *getReceiver() const override { return receiver; }
    virtual const IRadioMedium *getMedium() const override { return medium; }

    virtual const cGate *getRadioGate() const override { return radioIn; }

    virtual RadioMode getRadioMode() const override { return radioMode; }
    virtual void setRadioMode(RadioMode newRadioMode) override;

    virtual ReceptionState getReceptionState() const override { return receptionState; }
    virtual TransmissionState getTransmissionState() const override { return transmissionState; }

    virtual const ITransmission *getTransmissionInProgress() const override;
    virtual const ITransmission *getReceptionInProgress() const override;

    virtual IRadioSignal::SignalPart getTransmittedSignalPart() const override;
    virtual IRadioSignal::SignalPart getReceivedSignalPart() const override;

};

} // namespace physicallayer

} // namespace inet

#endif /* LORA_LORARADIO_H_ */
