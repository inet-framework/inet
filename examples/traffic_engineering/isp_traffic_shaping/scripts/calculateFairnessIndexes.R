calculateFairnessIndexes <- function(scalars,
                                     formulaString,
                                     modulePattern,
                                     namePattern,
                                     colNames,
                                     measureType)
{
###
### Extract performance measures per module from a given data frame
### based on OMNeT++ scalar files, calculate Raj Jain's fairness index
### of their means, and rearrange them in the resulting data frame for
### further processing.
###
### Args:
###   scalars: a data frame made from OMNeT++ scalar files
###   formulaString: a string for a formula used for casting the data frame
###   modulePattern: a string for a regula expression for module match
###   namePattern: a string for a regular expression for name match
###   colNames: a vector of column names to appear in the resultilng data frame
###   measureType: a string for measure type for this processing
###
### Returns:
###   a data frame including columns given in 'colNames',
###   'fairness.index', and 'measure.type'
###
    tmp <- subset(scalars, grepl(modulePattern, module) & grepl(namePattern, name), select=c('runid', colNames, 'module', 'name', 'value'))
    tmp$module_number <- as.numeric(sub(".*host\\[([0-9]+)\\].*", "\\1", tmp$module))
    tmp <- sort_df(tmp, vars=c('runid', 'name', colNames, 'module_number'))
    ## tmp <- cast(scalars, as.formula(paste('module+', formulaString, sep="")), mean, subset=grepl(modulePattern, module) & grepl(namePattern, name))

    n <- min(as.numeric(as.character(tmp$n))) # N.B.: We assume that the values of N and n are unique; otherwise, it won't work.
    tmp <- cast(tmp, as.formula(paste('runid+', formulaString, sep="")), function(x) {v <- colSums(matrix(x, nrow=n));return(sum(v)^2/(length(v)*sum(v^2)))})
    ## tmp <- cast(tmp, as.formula(formulaString), c(mean, ci.width))
    tmp <- cast(tmp, as.formula(formulaString), mean) # now calculating only the mean of fairness indexes to be compatible with the older versions
    ## tmp <- cast(tmp, as.formula(formulaString), function(v) {return(sum(v)^2/(length(v)*sum(v^2)))})
    ## tmp <- subset(tmp, select = c(colNames, 'name', '(all)'))
    numCols <- length(colNames)
    for (i in 1:numCols) {
        ## convert factor columns (e.g., 'dr' and 'n') into numeric ones
        tmp <- cbind(tmp, as.numeric(as.character(eval(parse(text=paste('tmp$', colNames[i], sep=''))))))
    }
    tmp <- subset(tmp, select=c(1:numCols+(2+numCols), 1:2+numCols))
    names(tmp)[1:length(colNames)]=colNames
    names(tmp)[length(colNames)+2]='fairness.index'
    tmp <- sort_df(tmp, vars=colNames)
    tmp['measure.type'] <- rep(measureType, length(tmp$fairness.index))
    return(tmp)
}
