// -*- C++ -*-
//
// Copyright (C) 2001, 2004 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/**
   @file opp_utils.h
   @brief Global utility functions
   @author Johnny Lai
 */

#ifndef OPP_UTILS_H
#define OPP_UTILS_H

#include <memory> //std::auto_ptr
#include <string>
#include <omnetpp.h>
#include "INETDefs.h"


// XXX dirty debugging hack, to be thrown out when cerr<< stuff
// gets properly done with exceptions --AV
inline void abort_ipv6suite()
{
#ifdef _WIN32
  __asm int 3;
#else
  exit(1);
#endif
}

class cModule;
class IPv6Datagram;
namespace XMLConfiguration
{
  class XMLOmnetParser;
}

namespace OPP_Global
{

  /**
     @class ContextSwitcher
     @brief Switch running module context
     XXX EVIL -- causes obscurity, to be eliminated!!!!! --AV
  */

  class ContextSwitcher
  {
  public:

    // Constructor/destructor.
    ContextSwitcher(const cModule* to);
    ~ContextSwitcher();

  private:

    // Unused ctor and assignment op.
    ContextSwitcher(const ContextSwitcher&);
    ContextSwitcher& operator=(const ContextSwitcher&);
    cModule* mod;
  };


  cModule* findModuleByName(cModule* curmod, const char* instanceName);

  ///Can only find simple modules by class name
  cModule* findModuleByType(cModule* curmod, const char* className);

  ///Same as findModuleByType except it searches inside compound modules too
  cModule* findModuleByTypeDepthFirst(cModule* curmod, const char* className);

  ///Looks at contained modules and returns the first matching simpleModule with
  ///className.  If enterCompound is set then compound modules are also
  ///searched.
  cModule* iterateSubMod(cModule* curmod, const char* className,
                         bool enterCompound = false);

  ///Retrieve a pointer to the network level module which contains the current
  ///module
  const cModule* findNetNodeModule(const cModule* curmod);
  cModule* findNetNodeModule(cModule* curmod);


  /**
     Displays the stack usage for the current network node module which contains
     self.
  */
  void stackUsage(cModule* self, std::ostream& os);

  /**
     Recursive function to return the sum of all child modules with reference to
     parent.  Accessory to stackUsage
  */
  int sumChildModules(const cModule* parent, int& modCount);

  unsigned int generateInterfaceId();

  /**
     Given a pointer to the calling module return the name of this network node.
   */
  const char* nodeName(const cModule* callingMod);

  /**
     Converts an integer to string.
   */
  std::string ltostr(long i);          //XXX make an ultostr as well, to be consistent with atoul

  /**
     Converts a double to string
   */
  std::string dtostr(double d);

  /**
     Converts string to double
   */
  double atod(const char *s);

  /**
     Converts string to unsigned long
   */
  unsigned long atoul(const char *s);

  ///downcast (convert down the class hierarchy) for auto_ptrs the source relinquishes
  ///ownership to the returned auto_ptr
  template<class Target, class  Source> std::auto_ptr<Target>
  auto_downcast(std::auto_ptr<Source> & r)
  {
    check_and_cast<Target*> (r.get());
    return std::auto_ptr<Target>(static_cast<Target*>(r.release()));
  }

/*FIXME temporarily disabled
  ///Returns the omnet++ parser for use by other classes to parse their own attributes
  XMLConfiguration::XMLOmnetParser* getParser();
*/
}

#endif

