//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel.errormodel;

import inet.physicallayer.wireless.common.base.bitlevel.LayeredErrorModelBase;

//
// This error model computes the erroneous bits, symbols, or simply whether the
// packet is error free or not based on the SNIR.
//
module ApskLayeredErrorModel extends LayeredErrorModelBase
{
    parameters:
        @class(ApskLayeredErrorModel);
}
