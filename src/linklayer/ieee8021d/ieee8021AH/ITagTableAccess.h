 /**
******************************************************
* @file ITagTableAccess.h
* @brief ITagTable access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __ITAGTABLE_ACCESS_H
#define __ITAGTABLE_ACCESS_H



#include "ModuleAccess.h"
#include "ITagTable.h"


/**
 * @brief Gives access to the ITagTable
 */
class ITagTableAccess : public ModuleAccess<ITagTable>
{
    public:
        ITagTableAccess() : ModuleAccess<ITagTable>("itagtable") {}
};

#endif
