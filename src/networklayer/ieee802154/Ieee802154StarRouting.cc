/**
 * @short Implementation of a simple packets forward function for IEEE 802.15.4 star network
 *  support device <-> PAN coordinator <-> device transmission
    MAC address translation will be done in MAC layer (refer to Ieee802154Mac::handleUpperMsg())
 * @author Feng Chen
*/

#include "Ieee802154StarRouting.h"

//#undef EV
//#define EV (ev.isDisabled()||!m_debug) ? std::cout : ev ==> EV is now part of <omnetpp.h>

Define_Module( Ieee802154StarRouting );

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ===================================
/**
 * Initialization routine
 */
void Ieee802154StarRouting::initialize(int aStage)
{
    cSimpleModule::initialize(aStage); //DO NOT DELETE!!
    if (0 == aStage)
    {
        // WirelessMacBase stuff...
        mUppergateIn  = findGate("uppergateIn");
        mUppergateOut = findGate("uppergateOut");
        mLowergateIn  = findGate("lowergateIn");
        mLowergateOut = findGate("lowergateOut");

        m_moduleName    = getParentModule()->getFullName();

        m_debug         = par("debug");
        isPANCoor       = par("isPANCoor");

        numForward      = 0;
    }
}

void Ieee802154StarRouting::finish()
{
    recordScalar("num of pkts forwarded", numForward);
}

/////////////////////////////// Msg handler ///////////////////////////////////////
void Ieee802154StarRouting::handleMessage(cMessage* msg)
{
    Ieee802154AppPkt* appPkt = check_and_cast<Ieee802154AppPkt *>(msg);

    // coming from App layer
    if (msg->getArrivalGateId() == mUppergateIn)
    {
        Ieee802154NetworkCtrlInfo *control_info = new Ieee802154NetworkCtrlInfo();

        if (isPANCoor)      // I'm PAN coordinator, msg is destined for a device
        {
            control_info->setToParent(false);
            control_info->setDestName(appPkt->getDestName());
        }
        else        // should always be sent to PAN coordinator first
        {
            control_info->setToParent(true);
            control_info->setDestName("PAN Coordinator");
        }

        appPkt->setControlInfo(control_info);
        send(appPkt, mLowergateOut);
    }

    // coming from MAC layer
    else if (msg->getArrivalGateId() == mLowergateIn)
    {
        if (strcmp(appPkt->getDestName(), m_moduleName) == 0)
        {
            EV << "[NETWORK]: sending received pkt to upper layer" << endl;
            send(appPkt, mUppergateOut);    // to app layer
        }
        else if (isPANCoor)     // need to forward this pkt
        {
            Ieee802154NetworkCtrlInfo *control_info = new Ieee802154NetworkCtrlInfo();
            control_info->setToParent(false);
            control_info->setDestName(appPkt->getDestName());
            appPkt->setControlInfo(control_info);
            EV << "[NETWORK]: received a pkt from " << appPkt->getSourceName() << " and forward it to " << appPkt->getDestName() << endl;
            numForward++;
            send(appPkt, mLowergateOut);
        }
        else
        {
            EV << "[NETWORK]: my name is " << m_moduleName << endl;
            EV << "[NETWORK]: received pkt is destined for " << appPkt->getDestName() << endl;
            error("[NETWORK]: devices are not able to forward pkts in a star network");
        }
    }
    else
    {
        // not defined
    }
}




