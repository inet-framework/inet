//
// Copyright (C) 2011 Alfonso Ariza Quintana
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef INOISEGENERATOR_H_
#define INOISEGENERATOR_H_

class INoiseGenerator  : public cPolymorphic {
public:
    /**
     * Allows parameters to be read from the module parameters of a
     * module that contains this object.
     */
    virtual void initializeFrom(cModule *radioModule) = 0;
	virtual double noiseLevel() {return 0;}
};

#endif /* INOISEGENERATOR_H_ */
