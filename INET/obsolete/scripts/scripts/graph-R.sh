#!/bin/sh
# -*- Ruby -*-
# Copyright (C) 2003 by Johnny Lai
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

THISSCRIPTNAME=~/scripts/graph-R.sh

#script to draw boxplots 
#ruby drawgraphs.rb --help for more information
GRAPHSCRIPT=drawgraphs.rb


RGRAPH=histgraph.R
#ruby script that should be executed to draw actual graphs.
RGRAPHCOMB=drawcombgraphs.R

#This script is a generator script to generate the ruby and R scripts to draw
#histograms. Ruby itself does not have an in place file syntax << EOF thingy.
#I was wrong HERE documents are supported in Ruby (from zenspider)
#This solutions allows indents in this document but output will not have begining indent
#(var = <<TARGET).gsub!(/^\s+/, '')
    #line one
    #line two
#TARGET
#Run $GRAPHSCRIPT from directory containing the factorial experiments. There
#will be two files generated an $RGRAPH file and many $GRAPHSCRIPT files per
#experiment. By running the ruby script the graphs will be drawn in the
#experiment subdirs.

#arguments to this script which are passed in by $GRAPHSCRIPT itself
#(recursively)

#prefix of output graphs i.e. preferably type name as prefix e.g. mip-noro
#main title (subtitle is inherent from type of graph i.e. dropcount or latency)
#"no" to prevent generation of $GRAPHSCRIPT
SCHEME=$4
SCHEME=`echo $SCHEME|perl -i -pwe 's/(PCOA-|HMIP-)//g'`
if [ "$1" != "" ]; then

cat << EOF > $RGRAPH
# -*- ESS[S] -*-
# Copyright (C) 2003 by Johnny Lai
#
#Two ways to use this script manually
#R  --no-save < $RGRAPH
#echo "source(\\"$RGRAPH\\", echo=FALSE)"| R --slave --quiet --vanilla 
ordertime <- function(hotime) {
  if (hotime < 95) 
    ret <- 1
  else if (hotime < 130)
    ret <- 2
  else if (hotime < 195)
    ret <- 3
  else
    ret <- 4
  ret
}

arlocalordertime <- function(hotime) {
  if (hotime < 75)
    ret <- 1
  else if (hotime < 150)
    ret <- 2
  else if (hotime < 250)
    ret <- 3
  else 
    ret <- 4
}

#ordermacro <- function(macroDelay) {
#if (macroDelay <= 0.05)
#  ret <- 1
#else if (macroDelay <=0.1)
#  ret <- 2
#else if (macroDelay <= 0.2)
#  ret <- 3
#else
#  ret <- 4
#ret
#}
#macDel <- sapply(macroDelay, ordermacro)
#macDel <- ordered(macroDelay, labels=c(0.05,0.1,0.2,0.5))
#
#ordermap <- function(microDelay) {
#if (microDelay <= 0.002)
# ret <- 1
#else
# ret <- 2
#ret
#}
#mapDel <- sapply(mapDelay, ordermap)
#mapDel <- ordered(mapDelay, labels=c(0.002,0.02))
 

