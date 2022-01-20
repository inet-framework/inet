#include "inet/common/clock/ClockUserModuleMixin.h"

namespace inet {

class ClockTestApp : public ClockUserModuleMixin<cSimpleModule>
{
private:
        size_t idx = 0;
        std::vector<clocktime_t> timeVector;
        ClockEvent *afterClock;
        clocktime_t after;
        int repeat;
    public:
       ClockTestApp() : ClockUserModuleMixin() {}
    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void finish() override;

    protected:
        void doSchedule(clocktime_t t, ClockEvent *msg);
        bool doScheduleAfter(clocktime_t t, ClockEvent *msg);
};

Define_Module(ClockTestApp);

void ClockTestApp::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LAST) {
        auto c = getClockTime();
        auto s = computeSimTimeFromClockTime(c);
        EV << "start" << ": simtime: " << simTime() << ", clock: " << getClockTime() << ", computed simtime: " << computeSimTimeFromClockTime(getClockTime()) << endl;
        timeVector.push_back(1.1);
        timeVector.push_back(2.11);
        timeVector.push_back(2.89);
        timeVector.push_back(2.9999999);
        timeVector.push_back(3.11);
        idx = 0;
        for (auto ct: timeVector) {
            std::string timerName = "Timer_" + ct.str();
            auto msg = new ClockEvent(timerName.c_str());
            doSchedule(ct, msg);
        }
        after = 1.2;
        int repeat = 6;
        std::string timerName = "After_" + after.str();
        afterClock = new ClockEvent(timerName.c_str(), repeat);
        doScheduleAfter(after, afterClock);
    }
}

void ClockTestApp::doSchedule(clocktime_t ct, ClockEvent *msg)
{
    scheduleClockEventAt(ct, msg);
    EV << "schedule " << msg->getName() << " to clock: " << ct << ", scheduled simtime: " << msg->getArrivalTime() << ", scheduled clock: " << getArrivalClockTime(msg) << endl;
}

bool ClockTestApp::doScheduleAfter(clocktime_t ct, ClockEvent *msg)
{
    if (msg->getKind() > 0) {
        msg->setKind(msg->getKind() - 1);
        scheduleClockEventAfter(ct, msg);
        EV << "schedule " << msg->getName() << " after clock: " << ct << ", scheduled simtime: " << msg->getArrivalTime() << ", scheduled clock: " << getArrivalClockTime(msg) << endl;
        return true;
    }
    else {
        return false;
    }
}

void ClockTestApp::handleMessage(cMessage *msg)
{
    ASSERT(msg->isSelfMessage());
    EV << "arrived " << msg->getName() << ": simtime: " << simTime() << ", clock: " << getClockTime() << endl;
    if (msg == afterClock) {
        if (!doScheduleAfter(after, afterClock)) {
            delete msg;
            afterClock = nullptr;
        }
    }
    else
        delete msg;
}

void ClockTestApp::finish()
{
    EV << "finished: simtime: " << simTime() << ", clock: " << getClockTime() << ", computed simtime: " << computeSimTimeFromClockTime(getClockTime()) << endl;
}

} // namespace inet

