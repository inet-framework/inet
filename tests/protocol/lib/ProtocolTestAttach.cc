//
// Protocol Test Framework -- attach the ProtocolTester to any running network.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/INETDefs.h"

#include "ProtocolTest.h"
#include "ProtocolTester.h"

namespace inet {
namespace protocoltest {

//
// When a build defines a single program (Define_ProtocolTestProgram) and the running
// network has no ProtocolTester of its own, this lifecycle listener creates one and attaches
// it to the network -- so a test can target an unmodified external network without editing
// its NED. The created ProtocolTester is an ordinary module (inspectable in Qtenv, etc.); it
// just isn't declared in the NED.
//
// It does nothing for the normal workflow (a network with its own tester selecting a named
// program), because that leaves no default program registered.
//
class ProtocolTestAttacher : public cISimulationLifecycleListener
{
  public:
    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override
    {
        // After the network is fully initialized but before the first event: any apps start
        // later (at their startTime), so subscribing now misses nothing.
        if (eventType != LF_POST_NETWORK_INITIALIZE || !ProtocolTestRegistry::hasDefault())
            return;
        cModule *network = cSimulation::getActiveSimulation()->getSystemModule();
        if (network == nullptr || findTester(network) != nullptr)
            return;   // the network already wired its own tester -- leave it alone

        cModuleType *type = cModuleType::get("inet.protocoltest.ProtocolTester");
        cModule *tester = type->create("protocolTester", network);
        tester->finalizeParameters();
        tester->buildInside();
        tester->callInitialize();   // the network's own init pass has already finished
    }

    virtual void listenerRemoved() override { delete this; }

  private:
    static cModule *findTester(cModule *parent)
    {
        for (cModule::SubmoduleIterator it(parent); !it.end(); ++it)
            if (dynamic_cast<ProtocolTester *>(*it) != nullptr)
                return *it;
        return nullptr;
    }
};

EXECUTE_ON_STARTUP(getEnvir()->addLifecycleListener(new ProtocolTestAttacher()));

} // namespace protocoltest
} // namespace inet