mipscan<-scan("handover.dat", list(scheme="",time=0,latency=0))
mipscandrop<-scan("dropcount.dat", list(scheme="",time=0,drop=0))
attach(mipscan)
hotime <- sapply(time, ordertime)
ho<-ordered(hotime, labels=c("1st","2nd","3rd","4th"))
rm(hotime)
#handover <- data.frame(scheme="$1", ho=ho, time=time, latency=latency,dropcount=mipscandrop\$drop)
handover <- data.frame(scheme="$SCHEME", ho=ho, time=time, latency=latency,dropcount=mipscandrop\$drop)
detach(mipscan)
rm(mipscan, mipscandrop,ho)
attach(handover)
#save.image("$1.Rdata");q("no")
#According to stats book sqrt of number of observations is a good determinant for number of breaks.
#R itself has 3 diff methods but they produce less breaks than this method
postscript("$1-handover.eps", horizontal=FALSE, onefile=FALSE, paper="a4",pointsize=8)
intervals <-ceiling(sqrt(length(latency)))
histret <- hist(latency, intervals,freq=TRUE,density=200,col="black",axes=FALSE,angle=0,main="$2",xlab="Handover Latency (s)")
#R's axis ticks are shocking i.e. not many ticks at all
yticks<-pretty(seq(0,max(histret\$counts), 1),n=length(unique(histret\$counts)))
axis(2, at=yticks)
xticks<-pretty(seq(0,ceiling(max(latency)),max(latency)/intervals), n=intervals)
axis(1,at=xticks)
#title(main="$2",sub="Handover Latency",xlab="Handover Latency")
box()
rug(latency)
dev.off()
postscript("$1-dropcount.eps", horizontal=FALSE, onefile=FALSE, paper="a4",pointsize=8)
intervals <-ceiling(sqrt(length(dropcount)))
histret <- hist(dropcount, intervals,freq=TRUE,density=200,col="black",axes=FALSE,angle=0,main="$2",xlab="Handover Packet Loss")
yticks<-pretty(seq(0,max(histret\$counts), 1),n=length(unique(histret\$counts)))
axis(2, at=yticks)
xticks<-pretty(seq(0,ceiling(max(dropcount)),max(latency)/intervals), n=intervals)
axis(1,at=xticks)
#title(main="$2",sub="Handover Packet Loss",xlab="Handover Packet Loss")
box()
rug(dropcount)
dev.off()
warnings()
detach(handover)
rm(xticks,yticks,intervals,histret,ordertime)
save.image("$1.Rdata")
EOF

fi

if [ "$1" = "" ]; then
cat << EOF > $RGRAPHCOMB
  library(gregmisc)

  boxplotmeanscomp <-function(out, var, data, fn="graph", tit="Handover Improvement", xlab="Mobility management scheme", ylab="Handover latency (s)", sub ="Comparing boxplots and non-robust mean +/- SD", log = FALSE, subset=0 ) 
  {
    #all measurements in inches for R
    width<-20/2.5
    height<-18/2.5
    fontsize<-8
    paper<-"special" #eps specify size for eps box
    if (log)
      logaxis="y"
    else
      logaxis=""
    cat(paste(fn, "-box.eps",sep=""))
    postscript(paste(fn, "-box.eps",sep="") , horizontal=FALSE, onefile=FALSE, paper=paper,width=width, height=height, pointsize=fontsize+2)

    #Assuming we never want to plot just one row
    if(length(subset) == 1)      
      box<-boxplot.n(out ~ var, data=data, log=logaxis,main=tit, xlab=xlab, ylab=ylab,top=TRUE)
    else
    {
      box<-boxplot.n(out[subset] ~ var[subset], data=data, log=logaxis,main=tit, xlab=xlab, ylab=ylab, top=TRUE)
      #box<-boxplot.n(out ~ var, data=data, log=logaxis,main=tit, xlab=xlab, ylab=ylab, subset=subset, top=TRUE)
      var <-var[subset]
      out <-out[subset]
    }

      #box<-boxplot(out ~ var, data=data, log=logaxis,main=tit, xlab=xlab, ylab=ylab)
      #boxplot(latency~scheme, subset= ho!="4th" & scheme!="l2trig")

    mn.t<-tapply(out, var, mean)
    sd.t <-tapply(out, var,sd)
    xi <- 0.3 +seq(box\$n)
    points(xi, mn.t, col = "orange", pch = 18)
    arrows(xi, mn.t - sd.t, xi, mn.t + sd.t, code = 3, col = "red", angle = 75,length = .1)
    title(sub=sub)
    #flush output buffer/graph
    dev.off()
#    library(gregmisc)

    #rm(box,mn.t,sd.t,xi)
    postscript(paste(fn, "-meancin.eps",sep=""), horizontal=FALSE, onefile=FALSE, paper=paper,width=width, height=height, pointsize=fontsize)
#    cat(paste(fn, "-meancin.eps",sep=""))  
#      X11()
    plotmeans(out ~ var, data, ci.label=TRUE, mean.label=TRUE,connect=FALSE,use.t=FALSE,ylab=ylab,xlab=xlab,main=tit)
    title(sub="Sample mean and confidence intervals")
    dev.off()

  }

