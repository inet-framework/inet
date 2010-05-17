vp.layout <- function(x, y) viewport(layout.pos.row=x, layout.pos.col=y)
arrange <- function(..., nrow=NULL, ncol=NULL, as.table=FALSE) {
    dots <- list(...)
    n <- length(dots)
    if(is.null(nrow) & is.null(ncol)) { nrow = floor(n/2) ; ncol = ceiling(n/nrow)}
    if(is.null(nrow)) { nrow = ceiling(n/ncol)}
    if(is.null(ncol)) { ncol = ceiling(n/nrow)}
    ## NOTE see n2mfrow in grDevices for possible alternative
    grid.newpage()
    pushViewport(viewport(layout=grid.layout(nrow,ncol) ) )
    ii.p <- 1
    for(ii.row in seq(1, nrow)){
        ii.table.row <- ii.row
        if(as.table) {ii.table.row <- nrow - ii.table.row + 1}
        for(ii.col in seq(1, ncol)){
            ii.table <- ii.p
            if(ii.p > n) break
            print(dots[[ii.table]], vp=vp.layout(ii.table.row, ii.col))
            ii.p <- ii.p + 1
        }
    }
}
