/***************************************************************************
 * author:      Andreas Kuntz
 *
 * copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
 *
 * author:      Alfonso Ariza
 *              Malaga university
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************/

#ifndef __INET_NAKAGAMIFADING_H_
#define __INET_NAKAGAMIFADING_H_

#include "FreeSpacePathLoss.h"

namespace radio
{

/**
 * This class implements the Nakagami fading model.
 */
class INET_API NakagamiFading : public FreeSpacePathLoss
{
    protected:
        double shapeFactor;

    protected:
        virtual void initialize(int stage);

    public:
        NakagamiFading();
        virtual void printToStream(std::ostream &stream) const;
        virtual double computePathLoss(mps propagationSpeed, Hz carrierFrequency, m distance) const;
};

}

#endif
