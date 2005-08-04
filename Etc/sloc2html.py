#!/usr/bin/env python2.2
# Written by Rasmus Toftdahl Olesen <rto@pohldata.dk>
# Released under the GNU General Public License v. 2 or higher
# Johnny minor changes to add extra languages in colours array;date in output;skip lines with (none)
# Fixed bug in outputting cost when value was person effort
from string import *
import sys
import time
NAME = "sloc2html"
VERSION = "0.0.1m"

if len(sys.argv) != 2:
    print "Usage:"
    print "\t" + sys.argv[0] + " <sloc output file>"
    print "\nThe output of sloccount should be with --wide formatting"
    sys.exit()

#Konqueror is better than mozilla(mozilla has no notion of light colours)
colors = { "python" : "blue", #light blue 
           "ansic" : "yellow",
           "cpp" : "green", #light green
           "sh" : "red",
           "yacc" : "brown",
           "lex" : "silver",
	   "tcl" : "orange",
	   "perl": "grey",
	   "makefile":"light grey",
	   "awk":"grey",
           "ruby":"pink",
	   "csh":"red",}


print "<html>"
print "<head>"
print "<title>Physical Source Lines of Code (SLOC)</title>"
print "</head>"
print "<body>"
print "<h1>Physical Source Lines of Code</h1>"
print "Generated on " + time.asctime() + " using <a href=\"http://www.dwheeler.com/sloccount/\">SLOCCount</a>"
file = open ( sys.argv[1], "r" )

print "<h2>Projects</h2>"
line = ""
while line != "SLOC\tDirectory\tSLOC-by-Language (Sorted)\n":
    line = file.readline()

print "<table>"
print "<tr><th>Lines</th><th>Project</th><th>Language distribution</th></tr>"
line = file.readline()
while line != "\n":
    num, project, langs = split ( line )
    if langs == "(none)":
        line = file.readline()
        continue
    print "<tr><td>" + num + "</td><td>" + project + "</td><td>"
    print "<table width=\"500\"><tr>"
    for lang in split ( langs, "," ):
        l, n = split ( lang, "=" )
        print "<td bgcolor=\"" + colors[l] + "\" width=\"" + str( float(n) / float(num) * 500 ) + "\">" + l + "=" + n + "&nbsp;(" + str(int(float(n) / float(num) * 100)) + "%)</td>"
    print "</tr></table>"
    print "</td></tr>"
    line = file.readline()
print "</table>"

print "<h2>Languages</h2>"
while line != "Totals grouped by language (dominant language first):\n":
    line = file.readline()

print "<table>"
print "<tr><th>Language</th><th>Lines</th></tr>"
line = file.readline()
while line != "\n":
    lang, lines, per = split ( line )
    lang = lang[:-1]
    print "<tr><td bgcolor=\"" + colors[lang] + "\">" + lang + "</td><td>" + lines + " " + per + "</td></tr>"
    line = file.readline()
print "</table>"

print "<h2>Totals</h2>"
while line == "\n":
    line = file.readline()

print "<table>"
print "<tr><td>Total Physical Lines of Code (SLOC):</td><td>" + strip(split(line,"=")[1]) + "</td></tr>"
line = file.readline()
print "<tr><td>Estimated development effort:</td><td>" + strip(split(line,"=")[1]) + " person-years (person-months)</td></tr>"
line = file.readline()
line = file.readline()
print "<tr><td>Schedule estimate:</td><td>" + strip(split(line,"=")[1]) + " years (months)</td></tr>"
line = file.readline()
line = file.readline()
print "<tr><td>Estimated Average Number of Developers (Effort/Schedule):</td><td>" + strip(split(line,"=")[1]) + "</td></tr>"
line = file.readline()
print "<tr><td>Total estimated cost to develop:</td><td>" + strip(split(line,"=")[1]) + "</td></tr>"
print "</table>"

file.close()

print "</body>"
print "</html>"
