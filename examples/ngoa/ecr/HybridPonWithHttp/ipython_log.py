#log# Automatic Logger file. *** THIS MUST BE THE FIRST LINE ***
#log# DO NOT CHANGE THIS LINE OR THE TWO BELOW
#log# opts = Struct({'__allownew': True,
 'logfile': 'ipython_log.py',
 'profile': 'scipy',
 'pylab': 1})
#log# args = []
#log# It is safe to make manual edits below here.
#log#-----------------------------------------------------------------------
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
"" = ""
"" == ""
tmp = raw_input()
tmp == ""
float("")
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
who 
_ip.magic("whos ")
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic("pwd ")
logstart(logfname=".\ecr_calculation_20091101.log")
help(logstart)
import IPython.Logger as log 
log.logstart(logfname=".\ecr_calculation_20091101.log")
log.Logger.logstart(logfname=".\ecr_calculation_20091101.log")
%logstart(logfname=".\ecr_calculation_20091101.log")
_ip.magic("lsmagic ")
_ip.magic("logstart ")
_ip.magic("logstop ")
dir *.py
_ip.system("dir /on ")
dir 
_ip.system("dir /on ")
_ip.system("dir /on -l")
_ip.system("dir /on d*")
_ip.system("dir /on log*")
_ip.magic("logstart ")

_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic(r'run "..\scripts\ecr_calculation.py"')
_ip.magic("pwd ")
_ip.magic("run plot_delay_ecr.py")
_ip.magic("run plot_delay_ecr.py")
_ip.magic("run plot_delay_ecr.py")
_ip.magic("run plot_delay_ecr.py")
_ip.magic("run plot_delay_ecr.py")
_ip.magic("run plot_delay_ecr.py")
_ip.magic("pwd ")
_ip.magic(r"cd ..\EcrReferenceWithHttp")
_ip.magic("run plot_delay2.py")
_ip.magic("run plot_delay2.py")
_ip.magic("run plot_delay2.py")
plot([1, 2, 3, 4])
clf()
plot([1, 2, 3, 4])
clf()
plot([1, 2, 3, 4], 'bo')
_ip.magic("pwd ")
_ip.magic(r"cd ..\HybridPonWithHttp")
_ip.magic("run plot_ecr2.py")
_ip.magic("run plot_ecr2.py")
_ip.magic("run plot_ecr2.py")
_ip.magic("run plot_ecr2.py")
_ip.magic("run plot_ecr2.py")
_ip.magic("run plot_ecr2.py")
_ip.magic("run plot_ecr2.py")
t = arange(0, 10, 0.01)
y = exp(t)
clf()
plot(t, y)
clf()
y = exp(-t)
plot(t, y)
y = exp(-3*t)
plot(t, y)
_ip.magic("pwd ")
x, y = loadtxt("N16_dr10_fr40_br1_bd5.ecr.new", usecols=[0,1], unpack=true)
x, y = loadtxt("N16_dr10_fr40_br1_bd5.ecr.new", usecols=[0,1], unpack=True)
clf()
plot(x,y)
from scipy.optimize import curve_fit
help(curve_fit)
dir(scipy.optimize)
from scipy.optimize.minpack import curve_fit
from scipy.optimize.minpack import leastsq
help(leastsq)
x
y
def residuals(p,x,y):
   return y-(p[0]*np.exp(p[1]*x)+p[2])
def func(x,p):
   return p[0]*np.exp(p[1]*x)+p[2]
p0=[1.,1.,1.]
plsq = opt.leastsq(residuals, p0, args=(x,y))
plsq = optimize.leastsq(residuals, p0, args=(x,y))
print(plsq[0])
plot(x, y, 'o', x, func(x, plsq[0]))
help(leastsq)
