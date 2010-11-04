source("~/programming/R/truncated/truncated.R")

f <- function(x) {   ## driver function to find mu and sigma of truncated lognormal
  x1 <- x[1]
  x2 <- x[2]
  (2e8 - extrunc("lnorm", a=0, b=5e8, mean=x1, sd=x2))^2 + +
  (0.722^2 - vartrunc("lnorm", a=0, b=5e8, mean=x1, sd=x2))^2
}

optim(c(50, 0.5), f)
