#ifndef GAUSSMARKOV_MOBILITY_H
#define GAUSSMARKOV_MOBILITY_H

#include <omnetpp.h>
#include <BasicMobility.h>


/**
 * @brief Gauss Markov movement model. See NED file for more info.
 *
 * @author Marcin Kosiba
 */
class INET_API GaussMarkovMobility : public BasicMobility
{
  protected:
    double m_speed;          ///< speed of the host
    double m_angle;          ///< angle of linear motion
    double m_alpha;          ///< alpha parameter
    double m_updateInterval; ///< time interval to update the hosts position
    int m_margin;            ///< margin at which the host gets repelled from the border
    bool m_stationary;       ///< if true, the host doesn't move
    double m_speedMean;      ///< speed mean
    double m_angleMean;      ///< angle mean
    double m_variance;       ///< variance

  private:
    /** @brief If the host is too close to the border it is repelled */
    void preventBorderHugging();
    
  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Move the host*/
    virtual void move();
};

#endif

