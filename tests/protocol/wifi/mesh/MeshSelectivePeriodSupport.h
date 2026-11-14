// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHSELECTIVEPERIODSUPPORT_H
#define INET_TEST_MESHSELECTIVEPERIODSUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"

class SelectiveDeadlineHcf : public EndingResponseHcf
{
  public:
    bool recovering(MacAddress peer) const {
        for (int ac=0;ac<AC_NUMCATEGORIES;ac++)
            if (edca->getEdcaf(static_cast<AccessCategory>(ac))->getRecoveryProcedure()->hasBlockAckRecovery(peer,0)) return true;
        return false;
    }
    simtime_t setConstituentDeadlines() {
        Enter_Method("setConstituentDeadlines");
        auto queue=edca->getEdcaf(AC_BE)->getPendingQueue();
        int changed=0;
        for (int i=0;i<queue->getNumPackets();i++) {
            auto packet=queue->getPacket(i);
            std::string name=packet->getName();
            if (name.find("service-payload-")!=0) continue;
            int index=std::stoi(name.substr(std::string("service-payload-").size()));
            // Each FIFO pair has different original deadlines. The intact
            // A-MSDU must inherit the pair's maximum, and retain it on retry.
            edca->getLifetimeHandler()->forgetFrame(packet->getId());
            edca->getLifetimeHandler()->registerFragments({packet},simTime()+SimTime(index%2 ? 3 : 2,SIMTIME_S));
            changed++;
        }
        ASSERT(changed==6); scheduleDataLifetimeTimer();
        return simTime()+SimTime(3,SIMTIME_S);
    }
};
namespace selectiveDeadlineRegistration { Define_Module(SelectiveDeadlineHcf); }

class SelectiveRecoveryMac : public EndingResponseMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        if (getContainingNode(this)->getIndex()==1 && std::string(packet->getName())=="mesh-period-ending"
                && !getSimulation()->getSystemModule()->getSubmodule("driver")->par("releaseReverse").boolValue()) return false;
        return EndingResponseMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace selectiveRecoveryRegistration { Define_Module(SelectiveRecoveryMac); }

class SelectivePowerObserver : public MeshPowerSave
{
    std::vector<int64_t> originals;
  public:
    const std::vector<int64_t>& selected() const { return originals; }
    bool authorizeTransmissionStart(const std::vector<MeshPowerTxMember>& members,simtime_t start) override {
        Enter_Method("authorizeTransmissionStart");
        bool allowed=MeshPowerSave::authorizeTransmissionStart(members,start);
        if (allowed) {
            originals.clear();
            for (const auto& member : members) originals.push_back(member.id);
        }
        return allowed;
    }
};
namespace selectiveObserverRegistration { Define_Module(SelectivePowerObserver); }

