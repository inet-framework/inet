//
// (C) 2005 Vojtech Janota
// (C) 2010 Zoltan Bojthe
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

#include <iostream>

#include "inet/common/fingerprint/PrinterTestFingerprintCalculator.h"
#include "inet/common/packet/printer/PacketPrinter.h"

namespace inet {

Register_Class(PrinterTestFingerprintCalculator);

void PrinterTestFingerprintCalculator::addEvent(cEvent *event)
{
    if (event->isMessage()) {
        cMessage *message = static_cast<cMessage *>(event);
        PacketPrinter().printMessage(std::cout, message);
    }
    cSingleFingerprintCalculator::addEvent(event);
}

} // namespace inet

