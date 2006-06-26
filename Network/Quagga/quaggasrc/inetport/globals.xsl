<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:str="http://exslt.org/strings" 
		extension-element-prefixes="str">

<xsl:output method="text" indent="yes" encoding="iso-8859-1"/>

<xsl:variable name="vars" select="//compounddef[@kind='file']/sectiondef[@kind='var']/memberdef[@kind='variable']"/>
<xsl:variable name="defuns" select="//compounddef[@kind='file']/sectiondef[@kind='func']/memberdef[@kind='function']/name[text()='DEFUN' or text()='DEFUN_HIDDEN' or text()='DEFUN_DEPRECATED' or text()='DEFUN_NOSH' or text()='ALIAS']"/>

<xsl:template match="/sources">
  <xsl:message>Processing global variables...</xsl:message>

  <xsl:document href="03_globals.patch.in" method="text">

    <xsl:text>@;G_h;&gt;globalvars.h.tmp</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:text>@;G_c;&gt;globalvars.c.tmp</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:text>@;G_on;&gt;globalvars_on.h.tmp</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:text>@;G_off;&gt;globalvars_off.h.tmp</xsl:text>
    <xsl:call-template name="newline"/>

    <xsl:for-each select="$vars">
      <xsl:sort select="name"/>

      <xsl:choose>
	<xsl:when test="location/@bodyfile=location/@file and initializer">

          <xsl:value-of select="concat('G;', name, ';', location/@file, ';', location/@bodystart, ';')"/>

	  <!-- doxygen bug (?!) workaround -->
          <xsl:if test="starts-with(definition/text(), 'struct') and not(starts-with(type/text(), 'struct'))">
	    <xsl:text>struct </xsl:text>
	  </xsl:if>

          <xsl:value-of select="concat(type, ';', argsstring, ';', str:encode-uri(initializer, true()))"/>
	  <xsl:call-template name="newline"/>

	</xsl:when>
	<xsl:otherwise>

          <xsl:value-of select="concat('G;', name, ';', location/@file, ';', location/@line, ';')"/>

	    <!-- doxygen bug (?!) workaround -->
	    <xsl:if test="starts-with(definition/text(), 'struct') and not(starts-with(type/text(), 'struct'))">
	      <xsl:text>struct </xsl:text>
	    </xsl:if>

	  <xsl:value-of select="concat(type, ';', argsstring, ';')"/>
          <xsl:call-template name="newline"/>

	</xsl:otherwise>
      </xsl:choose>

    </xsl:for-each>

    <!-- globals defined by DEFUN* and ALIAS -->

    <xsl:for-each select="$defuns">
      <xsl:value-of select="concat('G;', ../param[2]/type, ';', ../location/@file, ';none;struct cmd_element;;memcpy')"/>
      <xsl:call-template name="newline"/>
    </xsl:for-each>

  </xsl:document>


</xsl:template>

<xsl:template name="newline"><xsl:text>
</xsl:text></xsl:template>

</xsl:stylesheet>
