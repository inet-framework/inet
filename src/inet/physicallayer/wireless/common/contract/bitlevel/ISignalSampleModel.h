//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISIGNALSAMPLEMODEL_H
#define __INET_ISIGNALSAMPLEMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Units.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the waveform or sample domain.
 */
class INET_API ISignalSampleModel : public IPrintableObject
{
  public:
    virtual int getHeaderSampleLength() const = 0;

    virtual double getHeaderSampleRate() const = 0;

    virtual int getDataSampleLength() const = 0;

    virtual double getDataSampleRate() const = 0;

    virtual const std::vector<W> *getSamples() const = 0;
};

class INET_API ITransmissionSampleModel : public virtual ISignalSampleModel
{
};

class INET_API IReceptionSampleModel : public virtual ISignalSampleModel
{
};

} // namespace physicallayer

} // namespace inet

#endif

