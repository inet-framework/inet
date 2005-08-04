#!/bin/sh
# -*- Shell-script -*-
# Copyright (C) 2003 by Johnny Lai
#
# This script is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# Run from directory with many experiments as subdirs and each experiment has many runs
#This will call graph-calc.sh to collect results for each experiment and do so for every experiment in the current directory before creating a tar file of results

#cd $DATADIR (master/output)
echo "move old result dirs to an empty subdir before running this to make sure it does recalculate them or include them in the comparison (will get some rubbish in that empty subdir and large error.log file which can all be deleted)"
echo "suggested usage: "
echo "sh ~/scripts/graph-collect.sh ; sh ~/scripts/graph-R.sh ;ruby drawgraphs.rb hist"
rm -f results-list.out
find . \( -type d -maxdepth 1 -a ! -name . \) |xargs -i echo pushd {} \\\\\; "sh ~/scripts/graph-calc.sh &> errors.log" \\\\\; popd > procc-calc.out
sh ./procc-calc.out
find . \( -maxdepth 1 -type d -a ! -name . \) |xargs -i echo pushd {} \\\\\; "echo \`find \\\`pwd\\\` -maxdepth 1 -type f \` >> ../results-list.out" \\\\\; popd > procc-calc.out
sh ./procc-calc.out
command=`echo "perl -i -pwe \"s|$PWD/||g\" results-list.out"`
perl -i -pwe 's| \\$||g' results-list.out
#find . -name .vec |xargs rm
#don't know why tar /bzip combo decided to morph  (i.e. 0 size) the files I was adding
tar jcf results-`hostname`.tar.bz2  `cat results-list.out`
echo Tar file created
#This was more riduculous was twice size of other one
#tar --exclude-from exclude -c -f results-all.tar `find . -type d ! -name . `
