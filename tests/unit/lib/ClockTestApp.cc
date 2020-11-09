#include "inet/common/clock/ClockUserModuleMixin.h"

namespace inet {

class ClockTestApp : public ClockUserModuleMixin<cSimpleModule>
{
private:
        size_t idx = 0;
        std::vector<clocktime_t> timeVector;
    public:
       ClockTestApp() : ClockUserModuleMixin() {}
    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void finish() override;

    protected:
        void doSchedule(clocktime_t t, ClockEvent *msg);
};

Define_Module(ClockTestApp);

void ClockTestApp::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LAST) {
        EV << "start\n";
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
    }
}

void ClockTestApp::doSchedule(clocktime_t ct, ClockEvent *msg)
{
    scheduleClockEventAt(ct, msg);
    EV << "schedule " << msg->getName() << " to clock: " << ct << ", scheduled simtime: " << msg->getArrivalTime() << ", scheduled clock: " << getArrivalClockTime(msg) << endl;
}

void ClockTestApp::handleMessage(cMessage *msg)
{
    ASSERT(msg->isSelfMessage());
    EV << "arrived " << msg->getName() << ": simtime: " << simTime() << ", clock: " << getClockTime() << endl;
    delete msg;
}

void ClockTestApp::finish()
{
    EV << "finished: simtime: " << simTime() << ", clock: " << getClockTime() << endl;
}

} // namespace inet

