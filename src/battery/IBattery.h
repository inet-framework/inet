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

#ifndef __INET_BASIC_BATTERY_H
#define __INET_BASIC_BATTERY_H

// SYSTEM INCLUDES
#include "INETDefs.h"

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
    DrawAmount(int type = CURRENT, int value = 0) : type(type), value(value) {}

    /** @brief Returns the type of power drawn as PowerType. */
    virtual int getType() { return type; }
    /** @brief Returns the actual amount of power drawn. */
    virtual double getValue() { return value; }

    /** @brief Sets the type of power drawn. */
    virtual void setType(int t) { type = t; }
    /** @brief Sets the actual amount of power drawn. */
    virtual void setValue(double v) { value = v; }
};


class INET_API IBattery : public cSimpleModule, public INotifiable
{
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
    virtual int registerDevice(cObject *id, int numAccts) = 0;

    virtual void registerWirelessDevice(int id, double mUsageRadioIdle, double mUsageRadioRecv,
            double mUsageRadioSend, double mUsageRadioSleep) = 0;

    /**
     * @brief Draws power from the battery.
     *
     * The actual amount and type of power drawn is defined by the passed
     * DrawAmount parameter. Can be an fixed single amount or an amount
     * drawn over time.
     * The drainID identifies the device which drains the power.
     * "Account" identifies the account the power is drawn from.
     */
    virtual void draw(int drainID, DrawAmount& amount, int account) = 0;

//    virtual double getEnergy() {return residualCapacity;}

    // OPERATIONS

//    simtime_t GetLifetime() {return lifetime;}
};

class INET_API BatteryAccess : public ModuleAccess<IBattery>
{
  public:
    BatteryAccess() : ModuleAccess<IBattery>("battery") {}
};

#endif

