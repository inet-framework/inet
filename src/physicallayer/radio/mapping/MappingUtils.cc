/*
 * MappingUtils.cc
 *
 *  Created on: 26.08.2008
 *      Author: Karl Wessel
 */

#include "MappingUtils.h"


FilledUpMappingIterator::FilledUpMappingIterator(FilledUpMapping& mapping):
	MultiDimMappingIterator<Linear>(mapping) {}

FilledUpMappingIterator::FilledUpMappingIterator(FilledUpMapping& mapping, const Argument& pos):
	MultiDimMappingIterator<Linear>(mapping, pos) {}

const Argument::mapped_type MappingUtils::cMinNotFound =  std::numeric_limits<Argument::mapped_type>::infinity();
const Argument::mapped_type MappingUtils::cMaxNotFound = -std::numeric_limits<Argument::mapped_type>::infinity();

const ConstMapping* MappingUtils::createCompatibleMapping(const ConstMapping& src, const ConstMapping& dst){
	typedef FilledUpMapping::KeySet KeySet;
	typedef FilledUpMapping::KeyMap KeyMap;

	KeyMap                                     DimensionIndizes;

	const DimensionSet&                        srcDims    = src.getDimensionSet();
	const DimensionSet&                        dstDims    = dst.getDimensionSet();

	const DimensionSet::const_iterator         srcDimsEnd = srcDims.end();
	const DimensionSet::const_reverse_iterator dstDimsEnd = dstDims.rend();
	for (DimensionSet::const_reverse_iterator  dstDimIt   = dstDims.rbegin(); dstDimIt != dstDimsEnd; ++dstDimIt) {
		if(srcDims.find(*dstDimIt) == srcDimsEnd) {
			DimensionIndizes.insert(DimensionIndizes.end(), KeyMap::value_type(*dstDimIt, KeySet()));
		}
	}

	if(DimensionIndizes.empty())
		return &src;

	ConstMappingIterator* dstIt = dst.createConstIterator();

	if(!dstIt->inRange()){
		delete dstIt;
		return &src;
	}

	const KeyMap::iterator keysEnd = DimensionIndizes.end();
	do {
		for (KeyMap::iterator keyDimIt = DimensionIndizes.begin(); keyDimIt != keysEnd; ++keyDimIt) {
			keyDimIt->second.insert(dstIt->getPosition().getArgValue(keyDimIt->first));
		}

		if(!dstIt->hasNext())
			break;
		dstIt->next();
	} while(true);

	delete dstIt;

	return new FilledUpMapping(&src, dstDims, DimensionIndizes);
}

bool MappingUtils::iterateToNext(ConstMappingIterator* it1, ConstMappingIterator* it2){
	bool it1HasNext = it1->hasNext();
	bool it2HasNext = it2->hasNext();

	if(it1HasNext || it2HasNext){
		if(it1HasNext && (!it2HasNext || it1->getNextPosition() < it2->getNextPosition())){
			it1->next();
			it2->iterateTo(it1->getPosition());

		} else {
			it2->next();
			it1->iterateTo(it2->getPosition());
		}

		return true;
	} else {
		return false;
	}
}

Mapping* MappingUtils::createMapping(const DimensionSet& domain, Mapping::InterpolationMethod intpl) {
	assert(domain.hasDimension(Dimension::time));

	if(domain.size() == 1){
		switch(intpl){
		case Mapping::LINEAR:
			return new TimeMapping<Linear>();
			break;
		case Mapping::NEAREST:
			return new TimeMapping<Nearest>();
			break;
		case Mapping::STEPS:
			return new TimeMapping<NextSmaller>();
			break;
		}
		return 0;
	} else {
		switch(intpl){
		case Mapping::LINEAR:
			return new MultiDimMapping<Linear>(domain);
			break;
		case Mapping::NEAREST:
			return new MultiDimMapping<Nearest>(domain);
			break;
		case Mapping::STEPS:
			return new MultiDimMapping<NextSmaller>(domain);
			break;
		}
		return 0;
	}
}

