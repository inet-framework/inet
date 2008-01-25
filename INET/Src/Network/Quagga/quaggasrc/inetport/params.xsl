<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:output method="text" indent="yes" encoding="iso-8859-1"/>

<xsl:key name='findvar' match="//compounddef[@kind='file']/sectiondef[@kind='var']/memberdef[@kind='variable']" use="name/text()"/> 

<xsl:variable name="defuns" select="//compounddef[@kind='file']/sectiondef[@kind='func']/memberdef[@kind='function']/name[text()='DEFUN' or text()='DEFUN_HIDDEN' or text()='DEFUN_DEPRECATED' or text()='DEFUN_NOSH']/../param[2]"/>
<xsl:variable name="aliases" select="//compounddef[@kind='file']/sectiondef[@kind='func']/memberdef[@kind='function']/name[text()='ALIAS']/../param[2]"/>

<xsl:template match="/sources">
  <xsl:message>Processing problematic function arguments...</xsl:message>
  <xsl:document href="04_params.patch" method="text">
    <xsl:for-each select="//compounddef[@kind='file']/sectiondef/memberdef[@kind='function']/param">
      <xsl:sort select="../location/@bodyfile"/>
      <xsl:sort select="../location/@bodyend" order="descending" data-type="number"/>
      <xsl:if test="key('findvar', declname/text()) and ../location/@bodyfile">
	<xsl:value-of select="concat('P;', declname, ';', ../location/@bodyfile, ';', ../location/@bodystart, ';', ../location/@bodyend, ';' , ../name)"/>
	<xsl:call-template name="newline"/> 
      </xsl:if>
    </xsl:for-each>

    <xsl:for-each select="$defuns">
      <xsl:sort select="../location/@bodyfile"/>
      <xsl:sort select="../location/@bodyend" order="descending" data-type="number"/>

      <!-- another doxygen bug ?! -->
      <xsl:choose>
	<xsl:when test="type/text()">
          <xsl:value-of select="concat('P;', type/text(), ';', ../location/@bodyfile, ';', ../location/@bodystart, ';', ../location/@bodyend, ';' , ../name)"/>
	</xsl:when>
	<xsl:otherwise>
          <xsl:value-of select="concat('P;', type/ref/text(), ';', ../location/@bodyfile, ';', ../location/@bodystart, ';', ../location/@bodyend, ';' , ../name)"/>
	</xsl:otherwise>
      </xsl:choose>

      <xsl:call-template name="newline"/>
    </xsl:for-each>

    <xsl:for-each select="$aliases">
      <xsl:sort select="../location/@file"/>
      <xsl:sort select="../location/@line" order="descending" data-type="number"/>

      <xsl:choose>
	<xsl:when test="type/text()">
          <xsl:value-of select="concat('P;', type/text(), ';', ../location/@file, ';', ../location/@line, ';', ../location/@line, ';' , ../name)"/>
	</xsl:when>
	<xsl:otherwise>
          <xsl:value-of select="concat('P;', type/ref/text(), ';', ../location/@file, ';', ../location/@line, ';', ../location/@line, ';' , ../name)"/>
	</xsl:otherwise>
      </xsl:choose>

      <xsl:call-template name="newline"/>
    </xsl:for-each>

  </xsl:document>
</xsl:template>

<xsl:template name="newline"><xsl:text>
</xsl:text></xsl:template>

</xsl:stylesheet>
