// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHEXPIRYLIMITSUPPORT_H
#define INET_TEST_MESHEXPIRYLIMITSUPPORT_H

class ExpiryLimitDriver : public ServicePeriodDriver
{
    int expired=0, ordinary=0, failed=0, confirmed=0, bars=0, controls=0;
    int64_t opening=-1;
    std::set<int64_t> cohort;
    RecoveryHcfFixture *hcf() const { return check_and_cast<RecoveryHcfFixture *>(originator()); }
    MeshServicePeriodView owner() const {
        for (const auto& view : power(0)->getServicePeriods())
            if (view.peer==meshMib(1)->address && view.direction==MeshPeriodDirection::OWNER) return view;
        throw cRuntimeError("Expected pending expiry/ordinary owner");
    }
  protected:
    void initialize() override {
        ServicePeriodDriver::initialize();
        getSimulation()->getSystemModule()->subscribe(packetDroppedSignal,this);
    }
    void observedAggregate(const Ieee80211Transmission *) override {
        if (aggregateAir!=1) return;
        auto view=owner(); ASSERT(view.ending && view.cohort.size()==2 && view.cap==2 && view.spent==0);
        cohort=view.cohort;
        opening=check_and_cast<BudgetPowerObserver *>(power(0))->ownerOpening(meshMib(1)->address);
        ASSERT(hcf()->setEndingDeadlines(false)==2);
        at(simTime()+SimTime(2500,SIMTIME_US),10);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        ServicePeriodDriver::receiveSignal(source,signal,object,details);
        if (signal==packetDroppedSignal && source==hcf()) {
            auto packet=check_and_cast<Packet *>(object);
            if (cohort.count(packet->getId())) {
                auto reason=check_and_cast<PacketDropDetails *>(details)->getReason();
                expired+=reason==LIFETIME_EXPIRED; ordinary+=reason==RETRY_LIMIT_REACHED;
            }
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal && source==power(0)) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (fact->direction==MeshPeriodDirection::OWNER && fact->openingId==opening) {
                failed+=fact->outcome==MeshPeriodOutcome::FAILED;
                confirmed+=fact->outcome==MeshPeriodOutcome::CONFIRMED;
            }
        }
        if (signal==transmissionStartedSignal && from(source,"radio")==0) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            if (std::string(packet->getName())=="mesh-period-ending" && !failed && !confirmed) controls++;
            auto mode=packet->findTag<Ieee80211ModeReq>();
            if (!mode || mode->getTxVector().ampdu) return;
            auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            bars+=bool(dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength())));
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();
        if (kind!=4 && kind<10) { ServicePeriodDriver::handleMessage(event); return; }
        delete event;
        if (kind==10) {
            ASSERT(expired==1 && ordinary==0 && failed==0 && confirmed==0 && owner().cohort==cohort);
            ASSERT(hcf()->recovering(meshMib(1)->address));
            check_and_cast<RecoveryMacFixture *>(meshInterface(0)->getSubmodule("mac"))->releaseBar=true;
            hcf()->meshPowerEligibilityChanged();
        }
        else {
            std::cout << "expiry/limit expired=" << expired << " ordinary=" << ordinary << " failed=" << failed
                << " confirmed=" << confirmed << " bars=" << bars << " samples=" << samples << " failures=" << failures << std::endl;
            ASSERT(expired==1 && ordinary==1 && failed==1 && confirmed==0 && controls==0);
            ASSERT(samples==4 && failures==3 && bars>=1);
            emit(registerSignal("expiryLimitComplete"),1L);
        }
    }
};
namespace expiry_limit_registration { Define_Module(ExpiryLimitDriver); }
#endif
