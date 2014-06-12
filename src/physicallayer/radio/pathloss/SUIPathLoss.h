/***************************************************************************
 * author:      Konrad Polys, Krzysztof Grochla
 *
 * copyright:   (c) 2013 The Institute of Theoretical and Applied Informatics
 *                       of the Polish Academy of Sciences, Project
 *                       LIDER/10/194/L-3/11/ supported by NCBIR
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************/

#ifndef __INET_SUIPATHLOSS_H_
#define __INET_SUIPATHLOSS_H_

#include <string>
#include "IRadioSignalPathLoss.h"

namespace radio
{

/**
 * This class implements the empirical Stanford University Interim path loss model.
 *
 * @author Konrad Polys, Krzysztof Grochla
 */
class INET_API SUIPathLoss : public cModule, public IRadioSignalPathLoss
{
    protected:
        /** @brief Transmitter antenna high */
        m ht;

        /** @brief Receiver antenna high */
        m hr;

        double a, b, c, d, s;

    protected:
        virtual void initialize(int stage);

    public:
        SUIPathLoss();
        virtual void printToStream(std::ostream &stream) const;
        virtual double computePathLoss(mps propagationSpeed, Hz carrierFrequency, m distance) const;
};

}

#endif
