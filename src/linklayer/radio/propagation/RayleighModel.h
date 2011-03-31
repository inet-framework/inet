/* **************************************************************************
 * file:        Rayleigh.h
 *
 * author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
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
 * part of:     SNE4OMF
 * description: - This Module implements the rayleigh radio propagations model
 * http://www.tm.uka.de/sne4omf
 *
 ***************************************************************************
 */

#ifndef __RAYLEIGH_MODEL_H__
#define __RAYLEIGH_MODEL_H__

#include "FreeSpaceModel.h"
using namespace std;

/**
 * @brief Rayleigh Propagation Model
 *
 * This Class implements the Rayleigh PropagationModel
 * This is a  propabilitic Propagation Model
 * see Rappaport for details
 * @author Oliver Graute
 *
 * @ingroup snrEvalwithPropagation */


/** @class Rayleigh This Class implements the Rayleigh PropagationModel it is a ropabilitic Propagation Model */
class INET_API RayleighModel : public FreeSpaceModel
{
public:

  ~RayleighModel();

 	 virtual void initializeFrom(cModule *radioModule);
    /**
     * To be redefined to calculate the received power of a transmission.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance);

};

#endif /* __RAYLEIGH_H__ */
