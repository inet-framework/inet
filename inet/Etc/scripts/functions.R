### Copyright (C) 2004 by Johnny Lai
###
### This program is free software; you can redistribute it and/or
### modify it under the terms of the GNU General Public License
### as published by the Free Software Foundation; either version 2
### of the License, or (at your option) any later version.
###
### This program is distributed in the hope that it will be useful,
### but WITHOUT ANY WARRANTY; without even the implied warranty of
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
### GNU General Public License for more details.
###
### You should have received a copy of the GNU General Public License
### along with this program; if not, write to the Free Software
### Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


###Levels must be how you want the actual ordering of the levels to be Note
###according to the docs for relevel works for unordered factors only
###e.g. jl.relevel(scheme, c("mip", "pcoaf","hmip", "hmip-pcoaf", "ar",
###"pcoaf-ar","hmip-ar","hmip-pcoaf-ar"))
jl.relevel <- function(ufactor, levels)
  {
    rev.levels <- rev(levels)
    factor.new <- ufactor
    for (sch in rev.levels )
      {
        factor.new <- relevel(factor.new, sch)
      }
    factor.new
  }

jl.boxplotmeanscomp <-function(out, var, data, fn="graph", tit="Handover Improvement",
                               xlab="Mobility management scheme", ylab="Handover latency (s)",
                               sub ="Comparing boxplots and non-robust mean +/- SD",
                               log = FALSE, subset=0, print=TRUE, plotMeans=TRUE, notch=FALSE)
  {
    if (!require(gregmisc) && !require(gplots))
      return

    ##all measurements in inches for R
    width<-20/2.5
    height<-18/2.5
    fontsize<-8
    paper<-"special" #eps specify size for eps box
    if (log)
      logaxis="y"
    else
      logaxis=""
    
    ##if (fn==NULL)
    ##if (length(fn)==0) #pretty silly don't know why they didn't allow fn==NULL test
    onefile=FALSE #manual recommends this for eps

    if (print)
      postscript(paste(fn, "-box.eps",sep=""), horizontal=FALSE,
                 onefile=onefile,
                 paper=paper,width=width, height=height, pointsize=fontsize+2)
###    else
###      X11()
    ##Assuming we never want to plot just one row
    if(length(subset) == 1)      
      box<-boxplot.n(out ~ var, data=data, log=logaxis,main=tit, xlab=xlab, ylab
                     =ylab,top=TRUE,notch)
    else
      {
        box<-boxplot.n(out[subset] ~ var[subset], data=data, log=logaxis,main=tit,
                       xlab=xlab, ylab=ylab, top=TRUE,notch)
        ##box<-boxplot.n(out ~ var, data=data, log=logaxis,main=tit, xlab=xlab, ylab=ylab, subset=subset, top=TRUE)
        var <-var[subset]
        out <-out[subset]
      }
    ##box<-boxplot(out ~ var, data=data, log=logaxis,main=tit, xlab=xlab, ylab=ylab)
    ##boxplot(latency~scheme, subset= ho!="4th" & scheme!="l2trig")

    mn.t<-tapply(out, var, mean)
    sd.t <-tapply(out, var,sd)
    xi <- 0.3 +seq(box$n)
    points(xi, mn.t, col = "orange", pch = 18)
    arrows(xi, mn.t - sd.t, xi, mn.t + sd.t, code = 3, col = "red", angle = 75,length = .1)
    title(sub=sub)
    rm(mn.t,sd.t,xi)

    if (plotMeans)
      {

        if (print)
          {
            dev.off()#flush output buffer/graph
            postscript(paste(fn, "-meancin.eps",sep=""), horizontal=FALSE, onefile=onefile,
                       paper=paper,width=width, height=height, pointsize=fontsize)
          }
        else
          X11()

        plotmeans(out ~ var, data, n.label=FALSE, ci.label=TRUE, mean.label=TRUE,connect=FALSE,use.t=FALSE,
                  ylab=ylab,xlab=xlab,main=tit)
        title(sub="Sample mean and confidence intervals")

        if (print)
          dev.off()
      }

    box
  }

###Assumes that 0 is the omnetpp vector number for pingDelay
scanRoundTripTime <- function(filename, scheme)
{
  rttscan <- scan(pipe(paste("grep ^0", filename)),list(scheme=0,time=0,rtt=0))
  attach(rttscan)
  rtt <- data.frame(scheme=scheme, time=time, rtt=rtt)
  detach(rttscan)
  rtt
}

