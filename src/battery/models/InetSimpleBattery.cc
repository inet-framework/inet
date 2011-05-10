/***************************************************************************
 * Simple battery model for inetmanet framework
 * Author:  Alfonso Ariza
 * Based in the mixim code Author:  Laura Marie Feeney
 *
 * Copyright 2009 Malaga University.
 * Copyright 2009 Swedish Institute of Computer Science.
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/

/*
 * A simple linear model of battery consumption.  Simple Battery
 * receives DrawMsg's from one or more devices, updates residual
 * capacity (total current * voltage * time), publishes HostState
 * notification on battery depletion, and provides time series and
 * summary information to Battery Stats module.
 */

#include <omnetpp.h>
#include "RadioState.h"
#include "InetSimpleBattery.h"
#include "Energy.h"

Define_Module(InetSimpleBattery);


void InetSimpleBattery::initialize(int stage)
{

    BasicBattery::initialize(stage); //DO NOT DELETE!!
    if (stage == 0)
    {
        voltage = par("voltage");
        nominalCapmAh = par("nominal");
        if (nominalCapmAh <= 0)
        {
            error("invalid nominal capacity value");
        }
        capmAh = par("capacity");

        // Publish capacity to BatteryStats every publishTime (if > 0) and
        // whenever capacity has changed by publishDelta (if < 100%).
        publishTime = 0;

        publishDelta = 1;
        publishDelta = par("publishDelta");
        if (publishDelta < 0 || publishDelta > 1)
        {
            error("invalid publishDelta value");
        }

        resolution = par("resolution");
        EV<< "capacity = " << capmAh << "mA-h (nominal = " << nominalCapmAh <<
        ") at " << voltage << "V" << endl;
        EV << "publishDelta = " << publishDelta * 100 << "%, publishTime = "
        << publishTime << "s, resolution = " << resolution << "sec"
        << endl;

        capacity = capmAh * 60 * 60 * voltage; // use mW-sec internally
        nominalCapacity = nominalCapmAh * 60 * 60 * voltage;

        residualCapacity = lastPublishCapacity = capacity;
        lifetime = -1; // -1 means not dead

        publishTime = par("publishTime");
        if (publishTime > 0)
        {
            lastUpdateTime = simTime();
            publish = new cMessage("publish", PUBLISH);
            publish->setSchedulingPriority(2000);
            scheduleAt(simTime() + publishTime, publish);
        }

        mCurrEnergy=NULL;
        if (par("ConsumedVector"))
            mCurrEnergy = new cOutVector("Consumed");
        // DISable by default (use BatteryStats for data collection)
        residualVec.disable();

        residualVec.setName("residualCapacity");
        residualVec.record(residualCapacity);

        timeout = new cMessage("auto-update", AUTO_UPDATE);
        timeout->setSchedulingPriority(500);
        scheduleAt(simTime() + resolution, timeout);
        lastUpdateTime = simTime();
        WATCH(lastPublishCapacity);
    }
}



int InetSimpleBattery::registerDevice(cObject *id,int numAccts)
{
    for (unsigned int i =0; i<deviceEntryVector.size(); i++)
        if (deviceEntryVector[i]->owner == id)
            error("device already registered!");
    if (numAccts < 1)
    {
        error("number of activities must be at least 1");
    }

    DeviceEntry *device = new DeviceEntry();
    device->owner = id;
    device->numAccts = numAccts;
    device->accts = new double[numAccts];
    device->times = new simtime_t[numAccts];
    for (int i = 0; i < numAccts; i++)
    {
        device->accts[i] = 0.0;
    }
    for (int i = 0; i < numAccts; i++)
    {
        device->times[i] = 0.0;
    }

    EV<< "initialized device "  << deviceEntryVector.size() << " with " << numAccts << " accounts" << endl;
    deviceEntryVector.push_back(device);
    return deviceEntryVector.size()-1;
}