EOF

fi

if [ "$3" = "no" ]; then
  exit
fi


cat << EOF > $GRAPHSCRIPT
#!/usr/bin/env ruby
# Copyright (C) 2003 by Johnny Lai
#
#args hist to create histograms and Rdata file for each experiment (will create combined plot always)


#filter is just to separate the plot into easily cmoparible parts otherwise can
#get a huge dynamic range which is meaningless visually

#Filter is a word so that if binc is true then only directories matching filter
#will be graphed together in the plot. If binc is false everything else is
#graphed together except for the ones that match.

#subset is an expression to exclude certain data points from the dataset
def combinedplot(pwd,modsArray, bfilter = false, binc = false, filter =/.*/, subset="0", plot=true)
#modsArray, bfilter, binc, filter
  IO.popen("R --slave --quiet --vanilla --no-readline", "w+") { |p|
    puts("In combinedplot")
    tempEmpty=true
    modsArray.each { |prefix|
      if (bfilter)
        if binc
          next if not filter=~prefix
        else
          next if filter=~prefix
        end
      end
      datafile = prefix+".Rdata"   
      p.puts("load(\"#{datafile}\")")
      p.puts("handover<-rbind(temp, handover)") unless tempEmpty
      p.puts("temp<-handover")
      tempEmpty=false if tempEmpty
    }
    
    return if tempEmpty

    p.puts("rm(temp)")
    p.puts("save.image(\"#{\$CombineDataName}.Rdata\")")
    if !plot
      p.puts("q(\"no\")")
      return
    end


    p.puts("source(\"$RGRAPHCOMB\")")
    p.puts("attach(handover)")
    basename=File.basename(pwd)
    unique=""
    if (bfilter)
      unique="-"
      if (binc)
        unique+= "inc"
      else 
        unique+= "exc"
      end
      filter.to_s =~ /:(\w+)+/
      unique+="-"+ \$1                   
    end

    p.puts("boxplotmeanscomp(latency, scheme, handover, fn=\"#{basename + unique}-ho\", tit=\"Handover Improvement: Latency\", xlab=\"Mobility management schemes\", subset=#{subset})")
    
    p.puts("boxplotmeanscomp(dropcount, scheme, handover, fn=\"#{basename + unique}-dc\", tit=\"Handover Improvement: Packet Loss\", ylab=\"Packet Loss\", log = FALSE, subset=#{subset})")
    p.puts("detach(handover)")
    p.puts("q(\"no\")")

  }
end


def main() 

\$CombineDataName="combined"
opt_subset="ho!=\"1st\""
opt_hist=false
opt_combine=true
opt_filter=/fastras/  
opt_split=true
opt_plot=true
opt_aggregate=false
opt_force=false