simulateTimes <-  function()
  {
    time <- 0
    ping.begin <- 7
    sim.end <- 250
    ping.dur <- sim.end - ping.begin
    ping.period <- 0.05
    ##R starts at 1 as 0 is class or type of vector
    index <- 1
    fastbeacon.min <- 0.05
    fastbeacon.max <- 0.07
    ping.times <- 1:(ping.dur*(1/ping.period))
    ##Give it a longer length just to make it happy
    value <- vector(mode="numeric",length(ping.times))
    while (time < ping.dur)
      {
        value[index] <- runif(1, fastbeacon.min, fastbeacon.max)
        time <-time + value[index]
        value[index] <-  time
        index <- index + 1
      }
    
    #Eliminate 0 values
    value <- value[!is.element(value, 0)]
    ping.times <- ping.times[1:length(value)]
    cor(ping.times, value)
    ##ret <- data.frame(time=value)
    ##values get deleted once return from function
    ##    rm(index, ping.period, sim.end, ping.times, time, ping.dur,value,fastbeacon.min, fastbeacon.max)
#Don't know how to return a clist
    ##ret <- vector(length=2)
    ##ret[1]=value
    ##ret[2]=ping.times
    clist(value, ping.times)
  }

###In emacs use C-M-h to quickly highlight a function and type manuall M-x
###ess-eval-region.  After that we can use C-r and type eval to look up history
###of complex commands typed in and run straight away.

jl.ci <- function(x,y, p=0.95,use.t=TRUE, rowLabels=levels(y),
                  columnLabels=c("n", "Mean", "Lower CI limit",
                    "Upper CI limit"), unit=c("",rep("(s)",length(columnLabels)-1)))
  {
   means <- tapply(x,y, mean)
   ns <- tapply(x,y,length)
   vars <- tapply(x,y,var)
   if (use.t)
     ciw <- qt((1+p)/2,ns-1) * sqrt(vars)/sqrt(ns-1)
   else ciw <- qnorm((1+p)/2)*sqrt(vars/(ns-1))
   ci.lower <- means - ciw
   ci.upper <- means + ciw

   ##c is concatentate components of each arg so length of result is length of
   ##indiivdual cmps
   ##list is contatene components and length is number arguments concatenated
   hugelist <- c(ns,means,ci.lower,ci.upper)
   table <- matrix(hugelist,length(ciw),length(hugelist)/length(ciw))
   columnLabels <- paste(columnLabels, unit)
   dimnames(table) <- list(rowLabels, columnLabels)
   table
 }

jl.sem <- function(x,y=NULL)
  {
    if (length(y) == 0)
      {
        means <- mean(x)
        ns <- length(x)
        vars <- var(x)
      } else
      {
        means <- tapply(x,y, mean)
        ns <- tapply(x,y,length)
        vars <- tapply(x,y,var)
      }
   #standard error of the median (1.25 approximates sqrt(pi/2))
   #semedian <- 1.25*sem
   sqrt(vars/ns)
  }

jl.sed <- function(sem1,sem2)
  {
   #standard error of the difference in two means from same population diff samples
    sqrt(sem1^2 + sem2^2)    
  }

jl.cis <- function(x,p=0.95,use.t=TRUE, rowLabel="x", columnLabels=c("n", "Mean", "Lower CI limit",
                                                               "Upper CI limit"), unit="")
  {  
   means <- mean(x)
   ns <- length(x)
   vars <- var(x)
   sem <- sqrt(vars/ns) #is it ns-1 or just ns? I guess for large it doesn't matter but for small
   if (use.t)
     ciw <- qt((1+p)/2,ns-1) * sem
   else ciw <- qnorm((1+p)/2)* sem
   ciw
   ci.lower <- means - ciw
   ci.upper <- means + ciw
   hugelist <- c(ns,means,ci.lower,ci.upper)
   return(hugelist)
   if (FALSE) {
     table <- matrix(hugelist,length(ciw),length(hugelist)/length(ciw))
     dimnames(table) <- list(rowLabel, columnLabels)
     table
   }
  }

##Gives back the value of true for odd numbers 
jl.odd <- function(x)
  {
    if (length(x) == 1)
      {
        if (x %% 2 > 0)
          return(TRUE)
        else
          return(FALSE)
      }
    ##For each element
    for (i in x) {
      if (!jl.odd(i))
        return(FALSE)
    }
    TRUE
  }

##Change the dataframe$factor use new level labels
##dataframe is a single dataframe or list of them.
##factor is a factor column name in dataframe
##levels is the new level names. No checking is done on arguments
##Returns altered dataframe
jl.changeLevelName <- function(dataframe, factor, levels)
  {
    levels(dataframe[,factor]) <- levels
    return(dataframe)
  }
