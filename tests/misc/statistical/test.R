#
# Test if one batch of simulations produced statistically the same result as another batch,
# by looking at the recorded output scalars (histograms and vectors are ignored).
#
# Purpose: as regression test for cases where simpler methods such as fingerprint checks fail.
# (Certain code changes, such as rearranging modules, changing random number usage or 
# self-message scheduling alter the fingerprint but do not change model behavior, 
# i.e. simulations should produce statistically the same results as before. After such
# changes, this routine can be used to check that model behavior was not affected.)
# 
# The routine looks at each scalar, and performs Student t-test to check that the sample
# means from the two simulation batches match, then F-test to check that the variances 
# also match. If both tests accept for all scalars, then the two simulations match.
#
# Both t-test and F-test assume that the variables are normally distributed. Therefore,
# first we perform a normality test, and skip t-test and F-test if normality test rejects.
#
# In our experience, about 50 runs per batch are enough to satisfy the test.
# If there are less, e.g. 10 runs, then mismatch (reject) is likely to be reported 
# for some of the scalars. The test can be made more permissive by setting meantest.alpha 
# (the p-value threshold for the t-test) to be smaller, e.g. 0.025 instead of 0.05,
# of course on the cost of decreasing the reliability of the test.
#
# TO TRY:
#
# $ cd ~/inet/examples/inet/nclients
# $ ./run -f basicHTTP.ini -u Cmdenv --repeat=100 -r0..49  --result-dir=results1 --vector-recording=false
# $ ./run -f basicHTTP.ini -u Cmdenv --repeat=100 -r50..99 --result-dir=results2 --vector-recording=false
# $ Rscript ../../../tests/misc/statistical/test.R results1 results2
#

require(optparse, quietly = TRUE)
require(reshape, quietly = TRUE)
require(omnetpp, quietly = TRUE)