Mapping* MappingUtils::createMapping(Mapping::argument_value_cref_t outOfRangeVal, const DimensionSet& domain, Mapping::InterpolationMethod intpl) {
	assert(domain.hasDimension(Dimension::time));

	if(domain.size() == 1){
		switch(intpl){
		case Mapping::LINEAR:
			return new TimeMapping<Linear>(outOfRangeVal);
			break;
		case Mapping::NEAREST:
			return new TimeMapping<Nearest>(outOfRangeVal);
			break;
		case Mapping::STEPS:
			return new TimeMapping<NextSmaller>(outOfRangeVal);
			break;
		}
		return 0;
	} else {
		switch(intpl){
		case Mapping::LINEAR:
			return new MultiDimMapping<Linear>(domain, outOfRangeVal);
			break;
		case Mapping::NEAREST:
			return new MultiDimMapping<Nearest>(domain, outOfRangeVal);
			break;
		case Mapping::STEPS:
			return new MultiDimMapping<NextSmaller>(domain, outOfRangeVal);
			break;
		}
		return 0;
	}
}

Mapping* MappingUtils::multiply(const ConstMapping &f1, const ConstMapping &f2)
{
	return applyElementWiseOperator(f1, f2, std::multiplies<Mapping::argument_value_t>());
}

Mapping* MappingUtils::divide(const ConstMapping &f1, const ConstMapping &f2)
{
	return applyElementWiseOperator(f1, f2, std::divides<Mapping::argument_value_t>());
}

Mapping* MappingUtils::add(const ConstMapping &f1, const ConstMapping &f2)
{
	return applyElementWiseOperator(f1, f2, std::plus<Mapping::argument_value_t>());
}

Mapping* MappingUtils::subtract(const ConstMapping &f1, const ConstMapping &f2)
{
	return applyElementWiseOperator(f1, f2, std::minus<Mapping::argument_value_t>());
}


Mapping* MappingUtils::multiply(const ConstMapping &f1, const ConstMapping &f2, Mapping::argument_value_cref_t outOfRangeVal)
{
	return applyElementWiseOperator(f1, f2, std::multiplies<Mapping::argument_value_t>(), outOfRangeVal, false);
}

Mapping* MappingUtils::divide(const ConstMapping &f1, const ConstMapping &f2, Mapping::argument_value_cref_t outOfRangeVal)
{
	return applyElementWiseOperator(f1, f2, std::divides<Mapping::argument_value_t>(), outOfRangeVal, false);
}

Mapping* MappingUtils::add(const ConstMapping &f1, const ConstMapping &f2, Mapping::argument_value_cref_t outOfRangeVal)
{
	return applyElementWiseOperator(f1, f2, std::plus<Mapping::argument_value_t>(), outOfRangeVal, false);
}

Mapping* MappingUtils::subtract(const ConstMapping &f1, const ConstMapping &f2, Mapping::argument_value_cref_t outOfRangeVal)
{
	return applyElementWiseOperator(f1, f2, std::minus<Mapping::argument_value_t>(), outOfRangeVal, false);
}


Mapping* operator*(const ConstMapping& f1, const ConstMapping& f2) {
	return MappingUtils::multiply(f1, f2);
}

Mapping* operator/(const ConstMapping& f1, const ConstMapping& f2) {
	return MappingUtils::divide(f1, f2);
}

Mapping* operator+(const ConstMapping& f1, const ConstMapping& f2) {
	return MappingUtils::add(f1, f2);
}

Mapping* operator-(const ConstMapping& f1,const  ConstMapping& f2) {
	return MappingUtils::subtract(f1, f2);
}


Mapping::argument_value_t MappingUtils::findMax(const ConstMapping& m, Argument::mapped_type_cref cRetNotFound /*= cMaxNotFound*/) {
	ConstMappingIterator*     it       = m.createConstIterator();
	bool                      bIsFirst = true;
	Mapping::argument_value_t res;

	while(it->inRange()){
		Mapping::argument_value_cref_t val = it->getValue();
		if(bIsFirst || val > res) {
			res      = val;
			bIsFirst = false;
		}

		//std::cerr << "findMax(): " << val << " @ " << it->getPosition() << "; max is now: " << res << std::endl;
		if(!it->hasNext())
			break;

		it->next();
	}
	delete it;
	if (bIsFirst) {
		// no maximum available, maybe map is empty
		return cRetNotFound;
	}
	return res;
}

