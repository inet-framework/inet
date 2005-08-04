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
   @file opp_utils.cc
   @brief Global utility functions
   @author Johnny Lai
 */


#include "sys.h"
#include "debug.h"


#include <cstring>
#include <cmath> //std::pow
//#include <boost/random.hpp>

#include <omnetpp.h>

#include "opp_utils.h"
//#include "IPv6Datagram.h"
//#include "WorldProcessor.h"

namespace OPP_Global
{

  ContextSwitcher::ContextSwitcher(const cModule* to):mod(simulation.contextModule())
  {
    simulation.setContextModule(const_cast<cModule*>(to));
  }

  ContextSwitcher::~ContextSwitcher()
  {
    simulation.setContextModule(mod);
  }

  ///Goes into all submodules looking for one with name instanceName
  cModule* findModuleByName(cModule* curmod, const char* instanceName)
  {
#if !defined __INTEL_COMPILER
//   Dout(dc::debug, "Called from " <<
//        location_ct((char*)__builtin_return_address(0) + libcw::debug::builtin_return_address_offset));
//   Dout(dc::debug|flush_cf, curmod->fullPath()<<" looking for name "<<instanceName);
#endif //!defined __INTEL_COMPILER
    cObject* retModule = 0;
    for (curmod = curmod->parentModule(); curmod != NULL;
         curmod = curmod->parentModule())
    {
#if !defined __INTEL_COMPILER
//      Dout(dc::debug|flush_cf, curmod->name()<<"-");
#endif //!defined __INTEL_COMPILER

      if ((retModule = static_cast<cModule*> (curmod->findObject(instanceName, true))) != NULL)
        return static_cast<cModule*> (retModule);
    }
    return 0;
  }


  cModule* iterateSubMod(cModule* curmod, const char* className, bool  enterCompound)
  {
    cModule* retModule = 0;

    for (cSubModIterator submod(*curmod); !submod.end(); submod++)
    {
      retModule = static_cast<cModule*> (submod()) ;
      //Dout(dc::debug|flush_cf, retModule->className()<<"|");
      if (strcmp(className, retModule -> className()) == 0)
      {
        return retModule;
      }
      if ( enterCompound && !retModule->isSimple())
      {
        retModule = iterateSubMod(retModule, className, enterCompound);
        if (retModule)
          return retModule;
      }
    }
    return 0;
  }

  /**
     Breadth first traversal.  Will not go inside compound modules.
  */
  cModule* findModuleByType(cModule* curmod, const char* className)
  {
#if !defined __INTEL_COMPILER
    //Dout(dc::debug|flush_cf, " Looking for class="<<className);
#endif //!defined __INTEL_COMPILER
    cModule* retModule = 0;

    for (curmod = curmod->parentModule(); curmod != NULL;
         curmod = curmod->parentModule())
    {
      retModule = iterateSubMod(curmod, className);
      if (retModule != 0)
        return retModule;
    }
    return 0;
  }

  ///Depth first traversal on parent of curmod. Goes inside compound modules
  ///first.
  cModule* findModuleByTypeDepthFirst(cModule* curmod, const char* className)
  {
    cModule* retModule = 0;

    for (curmod = curmod->parentModule(); curmod != NULL;
         curmod = curmod->parentModule())
    {
      retModule = iterateSubMod(curmod, className, true);
      if (retModule != 0)
        return retModule;
    }
    return 0;
  }

  const cModule* findNetNodeModule(const cModule* curmod)
  {
    const cModule* retModule = 0;

    for (curmod = curmod->parentModule();;
         curmod = curmod->parentModule())
    {
      if (curmod->parentModule() == NULL)
        return retModule;
      retModule = curmod;
    }
  }

  cModule* findNetNodeModule(cModule* curmod)
  {
    cModule* retModule = 0;

    for (curmod = curmod->parentModule();;
         curmod = curmod->parentModule())
    {
      if (curmod->parentModule() == NULL)
        return retModule;
      retModule = curmod;
    }
  }

  std::string ltostr(long i)
  {
    std::ostringstream os;
    os << i;
    return os.str();
  }

  std::string dtostr(double d)
  {
    std::ostringstream os;
    os << d;
    return os.str();
  }

  double atod(const char *s)
  {
    char *e;
    double d = ::strtod(s,&e);
    if (*e)
        throw new cException("invalid cast: '%s' cannot be interpreted as a double", s);
    return d;
  }

  unsigned long atoul(const char *s)
  {
    char *e;
    unsigned long d = ::strtoul(s,&e,10);
    if (*e)
        throw new cException("invalid cast: '%s' cannot be interpreted as an unsigned long", s);
    return d;
  }

  void stackUsage(cModule* self, std::ostream& os)
  {
            //Relative to the actual Network
    const cModule* tcpClientMod = findNetNodeModule(self);

      //simulation.moduleByPath("client1");
    int mod_count = 0;

    int sum = sumChildModules(tcpClientMod, mod_count);

    os <<"Total stack size is " << sum <<"\n";
    os <<"Total no. of simple Modules is " << mod_count <<"\n";
  }

  int sumChildModules(const cModule* parent, int& modCount)
  {
    int sum = 0;

    for (cSubModIterator submod(*parent); !submod.end(); submod++)
    {

      cSimpleModule* simpleMod = NULL;
      if ((simpleMod = dynamic_cast<cSimpleModule*> (submod())) != NULL)
      {
        std::cerr << submod()->className()<< " stack usage: "
                  << simpleMod->stackUsage()<<std::endl;
        sum += simpleMod->stackUsage();
        modCount++;
      }
      else
        sum += sumChildModules(submod(), modCount);
    }
    return sum;
  }

  unsigned int generateInterfaceId()
  {
    // generate 32-bit random Id
    return (intuniform(0,0xffff)<<16) | intuniform(0,0xffff);
  }

  const char* nodeName(const cModule* callingMod)
  {
    static char unknown[] = "UNKNOWN!";

    cModule* network = simulation.systemModule();
    for (cSubModIterator submod(*network); !submod.end(); submod++)
    {
      if (OPP_Global::findNetNodeModule(callingMod) == submod())
        return submod()->name();
    }

    return unknown;
  }

/*FIXME temporarily disabled
  XMLConfiguration::XMLOmnetParser* getParser()
  {
    cModule *wp = simulation.moduleByPath("worldProcessor");
    return check_and_cast<WorldProcessor*>(wp)->xmlConfig();
  }
*/
}
