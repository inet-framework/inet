#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include "INETDefs.h"
#include "BaseConnectionManager.h"

/**
 * @brief BaseConnectionManager implementation which only defines a
 * specific max interference distance.
 *
 * Calculates the maximum interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power.
 *
 * @ingroup connectionManager
 */
class INET_API DetailedRadioChannel : public BaseConnectionManager
{
    protected:

        /**
         * @brief Calculate interference distance
         *
         * Calculation of the interference distance based on the transmitter
         * power, wavelength, pathloss coefficient and a threshold for the
         * minimal receive Power
         *
         * You may want to overwrite this function in order to do your own
         * interference calculation
         */
        virtual double calcInterfDist();
};

#endif /*CONNECTIONMANAGER_H_*/
