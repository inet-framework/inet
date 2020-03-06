//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/common/bitlevel/PulseFilter.h"

namespace inet {
namespace physicallayer {

PulseFilter::PulseFilter() :
    samplePerSymbol(-1)
{}

const IReceptionSymbolModel *PulseFilter::filter(const IReceptionSampleModel *sampleModel) const
{
    int headerSymbolLength = sampleModel->getHeaderSampleLength() / samplePerSymbol;
    double headerSymbolRate = sampleModel->getHeaderSampleRate() / samplePerSymbol;
    int dataSymbolLength = sampleModel->getDataSampleLength() / samplePerSymbol;
    double dataSymbolRate = sampleModel->getDataSampleRate() / samplePerSymbol;
    return new ReceptionSymbolModel(headerSymbolLength, headerSymbolRate, dataSymbolLength, dataSymbolRate, nullptr);
}

} // namespace physicallayer
} // namespace inet

