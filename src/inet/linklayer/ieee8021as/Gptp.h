//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Inspired by work of Enkhtuvshin Janchivnyambuu, Henning Puttnies, Peter Danielis at University of Rostock, Germany
//

#ifndef __INET_GPTP_H
#define __INET_GPTP_H

#include "inet/clock/common/ClockTimeScale.h"
#include "inet/clock/contract/ClockTime.h"
#include "inet/clock/contract/IClockServo.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API Gptp : public ClockUserModuleBase, public cListener
{
  protected:
    static const MacAddress GPTP_MULTICAST_ADDRESS;

    struct PdelayMeasurementRequesterProcess {
        enum class State {
            INITIALIZED,
            PDELAY_REQ_TRANSMISSION_STARTED,
            PDELAY_REQ_TRANSMISSION_ENDED,
            PDELAY_RESP_RECEPTION_STARTED,
            PDELAY_RESP_RECEPTION_ENDED,
            PDELAY_RESP_FOLLOW_UP_RECEPTION_STARTED,
            PDELAY_RESP_FOLLOW_UP_RECEPTION_ENDED,
            COMPLETED,
            FAILED,
        };

        State state = State::INITIALIZED;

        // measured by reading the unsynchronized local clock on notification from the physical layer
        clocktime_t pdelayReqTransmissionStartUnsychronized = -1;
        clocktime_t pdelayRespReceptionStartUnsychronized = -1;

        // messages sent or received by Gptp
        GptpPdelayReq *pdelayReq = nullptr;
        GptpPdelayResp *pdelayResp = nullptr;
        GptpPdelayRespFollowUp *pdelayRespFollowUp = nullptr;

        ~PdelayMeasurementRequesterProcess() {
            delete pdelayReq;
            delete pdelayResp;
            delete pdelayRespFollowUp;
        }
    };

    struct PdelayMeasurementResponderProcess {
        enum class State {
            INITIALIZED,
            PDELAY_REQ_RECEPTION_STARTED,
            PDELAY_REQ_RECEPTION_ENDED,
            PDELAY_RESP_TRANSMISSION_STARTED,
            PDELAY_RESP_TRANSMISSION_ENDED,
            PDELAY_RESP_FOLLOW_UP_TRANSMISSION_STARTED,
            PDELAY_RESP_FOLLOW_UP_TRANSMISSION_ENDED,
            COMPLETED,
            FAILED,
        };

        State state = State::INITIALIZED;

        // measured by reading the unsynchronized local clock on notification from the physical layer
        clocktime_t pdelayReqReceptionStartUnsychronized = -1;
        clocktime_t pdelayRespTransmissionStartUnsychronized = -1;

        // messages sent or received by Gptp
        GptpPdelayReq *pdelayReq = nullptr;
        GptpPdelayResp *pdelayResp = nullptr;
        GptpPdelayRespFollowUp *pdelayRespFollowUp = nullptr;

        ~PdelayMeasurementResponderProcess() {
            delete pdelayReq;
            delete pdelayResp;
            delete pdelayRespFollowUp;
        }
    };

    struct SyncSenderProcess {
        enum class State {
            INITIALIZED,
            SYNC_TRANSMISSION_STARTED,
            SYNC_TRANSMISSION_ENDED,
            FOLLOW_UP_TRANSMISSION_STARTED,
            FOLLOW_UP_TRANSMISSION_ENDED,
            COMPLETED,
            FAILED,
        };

        State state = State::INITIALIZED;

        // measured by reading the unsynchronized local clock on notification from the physical layer
        clocktime_t syncTransmissionStartUnsychronized = -1;

        // measured by reading the synchronized clock on notification from the physical layer
        clocktime_t syncTransmissionStartSynchronized = -1;

        // messages sent or received by Gptp
        GptpSync *sync = nullptr;
        GptpFollowUp *followUp = nullptr;

        ~SyncSenderProcess() {
            delete sync;
            delete followUp;
        }
    };

    struct SyncReceiverProcess {
        enum class State {
            INITIALIZED,
            SYNC_RECEPTION_STARTED,
            SYNC_RECEPTION_ENDED,
            FOLLOW_UP_RECEPTION_STARTED,
            FOLLOW_UP_RECEPTION_ENDED,
            COMPLETED,
            FAILED,
        };

        State state = State::INITIALIZED;

        // measured by reading the unsynchronized local clock on notification from the physical layer
        clocktime_t syncReceptionStartUnsychronized = -1;

        // measured by reading the synchronized clock on notification from the physical layer
        clocktime_t syncReceptionStartSynchronized = -1;

        // messages sent or received by Gptp
        GptpSync *sync = nullptr;
        GptpFollowUp *followUp = nullptr;

        ~SyncReceiverProcess() {
            delete sync;
            delete followUp;
        }
    };

  protected:
    // module parameters
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IClockServo> clockServo;
    IClock *localClock = nullptr;
    GptpNodeType gptpNodeType;
    int domainNumber = -1;
    int slavePortId = -1;
    std::set<int> masterPortIds;
    clocktime_t syncInterval = -1;
    clocktime_t pdelayInterval = -1;
    double pdelaySmoothingFactor = NaN;
    uint64_t clockIdentity = 0;

    // ongoing pdelay measurement processes
    std::map<int, PdelayMeasurementRequesterProcess> pdelayMeasurementRequesterProcesses;
    std::map<int, PdelayMeasurementResponderProcess> pdelayMeasurementResponderProcesses;

    // ongoing time synchronization processes
    std::map<int, SyncSenderProcess> syncSenderProcesses;
    SyncReceiverProcess syncReceiverProcess;

    // state variables
    uint16_t nextSequenceId = 0;

    // grand master rate ratio state variables
    ClockTimeScale gmRateRatio;
    clocktime_t previousSourceTime = -1;
    clocktime_t previousLocalTime = -1;

    // neighbor rate ratio state variables
    ClockTimeScale neighborRateRatio;
    clocktime_t previousCorrectedResponderEventTimestamp = -1;
    clocktime_t previousPdelayRespEventIngressTimestamp = -1;

    // pdelay state variables
    clocktime_t pdelay = -1;

    // timers
    ClockEvent *syncTimer = nullptr;
    ClockEvent *pdelayTimer = nullptr;

    // statistics
    static simsignal_t gmRateRatioChangedSignal;
    static simsignal_t neighborRateRatioChangedSignal;
    static simsignal_t pdelayChangedSignal;

  public:
    virtual ~Gptp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

  protected:
    virtual void startSyncProcesses();
    virtual void startSyncProcess(int portId);
    virtual void startPdelayMeasurementProcess();

    virtual void synchronize();

    virtual void sendSync(int portId);
    virtual void sendFollowUp(int portId);
    virtual void sendPdelayReq();
    virtual void sendPdelayResp(int portId);
    virtual void sendPdelayRespFollowUp(int portId, const GptpPdelayResp *pdelayResp);

    virtual void forwardSync(const GptpSync *receivedSync);
    virtual void forwardSync(const GptpSync *receivedSync, int portId);

    virtual void processSync(Packet *packet, const GptpSync *sync);
    virtual void processFollowUp(Packet *packet, const GptpFollowUp *followUp);
    virtual void processPdelayReq(Packet *packet, const GptpPdelayReq *pdelayReq);
    virtual void processPdelayResp(Packet *packet, const GptpPdelayResp *pdelayResp);
    virtual void processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp *pdelayRespFollowUp);

    virtual void computeGmRateRatio(); // IEEE 802.1AS-2020 Section 10.2.11.2.1
    virtual void computeNeighborRateRatio(); // IEEE 802.1AS-2020 Section 11.2.19.3.3
    virtual void computePropTime(); // IEEE 802.1AS-2020 Section 11.2.19.3.4

    virtual void sendPacketToNic(Packet *packet, int portId);

    virtual const GptpBase *extractGptpMessage(Packet *packet);

    virtual bool isAllSyncSenderProcessesCompleted() const;
};

}

#endif