class SelectivePeriodDriver : public ServicePeriodDriver
{
    struct Original {
        int sequence;
        std::vector<uint8_t> body;
        simtime_t deadline;
    };
    std::map<int64_t,Original> originals;
    int64_t missing=-1;
    simtime_t maximumDeadline=-1;
    int identityChecks=0, literalNChecks=0;
    int barAir=0, pendingChecks=0;
    bool lostBa() const { return par("lostBa"); }
    simtime_t deadline(int64_t id) const {
        for (const auto& frame : check_and_cast<IMeshPowerBuffer *>(originator())->getMeshBufferedFrames())
            if (frame.id==id) return frame.deadline;
        throw cRuntimeError("Expected selected live original deadline");
    }
  protected:
    void initialize() override {
        ServicePeriodDriver::initialize();
        at(SimTime(50,SIMTIME_MS),10);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        ServicePeriodDriver::receiveSignal(source,signal,object,details);
        if (!lostBa()) return;
        if (source==originator() && signal==Hcf::meshDataAttemptCompletedSignal && aggregateAir==2
                && !check_and_cast<MeshDataAttempt *>(details)->acknowledged)
            at(simTime()+SimTime::fromRaw(1),20);
        if (source==power(0) && signal==MeshPowerSave::meshServicePeriodOutcomeSignal && aggregateAir==2) {
            auto outcome=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (outcome->direction==MeshPeriodDirection::OWNER) {
                ASSERT(outcome->outcome==MeshPeriodOutcome::CONFIRMED && barAir==1);
                par("releaseReverse").setBoolValue(true);
                check_and_cast<IMeshPowerBuffer *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
            }
        }
        if (source==meshInterface(0)->getSubmodule("radio") && signal==transmissionStartedSignal) {
            auto transmission=check_and_cast<const Ieee80211Transmission *>(object);
            if (transmission->getTxVector().ampdu || transmission->getTxVector().singleMpdu) return;
            auto packet=transmission->getPacket(); auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            if (packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength())->getType()==ST_BLOCKACK_REQ) barAir++;
        }
    }
    void observedAggregate(const Ieee80211Transmission *transmission) override {
        auto packet=transmission->getPacket(); auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        Ieee80211AmpduLayout layout;
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),transmission->getTxVector().format==Ieee80211TxVector::VHT,layout));
        const auto& ids=check_and_cast<SelectivePowerObserver *>(power(0))->selected();
        ASSERT(ids.size()==layout.members.size());
        if (aggregateAir==1) {
            ASSERT(ids.size()==3);
            for (size_t i=0;i<ids.size();i++) {
                const auto& member=layout.members[i]; auto offset=phy->getChunkLength()+member.offset;
                auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(offset);
                ASSERT(!header->getRetry() && header->getEosp());
                auto body=packet->peekDataAt<BytesChunk>(offset+header->getChunkLength(),member.length-B(4)-header->getChunkLength())->getBytes();
                originals.emplace(ids[i],Original{header->getSequenceNumber().get(),body,deadline(ids[i])});
                if (nested()) ASSERT(originals.at(ids[i]).deadline==maximumDeadline);
            }
            missing=ids.back();
        }
        else {
            ASSERT(aggregateAir==2 && ids.size()==1 && ids.front()==missing);
            const auto& member=layout.members.front(); auto offset=phy->getChunkLength()+member.offset;
            auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(offset);
            auto body=packet->peekDataAt<BytesChunk>(offset+header->getChunkLength(),member.length-B(4)-header->getChunkLength())->getBytes();
            const auto& old=originals.at(missing);
            ASSERT(header->getRetry() && header->getSequenceNumber().get()==old.sequence);
            ASSERT(body==old.body && deadline(missing)==old.deadline);
            // Exact intact nested body includes every constituent MeshControl;
            // ordinary retry does not split/reidentify it or restart lifetime.
            identityChecks++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();
        if (kind==20) {
            delete event; ASSERT(lostBa() && pendingChecks==0 && sampleOrder=="1100");
            ASSERT(check_and_cast<SelectiveDeadlineHcf *>(originator())->recovering(meshMib(1)->address));
            bool ending=false;
            for (const auto& period : power(0)->getServicePeriods()) if (period.direction==MeshPeriodDirection::OWNER) {
                ASSERT(period.ending && period.cap==0 && period.spent==0 && period.cohort==std::set<int64_t>{missing}); ending=true;
            }
            ASSERT(ending); pendingChecks++; return;
        }
        if (kind==10) {
            delete event;
            fixturePath(0,1,1,SimTime(2,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(2,SIMTIME_S));
            auto forwarding=check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"));
            forwarding->processSourceMsdu(phyMsdu("stamp-prime",meshMib(1)->address));
            forwarding->processSourceMsdu(phyMsdu("stamp-sentinel",meshMib(1)->address));
            return;
        }
        ServicePeriodDriver::handleMessage(event);
        if (kind==3) {
            if (nested()) maximumDeadline=check_and_cast<SelectiveDeadlineHcf *>(originator())->setConstituentDeadlines();
            check_and_cast<EndingResponseHcf *>(originator())->expireSentinel();
        }
        if (kind==100) {
            ASSERT(nChecks==1 || nChecks==2);
            double expected=nested() ? (nChecks==1 ? 13./8 : 107./64) : (nChecks==1 ? 5./4 : 39./32);
            ASSERT(expectedN==expected);
            auto actual=check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"));
            ASSERT(actual->getPayloadEstimate(meshMib(1)->address)==expected);
            literalNChecks++;
        }
    }
    void finishAssertions() override {
        ASSERT(aggregateAir==2 && originals.size()==3 && identityChecks==1 && literalNChecks==2);
        ASSERT(sampleOrder==(lostBa() ? "1100" : "1101"));
        if (lostBa()) ASSERT(barAir==1 && pendingChecks==1);
    }
};
namespace selectiveDriverRegistration { Define_Module(SelectivePeriodDriver); }
#endif
