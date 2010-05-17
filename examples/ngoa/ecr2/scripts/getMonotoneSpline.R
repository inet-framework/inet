getMonotoneSpline <- function (xs, ys) {
###
### Computes a spline-based monotone smoothing function.
###
### Args:
###   xs: a vector of argument values (i.e., values of independent variable)
###   ys: a vector of observations (i.e., data values)
###
### Returns:
###   results from smooth.monotone() function
###
    library(fda)

    ## xs <- rep(1:5, each=5)  # vector of independent variables
    nxs <- length(xs)
    rng <- c(min(xs), max(xs))  # range of xs

    ## First set up a basis for monotone smooth.
    ## We use b-spline basis functions of order 6 (i.e., degree of 5)
    ## Knots are positioned at the x values of observation.
    norder <- 6
    nbreaks <- length(unique(xs))
    nbasis <- nbreaks + norder - 2
    wbasis <- create.bspline.basis(rng, nbasis, norder, unique(xs))

    ## starting values for coefficient
    cvec0 <- matrix(0,nbasis,1)
    Wfd0 <- fd(cvec0, wbasis)

    ## set up functional parameter object
    ## Lfdobj <- 3 # penalize curvature of acceleration
    Lfdobj <- 1  # penalize curvature of acceleration
    ## lambda <- 10^(-0.5) # smoothing parameter
    lambda <- 1 # smoothing parameter
    growfdPar <- fdPar(Wfd0, Lfdobj, lambda)

    ## Set up wgt vector
    wgt <- rep(1,nxs)

    ## Smooth the data
    ## ys = 1e5*(exp(-10*xs) + 2e-6*runif(length(xs)))
    ## return(smooth.monotone(xs, ys, growfdPar, wgt))
    return(smooth.monotone(xs, ys, growfdPar, wgt, conv=1e-4))

    ##     ## Extract the functional data object and regression coefficients
    ##     Wfd <- result$Wfdobj
    ##     beta <- result$beta

    ##     ## Evaluate the fitted height curve over a fine mesh
    ##     xsfine <- seq(1,20,len=101)
    ##     ysfine <- beta[1] + beta[2]*eval.monfd(xsfine, Wfd)

    ##     ## Plot the data and the curve
    ##     plot(xs, ys, type="p")
    ##     lines(xsfine, ysfine)
}
