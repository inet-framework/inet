/* **************************************************************************
 * file:        SUIModel.h
 *
 * author:      Konrad Polys, Krzysztof Grochla
 *
 * copyright:   (c) 2013 The Institute of Theoretical and Applied Informatics
 *                       of the Polish Academy of Sciences, Project
                          LIDER/10/194/L-3/11/ supported by NCBIR
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * description: - This Module implements the Stanford University Interim
 *                propagation model.
 *
 ***************************************************************************
 */

#ifndef __SUIMODEL_H__
#define __SUIMODEL_H__

#include "FreeSpaceModel.h"
#include <math.h>
#include <FWMath.h>
using namespace std;

/**
 * @brief  SUI PropagationModel
 *
 * This class implements Stanford University
 * Interim propagation model.
 * This is empirical propagation model.
 *
 * @author Konrad Polys, Krzysztof Grochla
 *
 * @ingroup snrEvalwithPropagation */

/** @class This class implement SUI empirical propagation model */
class INET_API SUIModel : public FreeSpaceModel {

public:
    ~SUIModel(){};
    virtual void initializeFrom(cModule *);
    /**
     * To be redefined to calculate the received power of a transmission.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance);
private:
    /** @brief  Terrain type */
    string terrain;

    /** @brief  Transmitter Antenna High */
    double ht;

    /** @brief  Receiver Antenna High */
    double hr;

    /** @brief  Receiver Antenna Gain */
    double Gr;

    /** @brief  Transmitter Antenna Gain */
    double Gt;

};


#endif /* __SUIMODEL_H__ */
