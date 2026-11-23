// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHSERVICEPERIODSUPPORT_H
#define INET_TEST_MESHSERVICEPERIODSUPPORT_H
#include "mesh/MeshPhySupport.h"
#include "inet/linklayer/ieee80211/mesh/MeshPowerSave.h"
#include "inet/linklayer/ieee80211/mesh/MeshPayloadTransmission.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"

class ServicePeriodDriver : public cSimpleModule, public cListener
{
  protected:
    int delivered = 0, payloadAir = 0, nullAir = 0, ownerClosed = 0, recipientClosed = 0;
    int samples = 0, failures = 0, sleeps = 0;
    int aggregateAir = 0, payloadPpdus = 0;
    int receivedAggregates=0;
    double expectedN=1, expectedEf=0;
    int nChecks=0, efChecks=0;
    int actualCompletedMsdus=0;
    std::string sampleOrder;
    void verifySampleOrder() const {
        if (hasPar("expectedSampleOrder")) ASSERT(sampleOrder==par("expectedSampleOrder").stdstringValue());
    }
    bool verifyMetric() const { return hasPar("verifyMetric") && par("verifyMetric").boolValue(); }
    bool nested() const { return hasPar("nested") && par("nested").boolValue(); }
    Hcf *originator() const { return check_and_cast<Hcf *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")); }
    void checkMetric() const {
        auto metric=check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"));
        ASSERT(metric->getPayloadEstimate(meshMib(1)->address)==expectedN);
        ASSERT(metric->getErrorEstimate(meshMib(1)->address)==expectedEf);
        auto timing=originator()->getMeshLinkTiming(meshMib(1)->address);
        int id=meshMib(0)->getMeshConfiguration().pathSelectionMetricId;
        long double seconds=(timing.overhead.dbl()/(id==1 ? 1 : expectedN)+8192.L/timing.rate.get())/(1-expectedEf);
        uint32_t encoded=uint32_t(std::ceil(seconds*100000000.L/(id==1 ? 1024 : 1)));
        ASSERT(metric->getMetric(meshMib(1)->address)==encoded);
    }
    virtual void observedAggregate(const Ieee80211Transmission *) {}
    virtual void finishAssertions() {}
    virtual bool expectAggregateEosp() const { return true; }
    void checkActualTuple(const Ieee80211TxVector& vector) const {
        bool vht=meshInterface(0)->par("phyProfile").stdstringValue()=="vht";
        ASSERT(vector.ampdu && vector.bandwidth==MHz(20) && vector.mode);
        ASSERT(vector.format==(vht ? Ieee80211TxVector::VHT : Ieee80211TxVector::HT_MIXED));
        ASSERT((vht ? vector.mode->getVhtMcsIndex() : vector.mode->getHtMcsIndex())==7);
        ASSERT(!(vht ? vector.mode->isVhtShortGuardInterval() : vector.mode->isHtShortGuardInterval()));
        ASSERT(vector.mode->getDataMode()->getNumberOfSpatialStreams()==1);
    }
    int payloadCount() const { return hasPar("payloadCount") ? par("payloadCount").intValue() : 1; }
    int expectedMpdus() const { return hasPar("expectedMpdus") ? par("expectedMpdus").intValue() : payloadCount(); }
    bool isPayload(const Packet *packet) const { return std::string(packet->getName()).find("service-payload")==0; }
    void checkMemberFcs(const Packet *packet,b offset,B length) const {
        auto bytes=packet->peekDataAt<BytesChunk>(offset,length)->getBytes();
        ASSERT(bytes.size()>=4);
        size_t end=bytes.size()-4;
        uint32_t received=uint32_t(bytes[end]) | uint32_t(bytes[end+1])<<8 | uint32_t(bytes[end+2])<<16 | uint32_t(bytes[end+3])<<24;
        ASSERT(independentMeshFcs(bytes,0,end)==received);
    }
    int scenario() const { return par("scenario"); }
    MeshPowerMode mode(int i) const {
        return (scenario() & (1<<i)) ? MeshPowerMode::DEEP_SLEEP : MeshPowerMode::LIGHT_SLEEP;
    }
    MeshPowerSave *power(int i) const {
        return check_and_cast<MeshPowerSave *>(meshInterface(i)->getSubmodule("powerSavePolicy"));
    }
    void at(simtime_t time, int kind) { scheduleAt(time,new cMessage("service-period-check",kind)); }
    int from(cComponent *source, const char *part) const {
        for (int i=0;i<2;i++) if (source==meshInterface(i)->getSubmodule(part)) return i;
        return -1;
    }
  protected:
    void initialize() override {
        auto network=getSimulation()->getSystemModule();
        for (auto signal : {transmissionStartedSignal,packetSentToUpperSignal,
                Hcf::meshDataAttemptCompletedSignal,MeshPowerSave::meshServicePeriodOutcomeSignal,
                IRadio::radioModeChangedSignal,Hcf::meshPayloadTransmittedSignal,
                Hcf::blockAckAgreementAddedSignal,Hcf::blockAckAgreementDeletedSignal,receptionEndedSignal}) network->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0);
        at(SimTime(60,SIMTIME_MS),1);
        at(SimTime(80,SIMTIME_MS),2);
        at(SimTime(130,SIMTIME_MS),3);
        at(scenario()==0 ? SimTime(380,SIMTIME_MS) : SimTime(1500,SIMTIME_MS),4);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,long value,cObject *) override {
        if (signal==IRadio::radioModeChangedSignal && from(source,"radio")>=0 && value==IRadio::RADIO_MODE_SLEEP) sleeps++;
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==receptionEndedSignal) {
            if (from(source,"radio")!=1) return;
            auto packet=dynamic_cast<Packet *>(details);
            auto actual=packet ? packet->findTag<Ieee80211ModeInd>() : nullptr;
            if (packet && isPayload(packet) && actual && actual->getTxVector().ampdu) {
                checkActualTuple(actual->getTxVector()); receivedAggregates++;
            }
            return;
        }
        if (verifyMetric() && source==originator() && (signal==Hcf::blockAckAgreementAddedSignal || signal==Hcf::blockAckAgreementDeletedSignal)
                && dynamic_cast<OriginatorBlockAckAgreement *>(object)) {
            expectedN=1; return; // Existing permission lifecycle resets aggregation, not ef.
        }
        if (verifyMetric() && source==originator() && signal==Hcf::meshPayloadTransmittedSignal) {
            auto packet=check_and_cast<Packet *>(object);
            if (!isPayload(packet)) return;
            auto count=check_and_cast<MeshPayloadTransmission *>(details)->msduCount;
            ASSERT(count==actualCompletedMsdus);
            expectedN=expectedN*0.875+count*0.125;
            at(simTime()+SimTime::fromRaw(1),100); return;
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            std::cout << "period outcome station=" << from(source,"powerSavePolicy") << " direction=" << int(fact->direction)
                << " outcome=" << int(fact->outcome) << " t=" << simTime() << std::endl;
            if (fact->outcome==MeshPeriodOutcome::CONFIRMED) {
                if (fact->direction==MeshPeriodDirection::OWNER) ownerClosed++; else recipientClosed++;
            }
            return;
        }
        if (signal==Hcf::meshDataAttemptCompletedSignal) {
            auto packet=check_and_cast<Packet *>(object);
            if (!isPayload(packet)) return;
            auto fact=check_and_cast<MeshDataAttempt *>(details);
            samples++; failures+=!fact->acknowledged;
            sampleOrder+=fact->acknowledged ? '1' : '0';
            if (verifyMetric()) {
                expectedEf=expectedEf*0.875+(fact->acknowledged ? 0 : 0.125);
                at(simTime()+SimTime::fromRaw(1),101);
            }
            return;
        }
        if (signal==packetSentToUpperSignal) {
            auto packet=dynamic_cast<Packet *>(object);
            if (packet && from(source,"mac")==1 && isPayload(packet)) delivered++;
            return;
        }
        if (signal!=transmissionStartedSignal || from(source,"radio")<0) return;
        auto transmission=check_and_cast<const Ieee80211Transmission *>(object);
        auto packet=transmission->getPacket();
        auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        if (transmission->getTxVector().ampdu) {
            checkActualTuple(transmission->getTxVector());
            Ieee80211AmpduLayout layout;
            ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),
                transmission->getTxVector().format==Ieee80211TxVector::VHT,layout));
            aggregateAir++;
            payloadPpdus++;
            actualCompletedMsdus=layout.members.size()*(nested() ? 2 : 1);
            observedAggregate(transmission);
            for (const auto& member : layout.members) {
                auto data=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength()+member.offset);
                ASSERT(data->getType()==ST_DATA_WITH_QOS && data->getEosp()==expectAggregateEosp() && !data->getRspi());
                ASSERT(data->getAMsduPresent()==nested());
                checkMemberFcs(packet,phy->getChunkLength()+member.offset,member.length);
                auto delimiter=packet->peekDataAt<BytesChunk>(phy->getChunkLength()+member.delimiterOffset,B(4))->getBytes();
                ASSERT((delimiter[0]&1)==0); // EOF0 BA exchange, including a one-member retry.
                payloadAir++;
            }
            return;
        }
        b macOffset=phy->getChunkLength();
        if (transmission->getTxVector().singleMpdu) {
            Ieee80211AmpduLayout layout;
            ASSERT(findAmpdu(packet,macOffset,phy->getLengthField(),true,layout,true));
            ASSERT(layout.members.size()==1);
            macOffset+=layout.members.front().offset;
        }
        auto header=packet->peekDataAt<Ieee80211MacHeader>(macOffset);
        if (header->getType()==ST_QOS_NULL || isPayload(packet)) {
            auto data=packet->peekDataAt<Ieee80211MeshDataHeader>(macOffset);
            if (isPayload(packet)) {
                actualCompletedMsdus=nested() ? 2 : 1;
                ASSERT(data->getAMsduPresent()==nested());
                if (!transmission->getTxVector().singleMpdu)
                    checkMemberFcs(packet,macOffset,phy->getLengthField());
                else {
                    Ieee80211AmpduLayout layout;
                    ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),true,layout,true));
                    checkMemberFcs(packet,macOffset,layout.members.front().length);
                }
            }
            std::cout << "period air station=" << from(source,"radio") << " name=" << packet->getName()
                << " rspi=" << data->getRspi() << " eosp=" << data->getEosp() << " t=" << simTime() << std::endl;
            if (header->getType()==ST_QOS_NULL) nullAir++; else { payloadAir++; payloadPpdus++; }
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==100 || kind==101) {
            checkMetric();
            if (kind==100) nChecks++; else efChecks++;
            return;
        }
        if (kind==0) {
            meshManagement(0)->openPeer(meshMib(1)->address);
            meshManagement(1)->openPeer(meshMib(0)->address);
        }
        else if (kind==1) power(0)->requestMode(meshMib(1)->address,mode(0));
        else if (kind==2) power(1)->requestMode(meshMib(0)->address,mode(1));
        else if (kind==3) {
            for (int i=0;i<2;i++) ASSERT(meshMib(i)->getMeshConfiguration().pathSelectionMetricId
                == meshInterface(i)->par("pathSelectionMetricId").intValue());
            for (int i=0;i<2;i++) {
                auto state=meshMib(i)->findMeshPeerPowerState(meshMib(1-i)->address);
                ASSERT(state && state->local==mode(i) && state->remote==mode(1-i));
            }
            // Deep rendezvous can be ten periods away; isolate this local
            // service-period test from HWMP discovery's independent deadline.
            if (scenario()!=0) {
                fixturePath(0,1,1,SimTime(2,SIMTIME_S));
                fixturePrecursor(1,1,0,SimTime(2,SIMTIME_S));
            }
            for (int i=0;i<payloadCount();i++) {
                auto name=payloadCount()==1 ? std::string("service-payload") : std::string("service-payload-")+std::to_string(i);
                check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))
                    ->processSourceMsdu(phyMsdu(name.c_str(),meshMib(1)->address));
            }
        }
        else {
            std::cout << "period final delivered=" << delivered << " payloadAir=" << payloadAir << " nullAir=" << nullAir
                << " ownerClosed=" << ownerClosed << " recipientClosed=" << recipientClosed << " samples=" << samples
                << " failures=" << failures << " sleeps=" << sleeps << std::endl;
            ASSERT(delivered==payloadCount() && payloadAir==expectedMpdus() && samples==expectedMpdus());
            ASSERT(failures==(hasPar("expectedFailures") ? par("expectedFailures").intValue() : 0));
            if (hasPar("expectAggregate") && par("expectAggregate").boolValue()) {
                ASSERT(aggregateAir>=1 && receivedAggregates>=1);
                if (!hasPar("expectedPpdus")) ASSERT(payloadPpdus<payloadCount());
            }
            if (hasPar("expectedPpdus")) ASSERT(payloadPpdus==par("expectedPpdus").intValue());
            if (verifyMetric()) ASSERT(nChecks==payloadPpdus && efChecks==expectedMpdus());
            verifySampleOrder();
            if (!hasPar("requireConfirmed") || par("requireConfirmed").boolValue()) ASSERT(ownerClosed>=1 && recipientClosed>=1);
            ASSERT(sleeps>=2);
            for (int i=0;i<2;i++) ASSERT(power(i)->getServicePeriods().empty());
            finishAssertions();
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
Define_Module(ServicePeriodDriver);

#endif
