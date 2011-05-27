/* **************************************************************************
 * file:        Rice.h
 *
 * author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
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
 ***************************************************************************
 * part of:     SNE4OMF
 * description: - This Module implements the rice radio propagations model
 * http://www.tm.uka.de/sne4omf
 *
 ***************************************************************************
 */

#ifndef __RICE_MODEL_H__
#define __RICE_MODEL_H__

#include "FreeSpaceModel.h"
#include <math.h>
#include <FWMath.h>
using namespace std;

/**
 * @brief Rice Propagation Model
 *
 * This Class implements the Rice PropagationModel
 * This is a  probabilistic Propagation Model
 *
 * @author Oliver Graute
 *
 * @ingroup snrEvalwithPropagation */

/** @class Rice This Class implements the Rice PropagationModel it is a probabilistic Propagation Model */
class INET_API RiceModel : public FreeSpaceModel {

    public:
    ~RiceModel(){};
     virtual void initializeFrom(cModule *);
    /**
     * To be redefined to calculate the received power of a transmission.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance);
    private:
    /** @brief  Ricean K Factor */
    double K;
};


#endif /* __RICE_H__ */
