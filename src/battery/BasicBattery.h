/***************************************************************************
 * Simple battery model for inetmanet framework
 * Author:  Isabel Dietrich
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/

/**
 * @short Implementation of a simple battery model
 *      "real" battery models should subclass this!
 *      The basic class publishes the remaining energy on the notification board,
 *      but does not decrement the energy!
 *      It can therefore be used for hosts having an infinite energy supply
 *      i.e. a power plug
 * @author Isabel Dietrich
*/

#ifndef BASIC_BATTERY_H
#define BASIC_BATTERY_H

// SYSTEM INCLUDES
#include <omnetpp.h>

// INCLUDES for access to the Notification board (publish energy)
#include "NotifierConsts.h"
#include "NotificationBoard.h"


/**
 * @brief Defines the amount of power drawn by a device from
 * a power source.
 *
 * Used as generic amount parameter for BaseBatteries "draw"-method.
 *
 * Can be either an instantaneous draw of a certain energy amount
 * in mWs (type=ENERGY) or a draw of a certain current in mA over
 * time (type=CURRENT).
 *
 * Can be sub-classed for more complex power draws.
 *
 */
class DrawAmount
{
  public:
    /** @brief The type of the amount to draw.*/
    enum PowerType
    {
        CURRENT,    /** @brief Current in mA over time. */
        ENERGY      /** @brief Single fixed energy draw in mWs */
    };

  protected:
    /** @brief Stores the type of the amount.*/
    int type;

    /** @brief Stores the actual amount.*/
    double value;

  public:
    DrawAmount(int type = CURRENT, int value = 0):
            type(type),
            value(value)
    {}

    /** @brief Returns the type of power drawn as PowerType. */
    virtual int getType() { return type; }
    /** @brief Returns the actual amount of power drawn. */
    virtual double getValue() { return value; }

    /** @brief Sets the type of power drawn. */
    virtual void setType(int t) { type = t; }
    /** @brief Sets the actual amount of power drawn. */
    virtual void setValue(double v) { value = v; }
};


class INET_API BasicBattery : public cSimpleModule, public INotifiable
{
  public:
    // LIFECYCLE

    virtual void initialize(int);
    virtual void finish();

    virtual int registerDevice(cObject *id,int numAccts)
    {
        error("BasicBattery::registerDevice not overloaded"); return 0;
    }
    virtual void registerWirelessDevice(int id,double mUsageRadioIdle,double mUsageRadioRecv,double mUsageRadioSend,double mUsageRadioSleep)
    {
        error("BasicBattery::registerWirelessDevice not overloaded");
    }

    virtual void draw(int drainID, DrawAmount& amount, int account)
    {
        error("BasicBattery::draw not overloaded");
    }
    double GetEnergy () {return residualCapacity;}


    // OPERATIONS


    simtime_t   GetLifetime() {return lifetime;}

  protected:
    // battery parameters
    double capmAh;
    double nominalCapmAh;
    double voltage;

    bool mustSubscribe;
    // debit battery at least once every resolution seconds
    simtime_t resolution;
    cMessage *timeout;
    double publishDelta;
    simtime_t publishTime;
    simtime_t lastUpdateTime;

    // INTERNAL state
    double capacity;
    double nominalCapacity;
    double residualCapacity;
    double lastPublishCapacity;
    simtime_t lifetime;


    // OPERATIONS
    void HandleSelfMsg(cMessage*);


    // pointer to the notification board
    NotificationBoard*  mpNb;

  private:
    // OPERATIONS
    // void         handleMessage(cMessage *msg);

    virtual void receiveChangeNotification (int, const cPolymorphic*);

};


class INET_API BatteryAccess : public ModuleAccess<BasicBattery>
{
  public:
    BatteryAccess() : ModuleAccess<BasicBattery>("battery") {}
};


#endif

