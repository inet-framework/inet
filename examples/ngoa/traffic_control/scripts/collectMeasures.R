collectMeasures <- function(scalars, formula, modulePattern, namePattern, colNames) {
    tmp <- cast(scalars, as.formula(formula), c(mean, ci.width), subset=grepl(modulePattern, module) & grepl(namePattern, name))
    tmp <- subset(tmp, select = c(colNames, 'name', 'mean', 'ci.width'))

    numCols <- length(colNames)
    for (i in 1:numCols) {
        ## convert factor columns (e.g., 'dr' and 'n') into numeric ones
        tmp <- cbind(tmp, as.numeric(as.character(eval(parse(text=paste('tmp$', colNames[i], sep=''))))))
    }
    tmp <- subset(tmp, select=c(1:numCols+(3+numCols), 1:3+numCols))
    names(tmp)[1:length(colNames)]=colNames
    tmp <- sort_df(tmp, vars=colNames)

    return(tmp)
    ## tmp_ftp$traffic <- rep('ftp', length(tmp$mean))
}