require 'optparse'
require 'pp'
ARGV.options { |opt|
    opt.banner = "Usage: ruby #{__FILE__} [options] "

  opt.on("--help", "What you see right now"){ puts opt; exit 0}

  opt.on("--[no-]hist=[FLAG]", "-h", "Generate histograms of each experimental permutation"){ |opt_hist| 
  "Skipping histogram generation" if !opt_hist
  }

  opt.on("--[no-]combine=[FLAG]", "-c", "load all results from subdirectories into",
         "one big RData file in topdir"){ |opt_combine|}

  opt.on("--sub=SUBSET", "-s", String, "Plot subset only"){ |opt_subset|   
    puts "subsetting on #{opt_subset}"
  }

  opt.on("--filter=FILTER", "-f", String, "Plot inc/exc FILTER regexp.",
         "Splits into two sets of data and plots for easier comparison",
         "(data is overwritten though so one set is missing)"){ |opt_filter| opt_split=true}

  opt.on("--no-split", "-n", "Don't split data set.  Want the whole thing"){|o| opt_split=false }

  opt.on("--no-combplot", "-P", "No to plotting combined graphs",
         "e.g. for creating a big big dataset (implies -n) "){ |o|
    opt_split = opt_plot = false
  }
  opt.on_tail("By default splits data according to #{opt_filter}, ",
              "Subset plots on #{opt_subset}. Will not create histograms")

  opt.on("-x", "parse arguments and show Usage") {|@quit|}

  opt.on("-A", "Aggregate on other factors e.g. propdelay.",
         "Name the directory with the propdelay arg for use as a factor.",
         "Will ignore other flags. Use in directory above usual"){ |opt_aggregate| }
  opt.on("-F","--forcegen", "Force regeneration of Rdata files with no plots"){|opt_force| }
  opt.parse!
} or exit(1); #Quits when unrecognised option parsed

#Using the instance @var would have been better so we can move argument processing
#function into separate getop function

if @quit
  pp self
  puts "opt_hist=#{opt_hist}, opt_combine=#{opt_combine}, opt_filter=#{opt_filter}",
       " opt_subset=#{opt_subset} opt_split=#{opt_split} opt_plot=#{opt_plot}"

  (print ARGV.options; exit) 

end


