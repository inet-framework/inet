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

#ifndef INET_SIMPLE_BATTERY_H
#define INET_SIMPLE_BATTERY_H

#include <vector>
#include <map>

#include "INETDefs.h"

#include "IBattery.h"

/**
 * Simple battery module
 *
 * generate an error, when discharged.
 */
class INET_API SimpleBattery : public IBattery
{
  protected:
    class DeviceEntry
    {
      public:
        DeviceEntry();
        ~DeviceEntry();
        int currentState;
        cObject *owner;
        double *radioUsageCurrent;
        double  draw;
        int     currentActivity;
        int     numAccts;
        double  *accts;
        simtime_t   *times;
    };

    typedef std::map<int,DeviceEntry*>  DeviceEntryMap;
    typedef std::vector<DeviceEntry*>  DeviceEntryVector;
    DeviceEntryMap deviceEntryMap;
    DeviceEntryVector deviceEntryVector;

  public:
    SimpleBattery();
    ~SimpleBattery();

  protected:
    virtual void initialize(int);
    virtual int numInitStages() const {return 1;}
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

    // update capacity and publish when need
    virtual void deductAndCheck(bool mustPublish = false);

    // publish capacity and restart publish timer
    virtual void publishCapacity();

    // implements INotifiable:
    virtual void receiveChangeNotification(int aCategory, const cPolymorphic* aDetails);

  public:
    /**
     * @brief Registers a power draining device with this battery.
     *
     * Takes the name of the device as well as a number of accounts
     * the devices draws power for (like rx, tx, idle for a radio device).
     *
     * Returns an ID by which the device can identify itself to the
     * battery.
     *
     * Has to be implemented by actual battery implementations.
     */
    virtual int registerDevice(cObject *id, int numAccts);

    virtual void registerWirelessDevice(int id, double mUsageRadioIdle, double mUsageRadioRecv,
            double mUsageRadioSend, double mUsageRadioSleep);

    /**
     * @brief Draws power from the battery.
     *
     * The actual amount and type of power drawn is defined by the passed
     * DrawAmount parameter. Can be an fixed single amount or an amount
     * drawn over time.
     * The drainID identifies the device which drains the power.
     * "Account" identifies the account the power is drawn from.
     */
    virtual void draw(int drainID, DrawAmount& amount, int account);

    virtual double getVoltage() const { return voltage; }

    /** @brief current state of charge of the battery, relative to its
     * rated nominal capacity [0..1]
     */
    virtual double getEstimateResidualRelative() const { return residualCapacity / nominalCapacity; }

    /** @brief current state of charge of the battery (mW-s) */
    double getEstimateResidualAbs() const { return residualCapacity; }

  protected:
    enum msgType
    {
        AUTO_UPDATE, PUBLISH,
    };

    cMessage *publish;
    cMessage *timeout;

    // battery parameters
    double voltage;

    bool mustSubscribe;
    // debit battery at least once every resolution seconds
    simtime_t resolution;
    double publishDelta;
    simtime_t publishTime;
    simtime_t lastUpdateTime;

    // INTERNAL state
    double nominalCapacity;
    double residualCapacity;
    double lastPublishCapacity;

    // pointer to the notification board
    NotificationBoard*  mpNb;

    //statistics:
    static simsignal_t currCapacitySignal;
    static simsignal_t consumedEnergySignal;
};

#endif

