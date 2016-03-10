//
// Copyright (C) 2015 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

#ifndef __INET_TLVOPTION_H_
#define __INET_TLVOPTION_H_

#include "TLVOption_m.h"

namespace inet {

class INET_API TLVOptions : public TLVOptions_Base
{
  public:
    typedef std::vector<TLVOptionBase*> TLVOptionVector;

  private:
    TLVOptionVector optionVector;
    void copy(const TLVOptions& other);

  public:
    TLVOptions() : TLVOptions_Base() {}
    ~TLVOptions() { clear(); }
    TLVOptions(const TLVOptions& other) : TLVOptions_Base(other) { copy(other); }
    TLVOptions& operator=(const TLVOptions& other) { if (this != &other) { TLVOptions_Base::operator=(other); clear(); copy(other); } return *this; }
    virtual TLVOptions *dup() const override { return new TLVOptions(*this); }

    int size() const { return optionVector.size(); }

    /**
     * Makes the container empty. Contained objects will be deleted.
     */
    void clear();

    /*
     * Insert the option at the end of optionVector. The inserted option will be deleted by this class.
     */
    void add(TLVOptionBase *option, int atPos = -1);

    /*
     * Removes option from optionVector and returns option when removed, otherwise returns nullptr.
     */
    TLVOptionBase *remove(TLVOptionBase *option);

    /*
     * Removes all options or first only option where type is the specified type.
     */
    void deleteOptionByType(int type, bool firstOnly = true);

    /*
     * Get the option at the specified position of optionVector. Throws an error if m invalid.
     */
    TLVOptionBase& at(int m) { return *optionVector.at(m); }

    /*
     * Get the option at the specified position of optionVector. Throws an error if m invalid.
     */
    TLVOptionBase& operator[](int m)  { return at(m); }

    /*
     * Calculate and returns the total length of all stored options in bytes
     */
    virtual int getLength() const;

    /*
     * Find the first option with specified type. Search started with element at firstPos.
     * Returns the position of found option, or return -1 if not found
     */
    int findByType(short int type, int firstPos=0) const;

    // redefine and implement pure virtual functions from TLVOptions_Base
    virtual void setTlvOptionArraySize(unsigned int size) override { throw cRuntimeError("Do not use it!"); }
    virtual unsigned int getTlvOptionArraySize() const override { return size(); }
    virtual TLVOptionBase& getTlvOption(unsigned int k) override { return at(k); }
    virtual void setTlvOption(unsigned int k, const TLVOptionBase& tlvOption) override { throw cRuntimeError("Do not use it!"); }

    virtual void parsimPack(cCommBuffer *b) PARSIMPACK_CONST override;
    virtual void parsimUnpack(cCommBuffer *b) override;
};

} // namespace inet

#endif // __INET_TLVOPTION_H_