void InetSimpleBattery::registerWirelessDevice(int id,double mUsageRadioIdle,double mUsageRadioRecv,double mUsageRadioSend,double mUsageRadioSleep)
{
    Enter_Method_Silent();
    if (deviceEntryMap.find(id)!=deviceEntryMap.end())
    {
        EV << "This device is register \n";
        return;
    }

    DeviceEntry *device = new DeviceEntry();
    device->numAccts = 4;
    device->accts = new double[4];
    device->times = new simtime_t[4];

    if (RadioState::IDLE>=4)
        error("Battery and RadioState problem");
    if (RadioState::RECV>=4)
        error("Battery and RadioState problem");
    if (RadioState::TRANSMIT>=4)
        error("Battery and RadioState problem");
    if (RadioState::SLEEP>=4)
        error("Battery and RadioState problem");
    device->radioUsageCurrent[RadioState::IDLE]=mUsageRadioIdle;
    device->radioUsageCurrent[RadioState::RECV]=mUsageRadioRecv;
    device->radioUsageCurrent[RadioState::TRANSMIT]=mUsageRadioSend;
    device->radioUsageCurrent[RadioState::SLEEP]=mUsageRadioSleep;

    for (int i = 0; i < 4; i++)
    {
        device->accts[i] = 0.0;
    }
    for (int i = 0; i < 4; i++)
    {
        device->times[i] = 0.0;
    }

    deviceEntryMap.insert(std::pair<int,DeviceEntry*>(id,device));
    if (mustSubscribe)
    {
        mpNb->subscribe(this, NF_RADIOSTATE_CHANGED);
        mustSubscribe=false;
    }
}

void InetSimpleBattery::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {

        switch (msg->getKind())
        {
        case AUTO_UPDATE:
            // update the residual capacity (ongoing current draw)
            scheduleAt(simTime() + resolution, timeout);
            deductAndCheck();
            break;

        case PUBLISH:
            // publish the state to the BatteryStats module
            lastPublishCapacity = residualCapacity;
            scheduleAt(simTime() + publishTime, publish);
            break;

        default:
            error("battery receives mysterious timeout");
            break;
        }
    }
    else
    {
        error("unexpected message");
        delete msg;
    }
}



void InetSimpleBattery::finish()
{
    // do a final update of battery capacity
    deductAndCheck();
    deviceEntryMap.clear();
    deviceEntryVector.clear();
}

void InetSimpleBattery::receiveChangeNotification (int aCategory, const cPolymorphic* aDetails)
{
    Enter_Method_Silent();
    //EV << "[Battery]: receiveChangeNotification" << endl;
    if (aCategory == NF_RADIOSTATE_CHANGED)
    {
        RadioState *rs = check_and_cast <RadioState *>(aDetails);

        DeviceEntryMap::iterator it = deviceEntryMap.find(rs->getRadioId());
        if (it==deviceEntryMap.end())
            return;

        if (rs->getState()>=it->second->numAccts)
            opp_error("Error in battery states");

        double current = it->second->radioUsageCurrent[rs->getState()];

        EV << simTime() << " wireless device " << rs->getRadioId() << " draw current " << current <<
        "mA, new state = " << rs->getState() << "\n";

        // update the residual capacity (finish previous current draw)
        deductAndCheck();

        // set the new current draw in the device vector
        it->second->draw = current;
        it->second->currentActivity = rs->getState();
    }
}



void InetSimpleBattery::draw(int deviceID, DrawAmount& amount, int activity)
{
    if (amount.getType() == DrawAmount::CURRENT)
    {

        double current = amount.getValue();
        if (activity < 0 && current != 0)
            error("invalid CURRENT message");

        EV << simTime() << " device " << deviceID <<
        " draw current " << current <<
        "mA, activity = " << activity << endl;

        // update the residual capacity (finish previous current draw)
        deductAndCheck();

        // set the new current draw in the device vector
        deviceEntryVector[deviceID]->draw = current;
        deviceEntryVector[deviceID]->currentActivity = activity;
    }

    else if (amount.getType() == DrawAmount::ENERGY)
    {
        double energy = amount.getValue();
        if (!(activity >=0 && activity < deviceEntryVector[deviceID]->numAccts))
        {
            error("invalid activity specified");
        }

        EV << simTime() << " device " << deviceID <<  " deduct " << energy <<
        " mW-s, activity = " << activity << endl;

        // deduct a fixed energy cost
        deviceEntryVector[deviceID]->accts[activity] += energy;
        residualCapacity -= energy;

        // update the residual capacity (ongoing current draw), mostly
        // to check whether to publish (or perish)
        deductAndCheck();
    }
    else
    {
        error("Unknown power type!");
    }
}

