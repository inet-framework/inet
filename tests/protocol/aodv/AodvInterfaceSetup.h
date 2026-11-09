// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_INTERFACE_SETUP_TEST_H
#define __AODV_INTERFACE_SETUP_TEST_H
#include "inet/common/SimpleModule.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif
using namespace inet;

class AodvInterfaceSetup : public SimpleModule, public LifecycleController
{
    cMessage *timer = nullptr;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        std::string mode = par("mode").stdstringValue();
        if (stage == INITSTAGE_ROUTER_ID_ASSIGNMENT) {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.dut.interfaceTable"));
            auto primary = interfaces->findInterfaceByName("wlan0");
            auto secondary = interfaces->findInterfaceByName("wlan1");
            if (mode == "Down") secondary->setState(NetworkInterface::DOWN);
            if (mode == "Unaddressed") secondary->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(Ipv4Address::UNSPECIFIED_ADDRESS);
            if (mode == "DuplicateAddress") secondary->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(primary->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
            if (mode == "MixedSecondary" || mode == "ExplicitIpv6") {
#ifdef INET_WITH_IPv6
                auto target = mode == "ExplicitIpv6" ? primary : secondary;
                delete target->removeProtocolData<Ipv4InterfaceData>();
                target->addProtocolData<Ipv6InterfaceData>()->assignAddress(Ipv6Address("2001:db8::1"), false, 0, 0);
#else
                throw cRuntimeError("Test setup requires IPv6 support");
#endif
            }
        }
        if (stage == INITSTAGE_APPLICATION_LAYER && mode == "StartDownUnknown") {
            timer = new cMessage("start-down-node"); scheduleAt(0.1, timer);
        }
    }
    void handleMessage(cMessage *message) override {
        ASSERT(message == timer);
        std::cout << "before actual start\n";
        auto operation = new ModuleStartOperation(); LifecycleOperation::StringMap parameters;
        operation->initialize(getModuleByPath("^.dut"), parameters); ASSERT(initiateOperation(operation));
    }
    void finish() override { std::cout << "startup accepted\n"; }
  public:
    ~AodvInterfaceSetup() { cancelAndDelete(timer); }
};
Define_Module(AodvInterfaceSetup);
#endif