testScalars <- function(description,
    scalars1, scalars2, # scalars to compare
    exclude = NULL,    # excluded scalars
    normalitytest.alpha = 0.05, meantest.alpha = 0.05, variancetest.alpha = 0.05, # statistical test parameters 
    logLevel = 3)
{
    numExclude <- 0
    numExactMatch <- 0
    numAccept <- 0
    numReject <- 0
    numNReject <- 0
    numTReject <- 0
    numFReject <- 0
    pass <- TRUE

    if (logLevel > 0)
        cat("Testing '", description, "'\n", sep="")

    # extract module-name pairs (they are the variables we're going to test one by one)
    scalarnames1 <- unique(data.frame(module = scalars1$module, name = scalars1$name))
    scalarnames2 <- unique(data.frame(module = scalars2$module, name = scalars2$name))
    
    if (logLevel > 0) cat("Processing ", length(scalarnames1$name), " scalars from '", resultsdir1, "'\n", sep="")
    if (logLevel > 1) print(scalarnames1)

    if (logLevel > 0) cat("Processing ", length(scalarnames2$name), " scalars from '", resultsdir2, "'\n", sep="")
    if (logLevel > 1) print(scalarnames2)

    # remove excluded scalars
    if (!is.null(exclude)) {
        excludedScalars1 <- subset(scalarnames1, grepl(exclude, name))
        excludedScalars2 <- subset(scalarnames2, grepl(exclude, name))
        numExclude1 <- length(excludedScalars1$name)
        numExclude2 <- length(excludedScalars2$name)
        if (numExclude1 > 0) {
            if (logLevel > 0) cat("Excluding ", numExclude1, " scalars from '", resultsdir1, "'\n", sep="")
            if (logLevel > 1) print(excludedScalars1)
        }
        if (numExclude2 > 0) {
            if (logLevel > 0) cat("Excluding ", numExclude2, " scalars from '", resultsdir2, "'\n", sep="")
            if (logLevel > 1) print(excludedScalars2)
        }
        numExclude <- numExclude1 + numExclude2
        scalarnames1 <- subset(scalarnames1, !grepl(exclude, name))
        scalarnames2 <- subset(scalarnames2, !grepl(exclude, name))
    }

    # check if the two datasets have the exact same set of scalars
    a1 <- paste(scalarnames1$module, scalarnames1$name)
    a2 <- paste(scalarnames2$module, scalarnames2$name)
    onlyInScalars1 <- setdiff(a1, a2)
    onlyInScalars2 <- setdiff(a2, a1)
    numAdditional1 <- length(onlyInScalars1)
    numAdditional2 <- length(onlyInScalars2)
    numAdditional <- numAdditional1 + numAdditional2 

    if (numAdditional1 > 0) {
        if (logLevel > 0) cat("Found ", numAdditional1, " additional scalars from '", resultsdir1, "'\n", sep="")
        if (logLevel > 1) print(onlyInScalars1)
    }
    if (numAdditional2 > 0) {
        if (logLevel > 0) cat("Ignoring ", numAdditional2, " additional scalars from '", resultsdir2, "'\n", sep="")
        if (logLevel > 1) print(onlyInScalars2)
    }

    scalarnames <- merge(scalarnames1, scalarnames2, all = FALSE)

    if (logLevel > 0) cat("Comparing", length(scalarnames$name), "scalars found in both resultsets\n")
    if (logLevel > 1) print(scalarnames)

    rejectedScalars <- list()

    # loop through the list of scalars, and test each scalar one by one
    for (i in 1:nrow(scalarnames)) {
    
        module <- scalarnames$module[i]
        name <- scalarnames$name[i]

        values1 <- scalars1[scalars1$name == as.character(name) & scalars1$module == as.character(module),]$value
        values2 <- scalars2[scalars2$name == as.character(name) & scalars2$module == as.character(module),]$value

        if (logLevel > 2) {
            cat("\nProcessing scalar:", as.character(module), as.character(name), "\n")
            print(values1)
            print(values2)
        }

        nameAndValuesString <- paste(as.character(module), " ", as.character(name), " VALUES: [", paste(values1,collapse=', '), "] AND [", paste(values2,collapse=', '), "]")
        
        
        # remove NaNs
        if (sum(is.na(values1)) != sum(is.na(values2))) {
            if (logLevel > 2) cat('    DIFFERENT NUMBER OF NANS, REJECT\n')
            numReject <- numReject + 1
            next
        }
        else {
            values1 <- values1[!is.na(values1)]
            values2 <- values2[!is.na(values2)]
        }
     
        # same numbers, in maybe different order?
        if (length(values1) == length(values2) && all(sort(values1) == sort(values2))) {
            if (logLevel > 2) cat('    EXACT MATCH, ACCEPT\n')
            numExactMatch <- numExactMatch + 1
            next
        }

        # same constant number in both? (must test it here, as t.test() throws "data are 
        # essentially constant" error for this case)
        if (min(values1) == max(values1) && min(values2) == max(values2)) {
            if (values1[1]==values2[1]) {
                if (logLevel > 2) cat('    SAME CONSTANTS, ACCEPT\n')
                numExactMatch <- numExactMatch + 1
            } else {
                if (logLevel > 2) cat('    DIFFERENT CONSTANTS, REJECT\n')
                numReject <- numReject + 1
                rejectedScalars <- append(rejectedScalars, nameAndValuesString)
            }
            next
        }

        # t-test requires that variables be normally distributed, so we must test that first
        # Note: it is not required that their variances be the same, as t.test() has a var.equal=FALSE argument
        
        # step 1: normality test (not needed [in fact causes an error] if all values are the same)
        if (min(values1) != max(values1)) {
            t1 <- stats::shapiro.test(values1)
            if (logLevel > 2) cat("normality test sample1 p-value:", t1$p.value, "\n")
            if (t1$p.value < normalitytest.alpha) {
                if (logLevel > 2) cat('    REJECT\n')   # t-test and F-test only work if values are normally distributed
                numNReject <- numNReject + 1
                next
            }
        }
        if (min(values2) != max(values2)) {
            t2 <- stats::shapiro.test(values2)
            if (logLevel > 2) cat("normality test sample2 p-value:", t2$p.value, "\n")
            if (t2$p.value < normalitytest.alpha) {
                if (logLevel > 2) cat('    REJECT\n')   # t-test and F-test only work if values are normally distributed
                numNReject <- numNReject + 1
                next
            }
        }

        # step 2: t-test for mean
        t <- t.test(values1, values2)
        if (logLevel > 2) cat("mean test p-value:", t$p.value)
        if (t$p.value < meantest.alpha) {
            if (logLevel > 2) cat('    REJECT\n')
            numTReject <- numTReject + 1
            rejectedScalars <- append(rejectedScalars, nameAndValuesString)
            next
        }

        # step 3: F-test for variance (t.test() only checks the mean)
        v <- var.test(values1, values2)
        if (logLevel > 2) cat("variance test p-value:", v$p.value, "\n")
        if (v$p.value < variancetest.alpha) {
            if (logLevel > 2) cat('    REJECT\n')
            numFReject <- numFReject + 1
            rejectedScalars <- append(rejectedScalars, nameAndValuesString)
            next
        }

        if (logLevel > 2) cat('    ACCEPT\n')
        numAccept <- numAccept + 1
    }

    if (logLevel > 2) cat("\n")

    if (logLevel > 0 && length(rejectedScalars) > 0) {
        cat("Found", length(rejectedScalars), "different scalars:\n");
        if (logLevel > 1) print(rejectedScalars)
    }

    pass <- (numAdditional == 0 && numReject == 0 && numNReject == 0 && numTReject == 0 && numFReject == 0 && numExclude + numExactMatch + numAccept > 0)
    if (logLevel > 0)
        cat("Success summary:", numExclude, "excluded,", numExactMatch, "exactly matched,", numAccept, "accepted", "\n")
    if (logLevel > 0 || !pass)
        cat("Failure summary:", numAdditional, "additional,", numReject, "differing constants,", numNReject, "rejected normality tests,", numTReject, "rejected t-tests,", numFReject, "rejected F-tests", "\n")
    cat("Test result for '", description, "' : ", sep="")
    if (pass) cat("PASS\n") else cat("FAIL\n")
    pass
}

