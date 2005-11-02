//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __CLASSIFIER_H__
#define __CLASSIFIER_H__

#include <omnetpp.h>

#include "IPDatagram.h"
#include "LIBtable.h"

/**
 * FIXME missing documentation
 */
class INET_API IClassifier
{
  public:
    virtual ~IClassifier() {}

    virtual bool lookupLabel(IPDatagram *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color) = 0;
};

#endif
