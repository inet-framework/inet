#ifndef GAUSSMARKOV_MOBILITY_H
#define GAUSSMARKOV_MOBILITY_H

#include <omnetpp.h>
#include <BasicMobility.h>
#include <vector>


/**
 * @brief Chiang's random walk movement model. See NED file for more info.
 *
 * @author Marcin Kosiba
 */
class INET_API ChiangMobility : public BasicMobility
{
  private:
    static const short MoveMessageKind = 1;
    static const short StateUpMessageKind = 2;
    
  protected:
    double m_speed;              ///< speed of the host
    double m_updateInterval;     ///< time interval to update the hosts position
    double m_stateTransInterval; ///< how often to calculate the new state
    bool m_stationary;           ///< if true, the host doesn't move
    int m_states[2];             ///< FSM states [x,y]

  private:
    /** @brief gets the next state in the FSM*/
    int getNextStateIndex(int currentState, double rvalue);    
    
  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Move the host*/
    virtual void move();

    /** @brief Recalculate the FSM state */
    virtual void recalculateState();
};

#endif

