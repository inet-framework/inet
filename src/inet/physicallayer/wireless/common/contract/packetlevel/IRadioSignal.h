//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRADIOSIGNAL_H
#define __INET_IRADIOSIGNAL_H

#include "inet/common/IPrintableObject.h"

namespace inet {
namespace physicallayer {

class INET_API IRadioSignal: public virtual IPrintableObject
{
  public:
    /**
     * This enumeration specifies a part of a radio signal.
     */
    enum SignalPart {
        SIGNAL_PART_NONE = -1,
        SIGNAL_PART_WHOLE,
        SIGNAL_PART_PREAMBLE,
        SIGNAL_PART_HEADER,
        SIGNAL_PART_DATA
    };

  protected:
    /**
     * The enumeration registered for signal part.
     */
    static cEnum *signalPartEnum;

  public:
    /**
     * Returns the name of the provided signal part.
     */
    static const char *getSignalPartName(SignalPart signalPart);
};

} // namespace physicallayer
} // namespace inet

#endif

