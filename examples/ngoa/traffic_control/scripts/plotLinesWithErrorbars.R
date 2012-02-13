plotLinesWithErrorbars <- function(data,
                                   group,
                                   xlab,
                                   ylab,
                                   glab,
                                   optional)
{
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=data, aes(group=group, colour=factor(group), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(data$mean+data$ci.width)))
    .p <- .p + xlab(xlab) + ylab(ylab)
    .p <- .p + geom_point(aes(group=group, shape=factor(group), x=n, y=mean), size=.pt_size) + scale_shape_manual(glab, values=0:9)
    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete(glab)
    return(.p)
}
