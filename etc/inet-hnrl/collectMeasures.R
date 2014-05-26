collectMeasures <- function(scalars,
                            formulaString,
                            modulePattern,
                            namePattern,
                            colNames,
                            measureType)
{
###
### Extract performance measures from a given data frame based on
### OMNeT++ scalar files, calculate their means and confidence
### intervals, and rearrange them in the resulting data frame for
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
###   a data frame including columns given in 'colNames', 'mean',
###   'ci.width', and 'measure.type'
###
    tmp <- cast(scalars, as.formula(formulaString), c(mean, ci.width), subset=grepl(modulePattern, module) & grepl(namePattern, name))
    tmp <- subset(tmp, select = c(colNames, 'name', 'mean', 'ci.width'))
    numCols <- length(colNames)
    for (i in 1:numCols) {
        ## convert factor columns (e.g., 'dr' and 'n') into numeric ones
        tmp <- cbind(tmp, eval(parse(text=as.character(eval(parse(text=paste('tmp$', colNames[i], sep='')))))))
        ## tmp <- cbind(tmp, as.numeric(as.character(eval(parse(text=paste('tmp$', colNames[i], sep=''))))))
    }
    tmp <- subset(tmp, select=c(1:numCols+(3+numCols), 1:3+numCols))
    names(tmp)[1:length(colNames)]=colNames
    tmp <- sort_df(tmp, vars=colNames)
    tmp['measure.type'] <- rep(measureType, length(tmp$mean))
    return(tmp)
}
