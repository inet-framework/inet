/* -*- mode:c++ -*- ********************************************************
 * file:        SimpleTimeConstMapping.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: This class implements a simple time-based constant mapping
 ***************************************************************************/

#ifndef _SIMPLETIMECONSTMAPPING_H
#define _SIMPLETIMECONSTMAPPING_H

#include "MiXiMDefs.h"
#include "Mapping.h"

/**
 * @brief TODO: short description for this class
 *
 * @ingroup mapping
 */
class MIXIM_API SimpleTimeConstMapping: public SimpleConstMapping {

protected:
	double myValue;
	simtime_t start, stop;
public:
	/** @brief Constructs the mapping with the value _myValue, in the
	 * time interval [start, stop].
	 */
	SimpleTimeConstMapping(double _myValue, simtime_t_cref _start, simtime_t_cref _stop)
		: SimpleConstMapping(DimensionSet(Dimension::time))
		, myValue(_myValue)
		, start(_start)
		, stop(_stop)
	{
		initializeArguments(Argument(_start), Argument(_stop), Argument(1));
	}
	virtual ~SimpleTimeConstMapping() {}

	/**
	 * @brief Returns a constant.
	 * */
	double getValue(const Argument& /*pos*/) const { return myValue; }

	/**
	 * @brief creates a clone of this mapping.
	 */
	ConstMapping* constClone() const {
		return new SimpleTimeConstMapping(myValue, start, stop); }
};

#endif