dirs=\`find . -type d -maxdepth 1 ! -name .\`


titles = {
  'l2trig' => 'L2 Trigger',
  'l2' => 'L2 Trigger', 
  'odad' => 'Optimistic DAD',
  'mip' => 'Mobile IPv6',
  'noro' => 'No Route Optimisation',
  'hmip' => 'Hierarchical MIPv6',
  'fastras' => 'Fast Solicited RA',
  'beacon' => 'Fast RA Beacons',
  'wlan' => 'IEEE802.11 WLAN',
  'pcoaf' => 'Previous CoA Forwarding',
  'PCOA' => 'PCOAF: ',
  'HMIP' => 'HMIP: '

}

pwd=\`pwd\`
pwd.chomp!

if !opt_aggregate then
modsArray = Array.new
for dir in dirs.split(/\n/)
  dir.gsub!(/\.\//,"")

  begin
    Dir.chdir(pwd + "/" + dir)
  rescue SystemCallError 
    \$stderr.print "cd to #{pwd + %Q{/} + dir} failed" + \$! 
    #opFile.close File.delete(opName) raise
    next
  end
  #puts(\`pwd\`)
  m = /(\w+-)+\d+.*$/.match(dir)
  next unless m
  mods=dir[0, m.offset(1)[1]].chop!;
  title=""
  mods.scan(/\w+/){ |w|
    
    if titles[w] 
      #    print( titles[w]  ) 
      title=title + titles[w]
      title += " + " unless /(HMIP|PCOA)/ =~ w
    else
     \$stderr.puts("unrecognized word when constructing title is #{w}")
    end
  }
  #Remove excess "+"
  title.chop!
  title=title[0,title.size-1].chop!
  
  title = "AR Local: " + title unless /(HMIP|PCOA)/ =~ dir  
  modsArray[modsArray.length] = dir + "/" + mods

  next unless opt_combine

  puts("In dir #{dir}")

  outfilename = String.new(mods)
  if /(HMIP|PCOA)/ =~ dir and /l2trig|odad|fastras/ =~ mods    
    mods.gsub!(/l2trig|odad|fastras/,"")
    mods.gsub!(/(-)+/,"-")
    mods.sub!(/-$|$/,"-ar")
  end
  
  system("sh $THISSCRIPTNAME #{outfilename} \"#{title}\" no #{mods}")
  #Same thing in system does not work funnily enough
  \`perl -i -pwe "s|sapply\\(time, ordertime\\)|sapply(time, arlocalordertime)|" $RGRAPH\` unless /(HMIP|PCOA)/ =~ dir

  \`perl -i -pwe "s|^#save|save|" $RGRAPH\`   if !opt_hist

  \`~/scripts/graph-calc.sh\` if not File.exist?("handover.dat") or \
                               not File.exist?("dropcount.dat")
    
  IO.popen("R --slave --quiet --vanilla", "w+") { |p|
    unless /debug/ =~ ARGV[0]
      p.puts("source(\"$RGRAPH\", echo=FALSE)")
    else
      File.open("$RGRAPH").each_line { |l| 
        p.print(l)
      }
    end
  }
end

begin
  Dir.chdir(pwd)
rescue SystemCallError 
  \$stderr.print "cd back to toplevel (#{pwd}) failed: " + \$!   
  \$stderr.print "Cannot continue!!"  
  exit(2)
end

if /(HMIP|PCOA)/ =~ dir 
  if opt_split
    for binc in [true, false]
      combinedplot(pwd,modsArray, opt_split, binc, opt_filter, opt_subset, opt_plot)
    end
  else
    combinedplot(pwd, modsArray, opt_split, false, "", opt_subset, opt_plot)
  end
else
  combinedplot(pwd, modsArray)
end
end

if opt_aggregate
  #rdata has keys of filename and value of labels
  rdata = Array.new
  #pwd is the top dir i.e. hmip-0.02-0.005
  pwdir=Dir.new(pwd)
  
  pwdir.each{ |d|
    begin
    
      Dir.chdir(pwd)
      next if d =~ /^[.]+$/  
      next unless File.directory?(d)

      Dir.chdir(d)
    rescue SystemCallError 
      \$stderr.print "cd to #{d} failed" + \$! 
      next
    end

    if Dir[\$CombineDataName+".Rdata"].size == 0 or opt_force
      system("sh $THISSCRIPTNAME")
      system("ruby #{__FILE__} --no-split --no-hist --no-combplot")
    end
    #d is one of the permutations on scheme
    #Running this again to make sure we reread all the correct results since the
    #split during plotting combgraphs overwrites half of the dataset
    #\`ruby drawgraphs.rb --no-hist --no-split --no-combplot\`
    ( \$stderr.puts("Cannot find Rdata file inside #{d} please check!!!");exit(2) ) if Dir[\$CombineDataName+".Rdata"].size == 0
    #rdata[d.path + "/" + \$CombineDataName+".Rdata"]=d.path
    rdata[rdata.size]=d
  }
  return if rdata.empty?

  Dir.chdir(pwd)
  puts rdata.size
  IO.popen("R --slave --quiet --vanilla --no-readline", "w+") { |p|
    puts("In Aggregation (based on directory names)")
    tempEmpty=true
    rdata.each { |prefix|      

      if prefix =~ /(\d+)\.(\d+)/
        macroDelay=\$& 
      else
        puts "Cannot determine the Internet delay from the prefix name #{prefix}"
        exit(1)
      end
      microDelay = ""
      if $' =~ /(\d+).(\d+)/
        microDelay=\$&
      end
      datafile = pwd + "/" + prefix + "/" + \$CombineDataName+".Rdata"
      p.puts("load(\"#{datafile}\")")
      #Add another two factors macro/micro delay
      p.print("handover<-data.frame(macroDelay=\"#{macroDelay}\",")
      p.print("mapDelay=\"#{microDelay}\",") if microDelay != ""
      #Using numbers and turning these to factors in order to rearrange them is too verbose
      #use jl.relevel for that
      #p.print("handover<-data.frame(macroDelay=#{macroDelay},")
      #p.print("mapDelay=#{microDelay},") if microDelay
      p.puts("handover)")
      p.puts("handover<-rbind(temp, handover)") unless tempEmpty
      p.puts("temp<-handover")
      tempEmpty=false if tempEmpty
    }
    p.puts("rm(temp)")
    p.puts("save.image(\"supercombined.Rdata\")")
  }
end #if aggregate data
end #End main

main

EOF
