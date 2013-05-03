/* **************************************************************************
 * file:        FreeSpace.h
 *
 * author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
 *
 * copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
 *
 ** author:      Alfonso Ariza
 *              Malaga university
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     SNE4OMF
 * description: - This Module implements the freespace radio propagations model
 * http://www.tm.uka.de/sne4omf
 *
 ***************************************************************************
 */

#ifndef __FREE_SPACE_MODEL_H__
#define __FREE_SPACE_MODEL_H__

#include <list>
#include <string>
#include <math.h>

#include "INETDefs.h"

#include "FWMath.h"
#include "IReceptionModel.h"

using namespace std;

/**
 * @brief FreeSpace Propagation Model
 *
 * This Class implements the FreeSpace PropagationModel
 * This is a deterministic Propagation Model
 *
 * @author Oliver Graute
 *
 * @ingroup snrEvalwithPropagation */


/** @class FreeSpace this Class implements the FreeSpace PropagationModel it is a deterministic Propagation Model */
class INET_API FreeSpaceModel : public IReceptionModel {

public:
    virtual void initializeFrom(cModule *radioModule);
    /**
     * To be redefined to calculate the received power of a transmission.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance);
    virtual double calculateDistance(double pSend, double pRec, double carrierFrequency);
    ~FreeSpaceModel() { };

    protected:
        double Gr, Gt, L;
        double pathLossAlpha;
        virtual void initializeFreeSpace(cModule *);
        virtual double freeSpace(double Gt, double Gr, double L, double Pt, double lambda, double distance, double pathLossAlpha);
};


#endif /* __FREE_SPACE_H__ */
