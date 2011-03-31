/* **************************************************************************
 * file:        TwoRayGround.h
 *
 * author:      Andreas Kuntz
 *
 * copyright:	(c) 2008 Institute of Telematics, University of Karlsruhe (TH)
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
 * part of:     SNE4OMF   http://www.tm.uka.de/sne4omf
 * description: - This Module implements the two ray ground radio propagations model
 *
 ***************************************************************************
 */
#ifndef __TWO_RAY_GROUND_MODEL_H__
#define __TWO_RAY_GROUND_MODEL_H__

#include "FreeSpaceModel.h"
class INET_API TwoRayGroundModel : public FreeSpaceModel {

    public:
	~TwoRayGroundModel();
     virtual void initializeFrom(cModule *radioModule);
    /**
     * To be redefined to calculate the received power of a transmission.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance);

    private:
	double ht, hr;
};

#endif /* __TWO_RAY_GROUND_H__ */
