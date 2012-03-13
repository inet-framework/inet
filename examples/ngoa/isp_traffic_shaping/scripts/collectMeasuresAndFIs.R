collectMeasuresAndFIs <- function(scalars,
                                  formulaString,
                                  modulePattern,
                                  namePattern,
                                  colNames,
                                  measureType)
{
###
### Extract performance measures from a given data frame based on
### OMNeT++ scalar files, calculate their means and confidence
### intervals and Raj Jain's fairness index of the means per module,
### and rearrange them in the resulting data frame for further
### processing.and rearrange them in the resulting data frame for
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
###   'ci.width', 'fairness.index', and 'measure.type'
###
    ## first, extract performance measures, their means and confidence intervals
    tmp1 <- cast(scalars, as.formula(formulaString), c(mean, ci.width), subset=grepl(modulePattern, module) & grepl(namePattern, name))
    tmp1 <- subset(tmp1, select = c(colNames, 'name', 'mean', 'ci.width'))
    numCols <- length(colNames)
    for (i in 1:numCols) {
        ## convert factor columns (e.g., 'dr' and 'n') into numeric ones
        tmp1 <- cbind(tmp1, as.numeric(as.character(eval(parse(text=paste('tmp1$', colNames[i], sep=''))))))
    }
    tmp1 <- subset(tmp1, select=c(1:numCols+(3+numCols), 1:3+numCols))
    names(tmp1)[1:length(colNames)]=colNames
    tmp1 <- sort_df(tmp1, vars=colNames)
    tmp1['measure.type'] <- rep(measureType, length(tmp1$mean))

    ## second, extract performance measures per module and fairness indexes of their means
    tmp2 <- calculateFairnessIndexes(scalars, formulaString, modulePattern, namePattern, colNames, measureType)

    ## combine the two data frames into one after sorting
    tmp1 <- sort_df(tmp1, vars=c('N', 'dr', 'mr', 'bs', 'n', 'name'))
    tmp2 <- sort_df(tmp2, vars=c('N', 'dr', 'mr', 'bs', 'n', 'name'))
    tmp1['fairness.index'] <- tmp2$fairness.index

    return(tmp1)
}
