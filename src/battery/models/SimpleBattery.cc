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

#include "INETDefs.h"

#include "SimpleBattery.h"

#include "Energy.h"
#include "RadioState.h"

Define_Module(SimpleBattery);


simsignal_t SimpleBattery::currCapacitySignal = SIMSIGNAL_NULL;
simsignal_t SimpleBattery::consumedEnergySignal = SIMSIGNAL_NULL;

SimpleBattery::DeviceEntry::DeviceEntry()
{
    currentState = 0;
    numAccts = 0;
    currentActivity = -1;
    accts = NULL;
    times = NULL;
    owner = NULL;
    radioUsageCurrent = NULL;
}

SimpleBattery::DeviceEntry::~DeviceEntry()
{
    delete [] accts;
    delete [] times;
    delete [] radioUsageCurrent;
}

SimpleBattery::SimpleBattery()
{
    timeout = NULL;
    publish = NULL;
    mpNb = NULL;
}

SimpleBattery::~SimpleBattery()
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
    cancelAndDelete(publish);
    cancelAndDelete(timeout);
}

void SimpleBattery::initialize(int stage)
{
    IBattery::initialize(stage); //DO NOT DELETE!!

    if (stage == 0)
    {
        mustSubscribe = true;
        mpNb = NotificationBoardAccess().get();

        voltage = par("voltage");
        double nominalCapmAh = par("nominal");
        double capmAh = par("capacity");
        if (nominalCapmAh <= 0 || nominalCapmAh < capmAh)
            error("Invalid nominal capacity value:%g mAh (capacity=%g mAh)", nominalCapmAh, capmAh);

        // Publish capacity to BatteryStats every publishTime (if > 0) and
        // whenever capacity has changed by publishDelta (if < 100%).
        publishTime = par("publishTime");
        publishDelta = par("publishDelta");
        if (publishDelta < 0 || publishDelta > 1)
            error("invalid publishDelta value: %g", publishDelta);

        resolution = par("resolution");

        EV << "capacity = " << capmAh << "mA-h (nominal = " << nominalCapmAh
           << ") at " << voltage << "V" << endl;
        EV << "publishDelta = " << publishDelta * 100 << "%, publishTime = "
           << publishTime << "s, resolution = " << resolution << "sec"
           << endl;

        double capacity = capmAh * 3600.0 * voltage; // use mW-sec internally
        nominalCapacity = nominalCapmAh * 3600.0 * voltage;
        lastUpdateTime = simTime();

        residualCapacity = lastPublishCapacity = capacity;

        if (publishTime > 0)
        {
            publish = new cMessage("publish", PUBLISH);
            publish->setSchedulingPriority(2000);
            scheduleAt(simTime() + publishTime, publish);
        }

        currCapacitySignal = registerSignal("currCapacity");
        consumedEnergySignal = registerSignal("consumedEnergy");

        timeout = new cMessage("auto-update", AUTO_UPDATE);
        timeout->setSchedulingPriority(500);
        scheduleAt(simTime() + resolution, timeout);
        WATCH(lastPublishCapacity);

        publishCapacity();
    }
}

int SimpleBattery::registerDevice(cObject *id, int numAccts)
{
    for (unsigned int i = 0; i < deviceEntryVector.size(); i++)
        if (deviceEntryVector[i]->owner == id)
            error("device already registered!");

    if (numAccts < 1)
    {
        error("Number of activities must be at least 1");
    }

    DeviceEntry *device = new DeviceEntry();
    device->owner = id;
    device->numAccts = numAccts;
    device->accts = new double[numAccts];
    device->times = new simtime_t[numAccts];
    for (int i = 0; i < numAccts; i++)
    {
        device->accts[i] = 0.0;
        device->times[i] = SIMTIME_ZERO;
    }

    EV << "Initialized device "  << deviceEntryVector.size() << " with "
       << numAccts << " accounts" << endl;
    deviceEntryVector.push_back(device);
    return deviceEntryVector.size()-1;
}

void SimpleBattery::registerWirelessDevice(int id, double mUsageRadioIdle,
        double mUsageRadioRecv, double mUsageRadioSend, double mUsageRadioSleep)
{
    Enter_Method_Silent();

    if (deviceEntryMap.find(id) != deviceEntryMap.end())
    {
        EV << "This wireless device (id=" << id << ") already registered\n";
        return;
    }

    DeviceEntry *device = new DeviceEntry();
    device->numAccts = RadioState::NUMBER_OF_ELEMENTS;
    device->accts = new double[RadioState::NUMBER_OF_ELEMENTS];
    device->times = new simtime_t[RadioState::NUMBER_OF_ELEMENTS];

    device->radioUsageCurrent = new double[RadioState::NUMBER_OF_ELEMENTS];
    device->radioUsageCurrent[RadioState::IDLE] = mUsageRadioIdle;
    device->radioUsageCurrent[RadioState::RECV] = mUsageRadioRecv;
    device->radioUsageCurrent[RadioState::TRANSMIT] = mUsageRadioSend;
    device->radioUsageCurrent[RadioState::SLEEP] = mUsageRadioSleep;

    for (int i = 0; i < RadioState::NUMBER_OF_ELEMENTS; i++)
    {
        device->accts[i] = 0.0;
        device->times[i] = SIMTIME_ZERO;
    }

    deviceEntryMap.insert(std::pair<int,DeviceEntry*>(id, device));
    if (mustSubscribe)
    {
        mustSubscribe = false;
        mpNb->subscribe(this, NF_RADIOSTATE_CHANGED);
    }
}