Mapping::argument_value_t MappingUtils::findMax(const ConstMapping& m, const Argument& pRangeFrom, const Argument& pRangeTo, Argument::mapped_type_cref cRetNotFound /*= cMaxNotFound*/){
	const DimensionSet& rDimSet = m.getDimensionSet();
	//the passed interval should define a value for every dimension
	//of the mapping.
	assert(pRangeFrom.getDimensions().isSubSet(rDimSet));
	assert(pRangeTo.getDimensions().isSubSet(rDimSet));

	ConstMappingIterator*     it       = m.createConstIterator(pRangeFrom);
	bool                      bIsFirst = true;
	Mapping::argument_value_t res;

	//std::cerr << "findMax(m, " << pRangeFrom << ", " << pRangeTo << "): Map is" << std::endl << m;
	if (it->inRange()) {
		res      = it->getValue();
		bIsFirst = false;
		//std::cerr << "findMax(...):  " << " @ " << it->getPosition() << "; max is at beginning: " << res << std::endl;
	}
	while(it->hasNext() && it->getNextPosition().compare(pRangeTo, &rDimSet) < 0){
		it->next();

		const Argument& next    = it->getPosition();
		bool            inRange = pRangeFrom.getTime() <= next.getTime() && next.getTime() <= pRangeTo.getTime();
		if(inRange) {
			const Argument::const_iterator itAEnd = next.end();
			for(Argument::const_iterator itA = next.begin(); itA != itAEnd; ++itA) {
				if(pRangeFrom.getArgValue(itA->first) > itA->second || itA->second > pRangeTo.getArgValue(itA->first)) {
					inRange = false;
					break;
				}
			}
		}
		if(inRange) {
			Mapping::argument_value_cref_t val = it->getValue();
			if(bIsFirst || val > res) {
				res      = val;
				bIsFirst = false;
			}
			//std::cerr << "findMax(...): " << val << " @ " << it->getPosition() << "; max is now: " << res << std::endl;
		}
	}
	it->iterateTo(pRangeTo);
	if (it->inRange()) {
		Mapping::argument_value_cref_t val = it->getValue();
		if(bIsFirst || val > res) {
			res      = val;
			bIsFirst = false;
		}
		//std::cerr << "findMax(...): " << val << " @ " << it->getPosition() << "; max is finally: " << res << std::endl;
	}
	delete it;
	if (bIsFirst) {
		// no minimum available
		return cRetNotFound;
	}
	return res;
}

Mapping::argument_value_t MappingUtils::findMin(const ConstMapping& m, Argument::mapped_type_cref cRetNotFound /*= cMinNotFound*/) {
	ConstMappingIterator*     it       = m.createConstIterator();
	bool                      bIsFirst = true;
	Mapping::argument_value_t res;

	while(it->inRange()) {
		Mapping::argument_value_cref_t val = it->getValue();
		if(bIsFirst || val < res) {
			res      = val;
			bIsFirst = false;
		}

		//std::cerr << "findMin(): " << val << " @ " << it->getPosition() << "; min is now: " << res << std::endl;
		if(!it->hasNext())
			break;

		it->next();
	}
	delete it;
	if (bIsFirst) {
		// no minimum available, maybe map is empty
		return cRetNotFound;
	}
	return res;
}

