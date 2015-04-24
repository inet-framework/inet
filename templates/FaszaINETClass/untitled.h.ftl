<#assign hFileName=targetFileName?default("")?replace("\\.[a-z]*$",".h", "ri")>
<@setoutput path=hFileName/>
${bannerComment}

#ifndef __${PROJECTNAME}_${targetTypeName?upper_case}_H
#define __${PROJECTNAME}_${targetTypeName?upper_case}_H

#include "inet/common/INETDefs.h"

<#list "${namespaceName}"?split("::") as namespace>
namespace ${namespace} {
</#list>

class INET_API ${targetTypeName}
{
};

<#list "${namespaceName}"?split("::") as namespace>
} /* namespace ${namespace} */
</#list>

#endif // ifndef __${PROJECTNAME}_${targetTypeName?upper_case}_H
