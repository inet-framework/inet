CalculateCI <- function (x, p = 0.95) {
###
### Computes the confidence interval half-width of a given vector.
###
### Args:
###   x: a vector of observations whose confidence interval is to be calculated.
###   p: confidence level. Default is 0.95
###
### Returns:
###   The half-width of p-percent confidence interval of x.
###
    .y <- x[!is.na(x)]
    .n = length(.y) # remove NA elements
    
    ## Error handling
    if (.n <= 1) {
        ## stop("Argument x should have more than one elements.")
        return(NA)
    }
    else {
        return(qt((1+p)/2, .n-1) * sqrt(var(.y)/.n))
    }
}