Mapping::argument_value_t MappingUtils::findMin(const ConstMapping& m, const Argument& pRangeFrom, const Argument& pRangeTo, Argument::mapped_type_cref cRetNotFound /*= cMinNotFound*/) {
	const DimensionSet& rDimSet = m.getDimensionSet();
	//the passed interval should define a value for every dimension
	//of the mapping.
	assert(pRangeFrom.getDimensions().isSubSet(rDimSet));
	assert(pRangeTo.getDimensions().isSubSet(rDimSet));

	Mapping::argument_value_t res;
	bool                      bIsFirst = true;
	ConstMappingIterator*     it       = m.createConstIterator(pRangeFrom);

	//std::cerr << "findMin(m, " << pRangeFrom << ", " << pRangeTo << "): Map is" << std::endl << m;
	if (it->inRange()) {
		res      = it->getValue();
		bIsFirst = false;
		//std::cerr << "findMin(...):  " << " @ " << it->getPosition() << "; min is at beginning: " << res << std::endl;
	}
	while(it->hasNext() && it->getNextPosition().compare(pRangeTo, &rDimSet) < 0) {
		it->next();

		const Argument& next    = it->getPosition();
		bool            inRange = pRangeFrom.getTime() <= next.getTime()  && next.getTime() <= pRangeTo.getTime();
		if(inRange) {
			const Argument::const_iterator itAEnd = next.end();
			for(Argument::const_iterator itA = next.begin(); itA != itAEnd; ++itA) {
				if(pRangeFrom.getArgValue(itA->first) > itA->second || itA->second > pRangeTo.getArgValue(itA->first)) {
					inRange = false;
					break;
				}
			}
		}
		if(inRange) {
			Mapping::argument_value_cref_t val = it->getValue();
			if(bIsFirst || val < res) {
				res      = val;
				bIsFirst = false;
			}
			//std::cerr << "findMin(...): " << val << " @ " << it->getPosition() << "; min is now: " << res << std::endl;
		}
	}
	it->iterateTo(pRangeTo);
	if (it->inRange()) {
		Mapping::argument_value_cref_t val = it->getValue();
		if(bIsFirst || val < res) {
			res      = val;
			bIsFirst = false;
		}
		//std::cerr << "findMin(...): " << val << " @ " << it->getPosition() << "; min is finally: " << res << std::endl;
	}
	delete it;
	if (bIsFirst) {
		// no minimum available
		return cRetNotFound;
	}
	return res;
}


void MappingUtils::addDiscontinuity(Mapping* m,
                                    const Argument& pos, Mapping::argument_value_cref_t value,
                                    simtime_t_cref limitTime, Mapping::argument_value_cref_t limitValue)
{
	// asserts/preconditions
	// make sure the time really differs at the discontinuity
	assert(limitTime != pos.getTime());

	// add (pos, value) to mapping
	m->setValue(pos, value);

	// create Argument limitPos for the limit-position, i.e. copy pos and set limitTime as its time
	Argument limitPos = pos;
	limitPos.setTime(limitTime);

	// add (limitPos, limitValue) to mapping
	m->setValue(limitPos, limitValue);
}

simtime_t MappingUtils::pre(simtime_t_cref t)
{
	assert(SIMTIME_RAW(t) > SIMTIME_RAW(SIMTIME_ZERO));

	simtime_t stPre = SIMTIME_ZERO;
	stPre.setRaw(SIMTIME_RAW(t) - 1);

	return stPre;
}

simtime_t MappingUtils::post(simtime_t_cref t)
{
	assert(SIMTIME_RAW(t) < SIMTIME_RAW(MAXTIME));

	simtime_t stPost = SIMTIME_ZERO;
	stPost.setRaw(SIMTIME_RAW(t) + 1);

	return stPost;
}

simtime_t MappingUtils::incNextPosition(simtime_t_cref t)
{
	//assert(SIMTIME_RAW(t) < SIMTIME_RAW(MAXTIME));
	// original it was following formula, but I do not know why
	// the '+1' is used here!? I think post should be enough!
	//return t + 1;
	return post(t);
}


/*
Mapping* Mapping::multiply(ConstMapping &f1, ConstMapping &f2, const Argument& from, const Argument& to)
{
	return applyElementWiseOperator(f1, f2, std::multiplies<double>());
}

Mapping* Mapping::divide(ConstMapping &f1, ConstMapping &f2, const Argument& from, const Argument& to)
{
	return applyElementWiseOperator(f1, f2, std::divides<double>());
}

Mapping* Mapping::add(ConstMapping &f1, ConstMapping &f2, const Argument& from, const Argument& to)
{
	return applyElementWiseOperator(f1, f2, std::plus<double>());
}

Mapping* Mapping::subtract(ConstMapping &f1, ConstMapping &f2, const Argument& from, const Argument& to)
{
	return applyElementWiseOperator(f1, f2, std::minus<double>());
}
*/


LinearIntplMappingIterator::LinearIntplMappingIterator(ConstMappingIterator* leftIt, ConstMappingIterator* rightIt, Mapping::argument_value_cref_t f):
	MappingIterator(), leftIt(leftIt), rightIt(rightIt), factor(f) {

	assert(leftIt->getPosition() == rightIt->getPosition());
}

LinearIntplMappingIterator::~LinearIntplMappingIterator() {
	if(leftIt)
		delete leftIt;
	if(rightIt)
		delete rightIt;
}
