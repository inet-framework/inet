confidenceInterval <- function(conf.level) {
  function(v) {
    n <- length(v)
    if (n <= 1)
      Inf
    else
      qt(1-(1-conf.level)/2,df=n-1)*sd(v)/sqrt(n)
  }
}
