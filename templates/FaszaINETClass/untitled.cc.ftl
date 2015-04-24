<#assign hFileName=targetFileName?default("")?replace("\\.[a-z]*$",".h", "ri")>
<#assign ccFileName=targetFileName?default("")?replace("\\.[a-z]*$",".cc", "ri")>
<@setoutput path=ccFileName/>
${bannerComment}

#include "${hFileName}"

<#list "${namespaceName}"?split("::") as namespace>
namespace ${namespace} {
</#list>


<#list "${namespaceName}"?split("::") as namespace>
} /* namespace ${namespace} */
</#list>