void SimpleBattery::handleMessage(cMessage *msg)
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
            publishCapacity();
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

void SimpleBattery::finish()
{
    // do a final update of battery capacity
    deductAndCheck(true);

    deviceEntryMap.clear();
    deviceEntryVector.clear();
    cancelAndDelete(publish);
    publish = NULL;
    cancelAndDelete(timeout);
    timeout = NULL;
}

void SimpleBattery::receiveChangeNotification(int aCategory, const cPolymorphic* aDetails)
{
    Enter_Method_Silent();

    //EV << "[Battery]: receiveChangeNotification" << endl;
    if (aCategory == NF_RADIOSTATE_CHANGED)
    {
        RadioState *rs = check_and_cast <RadioState *>(aDetails);

        DeviceEntryMap::iterator it = deviceEntryMap.find(rs->getRadioId());
        if (it == deviceEntryMap.end())
            return;

        // update the residual capacity (finish previous current draw)
        deductAndCheck();

        if (rs->getState() >= it->second->numAccts)
            opp_error("Error in battery states");

        double current = it->second->radioUsageCurrent[rs->getState()];

        EV << simTime() << " wireless device " << rs->getRadioId() << " draw current " << current
           << "mA, new state = " << rs->getState() << "\n";

        // set the new current draw in the device vector
        it->second->draw = current;
        it->second->currentActivity = rs->getState();
    }
}

void SimpleBattery::draw(int deviceID, DrawAmount& amount, int activity)
{
    if (amount.getType() == DrawAmount::CURRENT)
    {
        double current = amount.getValue();
        if (activity < 0 && current != 0)
            error("invalid CURRENT message");

        EV << simTime() << " device " << deviceID << " draw current " << current
           << "mA, activity = " << activity << endl;

        // update the residual capacity (finish previous current draw)
        deductAndCheck();

        // set the new current draw in the device vector
        deviceEntryVector[deviceID]->draw = current;
        deviceEntryVector[deviceID]->currentActivity = activity;
    }
    else if (amount.getType() == DrawAmount::ENERGY)
    {
        double energy = amount.getValue();
        if (!(activity >= 0 && activity < deviceEntryVector[deviceID]->numAccts))
        {
            error("invalid activity specified");
        }

        EV << simTime() << " device " << deviceID <<  " deduct " << energy
           << " mW-s, activity = " << activity << endl;

        // deduct a fixed energy cost
        deviceEntryVector[deviceID]->accts[activity] += energy;

        // update the residual capacity (ongoing current draw), mostly
        // to check whether to publish (or perish)
        deductAndCheck(false, energy);
    }
    else
    {
        error("Unknown power type!");
    }
}

/**
 *  Function to calculate consumed energy and update the remaining capacity,
 *  and publish these, when need.
 */
void SimpleBattery::deductAndCheck(bool mustPublish, double consumedEnergy)
{
    ASSERT(consumedEnergy >= 0.0);

    // already depleted, devices should have stopped sending drawMsg,
    // but we catch any leftover messages in queue
    if (residualCapacity <= 0.0)
        return;

    simtime_t now = simTime();

    // If device[i] has never drawn current (e.g. because the device
    // hasn't been used yet or only uses ENERGY) the currentActivity is
    // still -1.  If the device is not drawing current at the moment,
    // draw has been reset to 0, so energy is also 0.  (It might perhaps
    // be wise to guard more carefully against fp issues later.)

    if (now != lastUpdateTime)
    {
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
                    consumedEnergy += energy;
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
                    consumedEnergy += energy;
                }
            }
        }

        lastUpdateTime = now;
        residualCapacity -= consumedEnergy;
        emit(currCapacitySignal, residualCapacity *100.0 / nominalCapacity);
        emit(consumedEnergySignal, consumedEnergy);
    }
    if (mustPublish
            || residualCapacity <= 0.0
            || (lastPublishCapacity - residualCapacity) / nominalCapacity >= publishDelta)
        publishCapacity();
}

/**
 *  Function to update the display string with the remaining energy
 */
void SimpleBattery::publishCapacity()
{
    EV << "residual capacity = " << residualCapacity << "\n";

    int capacityPercent = 0;

    if (residualCapacity <= 0.0)   //TODO suggest: use minimal capacity parameter instead 0.0
    {
        lastPublishCapacity = residualCapacity;
        // battery is depleted
        EV << "[BATTERY]: " << getParentModule()->getFullName()
           <<" 's battery exhausted, stop simulation" << "\n";
        endSimulation();    //FIXME why can stop the simulation?
    }
    else
    {
        lastPublishCapacity = residualCapacity;
        capacityPercent = lastPublishCapacity * 100.0 / nominalCapacity;
        Energy* p_ene = new Energy(residualCapacity);
        mpNb->fireChangeNotification(NF_BATTERY_CHANGED, p_ene);
        delete p_ene;

        EV << "[BATTERY]: " << getParentModule()->getFullName() << " 's battery energy left: "
           << capacityPercent << "%" << "\n";
    }

    if (ev.isGUI())
    {
        char buf[20];
        sprintf(buf, " %d%%", capacityPercent);
        getDisplayString().setTagArg("t", 0, buf);
    }

    if (publish)
    {
        if (publish->isScheduled())
            cancelEvent(publish);
        scheduleAt(simTime() + publishTime, publish);
    }
}