# parse options and print help when necessary
optionSpecifications <- list(
   make_option(c("-v", "--verbose"), action = "store", default = 0, help = "Set output level of detail"),
   make_option(c("-n", "--normality-test-alpha"), action = "store", default = 0.05, help = "Set normality test threshold"),
   make_option(c("-m", "--mean-test-alpha"), action = "store", default = 0.05, help = "Set mean test (t test) threshold"),
   make_option(c("-a", "--variance-test-alpha"), action = "store", default = 0.05, help = "Set variance test (f test) threshold"),
   make_option(c("-t", "--test"), action = "store", default = NULL, help = "Test matching simulation results only"),
   make_option(c("-x", "--exclude-scalars"), action = "store", default = NULL, help = "Exclude matching scalars from testing"))
optionParser <- OptionParser(usage = "%prog [options] <results-1> <results-2>", option_list = optionSpecifications)
parsedArgs <- parse_args(optionParser, args = commandArgs(trailingOnly = TRUE), print_help_and_exit = TRUE, positional_arguments = TRUE)
options <- parsedArgs$options
args <- parsedArgs$args

if (length(args) != 2) {
    print_help(optionParser)
    q()
}

options(width=120)

# results directories
resultsdir1 <- args[1]
resultsdir2 <- args[2]
if (options$verbose > 0) cat("Statistical scalar comparison test started\n", sep="")

# read results
if (options$verbose > 1) cat("Loading results from '", resultsdir1, "'\n", sep="")
results1 <- loadDataset(paste(resultsdir1, "*.sca", sep="/"))
if (options$verbose > 1) cat("Loading results from '", resultsdir2, "'\n", sep="")
results2 <- loadDataset(paste(resultsdir2, "*.sca", sep="/"))

# preparing results
wideattributes1 <- cast(results1$runattrs, runid~attrname, value='attrvalue')
wideattributes2 <- cast(results2$runattrs, runid~attrname, value='attrvalue')
scalars1 <- results1$scalars
scalars2 <- results2$scalars 
widescalars1 <- merge(scalars1, wideattributes1)
widescalars2 <- merge(scalars2, wideattributes2)
keycolumns <- c('experiment', 'measurement', 'inifile', 'configname')
tests1 <- widescalars1[!duplicated(widescalars1[,keycolumns]), keycolumns]
tests2 <- widescalars2[!duplicated(widescalars2[,keycolumns]), keycolumns]
tests <- merge(tests1, tests2, all = TRUE)
if (!is.null(options$test))
    tests <- subset(tests, grepl(options$test, experiment))

invisible(by(tests, 1:nrow(tests), function(key) {
    cat("\n"); 
    selectedscalars1 <- subset(widescalars1, as.character(experiment) == key$experiment & as.character(measurement) == key$measurement & as.character(inifile) == key$inifile & as.character(configname) == key$configname)
    selectedscalars2 <- subset(widescalars2, as.character(experiment) == key$experiment & as.character(measurement) == key$measurement & as.character(inifile) == key$inifile & as.character(configname) == key$configname)
    description <- paste(key$experiment, key$inifile, "[", key$configname, "]", key$measurement, sep="")
    testScalars(description, selectedscalars1, selectedscalars2,
        exclude = options$exclude, 
        normalitytest.alpha = options$`normality-test-alpha`,
        meantest.alpha = options$`mean-test-alpha`,
        variancetest.alpha = options$`variance-test-alpha`, 
        logLevel = options$verbose)
}))

if (options$verbose > 0) cat("Statistical scalar comparison test finished\n", sep="")
