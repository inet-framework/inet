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

#include <assert.h>
#include "inet/physicallayer/mapping/MappingBase.h"

namespace inet {
namespace physicallayer {
//---Dimension implementation-----------------------------

const Dimension Dimension::time("time", 0);
const Dimension Dimension::frequency("frequency", 1);
const Argument::mapped_type Argument::MappedZero = Argument::mapped_type(0);
const Argument::mapped_type Argument::MappedOne = Argument::mapped_type(1);

Dimension::Dimension(const char *name, int id) :
    name(name),
    id(id)
{
}

//--DimensionSet implementation ----------------------
const DimensionSet DimensionSet::timeDomain(Dimension::time);
const DimensionSet DimensionSet::freqDomain(Dimension::frequency);
const DimensionSet DimensionSet::timeFreqDomain(Dimension::time, Dimension::frequency);

//--Argument implementation---------------------------

Argument::Argument(simtime_t_cref timeVal) :
    time(timeVal), values()
{
}

Argument::Argument(const DimensionSet& dims, simtime_t_cref timeVal) :
    time(timeVal), values()
{
    DimensionSet::const_iterator it = dims.begin();
    const DimensionSet::const_iterator itEnd = dims.end();

    for (++it; it != itEnd; ++it) {
        values.insert(Argument::value_type(*it, Argument::MappedZero));
    }
}

simtime_t_cref Argument::getTime() const
{
    return time;
}

void Argument::setTime(simtime_t_cref time)
{
    this->time = time;
}

Argument::iterator Argument::find(const Argument::key_type& dim)
{
    assert(!(dim == Dimension::time));

    return values.find(dim);
}

Argument::const_iterator Argument::find(const Argument::key_type& dim) const
{
    assert(!(dim == Dimension::time));

    return values.find(dim);
}

Argument::iterator Argument::lower_bound(const Argument::key_type& dim)
{
    assert(!(dim == Dimension::time));

    return values.lower_bound(dim);
}

Argument::const_iterator Argument::lower_bound(const Argument::key_type& dim) const
{
    assert(!(dim == Dimension::time));

    return values.lower_bound(dim);
}

bool Argument::hasArgVal(const Argument::key_type& dim) const
{
    return values.find(dim) != values.end();
}

Argument::mapped_type_cref Argument::getArgValue(const Argument::key_type& dim) const
{
    const_iterator it = find(dim);

    if (it == values.end())
        return MappedZero;

    return it->second;
}

void Argument::setArgValue(const Argument::key_type& dim, Argument::mapped_type_cref value)
{
    assert(!(dim == Dimension::time));

    iterator pos = values.lower_bound(dim);
    if (pos != values.end() && !(values.key_comp()(dim, pos->first))) {
        // key already exists
        // update pos->second if you care to
        pos->second = value;
        return;
    }
    values.insert(pos, Argument::value_type(dim, value));
}

inline Argument::iterator Argument::insertValue(iterator pos, const Argument::value_type& valPair, iterator& itEnd, bool ignoreUnknown)
{
    pos = values.lower_bound(valPair.first);
    if (pos != itEnd && !(values.key_comp()(valPair.first, pos->first))) {
        // key already exists
        // update pos->second if you care to
        pos->second = valPair.second;
        return pos;
    }
    if (ignoreUnknown)
        return pos;

    pos = values.insert(pos, valPair);
    itEnd = values.end();
    return pos;
}

void Argument::setArgValues(const Argument& o, bool ignoreUnknown)
{
    time = o.time;

    iterator pos = begin();
    const const_iterator oEndIter = o.end();
    iterator EndIter = values.end();

    for (const_iterator it = o.begin(); it != oEndIter; ++it) {
        pos = insertValue(pos, *it, EndIter, ignoreUnknown);
        if (ignoreUnknown && pos == EndIter)
            break; //current dimension was not found and next will be also not in our dimension set
    }
}

bool Argument::isSamePosition(const Argument& o) const
{
    if (time != o.time) {
        return false;
    }

    if (values.size() < o.values.size()) {
        return false;
    }

    if (o.values.empty())
        return true;

    const_iterator itO = o.begin();
    const_iterator it = begin();
    const const_iterator itEnd = end();
    const const_iterator itEndO = o.end();

    while (it != itEnd) {
        if (itO->first < it->first) {
            break;
        }
        else if (it->first < itO->first)
            ++it;
        else {
            if (it->second != itO->second) {
                break;
            }
            ++it;
            ++itO;
        }
        if (itO == itEndO)
            return true;
    }

    return false;
}

bool Argument::isClose(const Argument& o, Argument::mapped_type_cref epsilon) const
{
    if (values.size() != o.values.size())
        return false;

    if (fabs(SIMTIME_DBL(time - o.time)) > epsilon)
        return false;

    const_iterator itO = o.begin();
    const const_iterator itEnd = end();
    const const_iterator itEndO = o.end();

    for (const_iterator it = begin();
         it != itEnd && itO != itEndO; ++it)
    {
        if (!(it->first == itO->first) || (fabs(it->second - itO->second) > epsilon)) {
            return false;
        }
        ++itO;
    }

    return true;
}

bool Argument::operator==(const Argument& o) const
{
    if (time != o.time || values.size() != o.values.size())
        return false;

    return compare(o) == 0;
}

Argument& Argument::operator=(const Argument& o)
{
    values = o.values;
    time = o.time;
    return *this;
}

bool Argument::operator<(const Argument& o) const
{
    assert(getDimensions() == o.getDimensions());

    return compare(o) < 0;
}

int Argument::compare(const Argument& o, const DimensionSet *const dims    /*= NULL*/) const
{
    DimensionSet::const_iterator DimItLast;
    if (dims != NULL)
        DimItLast = dims->end();
    const DimensionSet::const_iterator DimItEnd = DimItLast;

    const container_type::const_reverse_iterator rItEnd = values.rend();
    const container_type::const_iterator itEndO = o.values.end();
    container_type::const_reverse_iterator rIt;
    container_type::const_iterator itO;
    bool bDidCompare = false;

    //iterate through passed dimensions and compare arguments in these dimensions
    for (container_type::const_reverse_iterator rIt = values.rbegin(); rIt != rItEnd; ++rIt) {
        bool bMissedDimsEntry = false;

        if (dims != NULL) {
            DimensionSet::const_iterator DimItCurr = dims->find(rIt->first);
            if (DimItCurr == DimItEnd) {
                continue;
            }
            if (DimItLast != DimItEnd && (--DimItLast) != DimItEnd) {
                bMissedDimsEntry = (DimItLast != DimItCurr);    // missed something
            }
            DimItLast = DimItCurr;
        }
        bDidCompare = true;
        //catch special cases time, missing dimension values (after which we can abort)
        if (bMissedDimsEntry || (itO = o.values.find(rIt->first)) == itEndO) {
            if (time == o.time)
                return 0;
            return (time < o.time) ? -1 : 1;
        }

        //if both Arguments are defined in the current dimensions
        //compare them (otherwise we assume them equal and continue)
        if (rIt->second != itO->second)
            return (rIt->second < itO->second) ? -1 : 1;
    }
    if (dims == NULL || (dims->find(Dimension::time) != DimItEnd || (!bDidCompare && !dims->empty()))) {
        if (time == o.time)
            return 0;
        return (time < o.time) ? -1 : 1;
    }
    return 0;
}

//---Mapping implementation---------------------------------------

SimpleConstMappingIterator::SimpleConstMappingIterator(const ConstMapping *mapping,
        const SimpleConstMappingIterator::KeyEntrySet *keyEntries,
        const Argument& start)
    : ConstMappingIterator()
    , mapping(mapping)
    , dimensions(mapping->getDimensionSet())
    , position(mapping->getDimensionSet(), start.getTime())
    , keyEntries(keyEntries)
    , nextEntry()
{
    assert(keyEntries);

    //the passed start position should define a value for every dimension
    //of this iterators underlying mapping.
    assert(start.getDimensions().isSubSet(dimensions));

    //Since the position is compared to the key entries we have to make
    //sure it always contains only the dimensions of the underlying mapping.
    //(the passed Argument might have more dimensions)
    position.setArgValues(start, true);

    nextEntry = keyEntries->upper_bound(position);
}

SimpleConstMappingIterator::SimpleConstMappingIterator(const ConstMapping *mapping,
        const SimpleConstMappingIterator::KeyEntrySet *keyEntries)
    : ConstMappingIterator()
    , mapping(mapping)
    , dimensions(mapping->getDimensionSet())
    , position(dimensions)
    , keyEntries(keyEntries)
    , nextEntry()
{
    assert(keyEntries);

    jumpToBegin();
}

void SimpleConstMapping::createKeyEntries(const Argument& from, const Argument& to, const Argument& step, Argument& pos)
{
    //get iteration borders and steps
    simtime_t_cref fromT = from.getTime();
    simtime_t_cref toT = to.getTime();
    simtime_t_cref stepT = step.getTime();

    //iterate over interval without the end of the interval
    for (simtime_t t = fromT; t < toT; t += stepT) {
        //create key entry at current position
        pos.setTime(t);
        keyEntries.insert(pos);
    }

    //makes sure that the end of the interval becomes it own key entry
    pos.setTime(toT);
    keyEntries.insert(pos);
}

void SimpleConstMapping::createKeyEntries(const Argument& from, const Argument& to, const Argument& step,
        DimensionSet::const_iterator curDim, Argument& pos)
{
    //get the dimension to iterate over
    DimensionSet::const_reference d = *curDim;

    //increase iterator to next dimension (means curDim now stores the next dimension)
    --curDim;
    bool nextIsTime = (*curDim == Dimension::time);

    //get our iteration borders and steps
    argument_value_cref_t fromD = from.getArgValue(d);
    argument_value_cref_t toD = to.getArgValue(d);
    argument_value_cref_t stepD = step.getArgValue(d);

    //iterate over interval without the last entry
    for (argument_value_t i = fromD; i < toD; i += stepD) {
        pos.setArgValue(d, i);    //update position

        //call iteration over sub dimension
        if (nextIsTime) {
            createKeyEntries(from, to, step, pos);
        }
        else {
            createKeyEntries(from, to, step, curDim, pos);
        }
    }

    //makes sure that the end of the interval has its own key entry
    pos.setArgValue(d, toD);
    if (nextIsTime) {
        createKeyEntries(from, to, step, pos);
    }
    else {
        createKeyEntries(from, to, step, curDim, pos);
    }
}

void SimpleConstMapping::initializeArguments(const Argument& min,
        const Argument& max,
        const Argument& interval)
{
    keyEntries.clear();
    DimensionSet::const_iterator dimIt = --(dimensions.end());
    Argument pos = min;

    if (*dimIt == Dimension::time)
        createKeyEntries(min, max, interval, pos);
    else
        createKeyEntries(min, max, interval, dimIt, pos);
}
} // namespace physicallayer
} // namespace inet