/**
 *  Function to update the display string with the remaining energy
 */

InetSimpleBattery::~InetSimpleBattery()
{
    while (!deviceEntryMap.empty())
    {
        delete deviceEntryMap.begin()->second;
        deviceEntryMap.erase(deviceEntryMap.begin());
    }

    while (!deviceEntryVector.empty())
    {
        delete deviceEntryVector.back();
        deviceEntryVector.pop_back();
    }
    if (mCurrEnergy)
        delete mCurrEnergy;
}


void InetSimpleBattery::deductAndCheck()
{
    // already depleted, devices should have stopped sending drawMsg,
    // but we catch any leftover messages in queue
    if (residualCapacity <= 0)
    {
        return;
    }

    simtime_t now = simTime();

    // If device[i] has never drawn current (e.g. because the device
    // hasn't been used yet or only uses ENERGY) the currentActivity is
    // still -1.  If the device is not drawing current at the moment,
    // draw has been reset to 0, so energy is also 0.  (It might perhaps
    // be wise to guard more carefully against fp issues later.)

    for (unsigned int i = 0; i < deviceEntryVector.size(); i++)
    {
        int currentActivity = deviceEntryVector[i]->currentActivity;
        if (currentActivity > -1)
        {
            double energy = deviceEntryVector[i]->draw * voltage * (now - lastUpdateTime).dbl();
            if (energy > 0)
            {
                deviceEntryVector[i]->accts[currentActivity] += energy;
                deviceEntryVector[i]->times[currentActivity] += (now - lastUpdateTime);
                residualCapacity -= energy;
            }
        }
    }

    for (DeviceEntryMap::iterator it = deviceEntryMap.begin(); it!=deviceEntryMap.end(); it++)
    {
        int currentActivity = it->second->currentActivity;
        if (currentActivity > -1)
        {
            double energy = it->second->draw * voltage * (now - lastUpdateTime).dbl();
            if (energy > 0)
            {
                it->second->accts[currentActivity] += energy;
                it->second->times[currentActivity] += (now - lastUpdateTime);
                residualCapacity -= energy;
            }
        }
    }


    lastUpdateTime = now;

    EV << "residual capacity = " << residualCapacity << "\n";

    cDisplayString* display_string = &getParentModule()->getDisplayString();

    // battery is depleted
    if (residualCapacity <= 0.0 )
    {

        EV << "[BATTERY]: " << getParentModule()->getFullName() <<" 's battery exhausted, stop simulation" << "\n";
        display_string->setTagArg("i", 1, "#ff0000");
        endSimulation();
    }

    // battery is not depleted, continue
    else
    {
        // publish the battery capacity if it changed by more than delta
        if ((lastPublishCapacity - residualCapacity)/capacity >= publishDelta)
        {
            lastPublishCapacity = residualCapacity;
            Energy* p_ene = new Energy(residualCapacity);
            mpNb->fireChangeNotification(NF_BATTERY_CHANGED, p_ene);
            delete p_ene;

            display_string->setTagArg("i", 1, "#000000"); // black coloring
            EV << "[BATTERY]: " << getParentModule()->getFullName() << " 's battery energy left: " << lastPublishCapacity  << "%" << "\n";
            //char buf[3];

        }
    }
    residualVec.record(residualCapacity);
    if (mCurrEnergy)
        mCurrEnergy->record(capacity-residualCapacity);
}
