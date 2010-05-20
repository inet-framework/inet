GetMeanAndCiWidth <- function(df, col) {
###
### Computes the sample mean and the confidence interval half-width.
###
### Args:
###   df:   a data frame.
###   col:  the column of the data frame whose mean and confidence interval
###         half-width are to be calculated.
###
### Returns:
###   The sample mean and the half-width of 95-percent confidence interval
###   df$col.
###
    return (data.frame(mean=mean(df[col], na.rm=TRUE),
                       ci.width=CalculateCI(df[col])))
}


GetMeansAndCiWidths <- function(df) {
###
### Computes the sample means and the confidence interval half-widths.
###
### Args:
###   df:   a data frame.
###
### Returns:
###   The sample mean and the half-width of 95-percent confidence interval
###   df$col.
###
    return (
            data.frame(
                       ftp.delay.mean=mean(df$ftp.delay, na.rm=TRUE),
                       ftp.delay.ci.width=CalculateCI(df$ftp.delay),
                       ftp.throughput.mean=mean(df$ftp.throughput, na.rm=TRUE),
                       ftp.throughput.ci.width=CalculateCI(df$ftp.throughput),
                       ftp.transferrate.mean=mean(df$ftp.transferrate, na.rm=TRUE),
                       ftp.transferrate.ci.width=CalculateCI(df$ftp.transferrate),
                       http.delay.mean=mean(df$http.delay, na.rm=TRUE),
                       http.delay.ci.width=CalculateCI(df$http.delay),
                       http.throughput.mean=mean(df$http.throughput, na.rm=TRUE),
                       http.throughput.ci.width=CalculateCI(df$http.throughput),
                       http.transferrate.mean=mean(df$http.transferrate, na.rm=TRUE),
                       http.transferrate.ci.width=CalculateCI(df$http.transferrate),
                       video.dfr.mean=mean(df$video.dfr, na.rm=TRUE),
                       video.dfr.ci.width=CalculateCI(df$video.dfr)
                       )
            )
}
