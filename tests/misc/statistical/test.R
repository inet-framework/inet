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
# first we perform a normality test, and skip t-test and F-test it if normality test
# rejects.
#
# We require that both simulation batches produce the same set of scalars. If that is
# not the case, some preprocessing is required to resolve that before invoking the test.
# For example, when simulating a scanario where nodes are dynamically created and deleted
# (i.e. pedestrians enter/leave the simulated area) and each node records statistics,
# each simulation will record a different set of scalars. This can be resolved by replacing
# per-node statistics by some aggregate statistics during preprocessing.
#
# In our experience, about 50 runs per batch are enough to satisfy the test.
# If there are less, e.g. 10 runs, then mismatch (reject) is likely to be reported 
# for some of the scalars. The test can be made more permissive by setting meantest.alpha 
# (the p-value threshold for the t-test) to be smaller, e.g. 0.025 instead of 0.05,
# of course on the cost of decreasing the relibability of the test.
#


#
# TO TRY:
#
# $ cd ~/inet/examples/inet/nclients
# $ ./run -f basicHTTP.ini -u Cmdenv --repeat=100 -r0..49  --result-dir=sample1 --vector-recording=false
# $ ./run -f basicHTTP.ini -u Cmdenv --repeat=100 -r50..99 --result-dir=sample2 --vector-recording=false
# $ Rscript ../../../tests/misc/statistical/test.R
#

require(omnetpp)

sample1 <- loadDataset("sample1/*.sca")
sample2 <- loadDataset("sample2/*.sca")

scalars1 <- sample1$scalars
scalars2 <- sample2$scalars


testScalars <- function(
    scalars1, scalars2, # data frames with at least $module, $name, $value columns
    normalitytest.alpha = 0.05, meantest.alpha = 0.05, variancetest.alpha = 0.05, 
    verbosity = 3) 
{
    # extract module-name pairs (they are the variables we're going to test one by one)
    scalarnames1 <- unique(data.frame(module = scalars1$module, name = scalars1$name))
    scalarnames2 <- unique(data.frame(module = scalars1$module, name = scalars1$name))

    # the two datasets must have exactly the same set of scalars. If they don't, 
    # the user is expected to do some preprocessing to iron out the differences
    a1 <- paste(scalarnames1$module, scalarnames1$name)
    a2 <- paste(scalarnames2$module, scalarnames2$name)
    onlyInValues1 = setdiff(a1,a2)
    onlyInValues2 = setdiff(a2,a1)
    if (length(onlyInValues1)!=0 | length(onlyInValues2)!=0) {
        if (verbosity>0) {
            cat("ERROR: sets of scalars differ in the two datasets\n")
            cat("only in the first:", onlyInValues1, "\n")
            cat("only in the second:", onlyInValues2, "\n")
        }
        throw("sets of scalars differ in the two datasets")
    }

    if (verbosity>1) {
        cat("Scalars tested:\n")
        print(scalarnames1)
    }
    
    numSkip = 0
    numExactMatch = 0
    numAccept = 0
    numReject = 0
    numFReject = 0

    rejectedScalars = character(0)

    # loop through the list of scalars, and test each scalar one by one
    for (i in 1:nrow(scalarnames1)) {
    
        module <- scalarnames1$module[i]
        name <- scalarnames1$name[i]

        values1 <- scalars1[scalars1$name == name & scalars1$module == module,]$value
        values2 <- scalars2[scalars2$name == name & scalars2$module == module,]$value

        if (verbosity>2) {
            cat("\nProcessing scalar:", as.character(module), as.character(name), "\n")
            print(values1)
            print(values2)
        }

        nameAndValuesString <- paste(as.character(module), " ", as.character(name), " VALUES: [", paste(values1,collapse=', '), "] AND [", paste(values2,collapse=', '), "]")
     
        # same numbers, in maybe different order?
        if (length(values1) == length(values2) & all(sort(values1) == sort(values2))) {
            if (verbosity>2) cat('    EXACT MATCH, ACCEPT\n')
            numExactMatch <- numExactMatch+1
            next
        }

        # same constant number in both? (must test it here, as t.test() throws "data are 
        # essentially constant" error for this case)
        if (min(values1)==max(values1) & min(values2)==max(values2)) {
            if (values1[1]==values2[1]) {
                if (verbosity>2) cat('    SAME CONSTANTS, ACCEPT\n')
                numExactMatch <- numExactMatch+1
            } else {
                if (verbosity>2) cat('    DIFFERENT CONSTANTS, REJECT')
                numReject <- numReject+1
                rejectedScalars <- append(rejectedScalars, nameAndValuesString)
            }
            next
        }

        # t-test requires that variables be normally distributed, so we must test that first
        # Note: it is not required that their variances be the same, as t.test() has a var.equal=FALSE argument
        
        # step 1: normality test (not needed [in fact causes an error] if all values are the same)
        if (min(values1)!=max(values1)) {
            t1 <- stats::shapiro.test(values1)
            if (verbosity>2) cat("normality test sample1 p-value:", t1$p.value, "\n")
            if (t1$p.value < normalitytest.alpha) {
                if (verbosity>2) cat('    SKIP\n')   # t-test and F-test only work if values are normally distributed
                numSkip <- numSkip+1
                next
            }
        }
        if (min(values2)!=max(values2)) {
            t2 <- stats::shapiro.test(values2)
            if (verbosity>2) cat("normality test sample2 p-value:", t2$p.value, "\n")
            if (t2$p.value < normalitytest.alpha) {
                if (verbosity>2) cat('    SKIP\n')   # t-test and F-test only work if values are normally distributed
                numSkip <- numSkip+1
                next
            }
        }

        # step 2: t-test for mean
        t <- t.test(values1, values2)
        if (verbosity>2) cat("mean test p-value:", t$p.value)
        if (t$p.value < meantest.alpha) {
            if (verbosity>2) cat('    REJECT')
            numReject <- numReject+1
            rejectedScalars <- append(rejectedScalars, nameAndValuesString)
            next
        }

        # step 3: F-test for variance (t.test() only checks the mean)
        v <- var.test(values1, values2)
        if (verbosity>2) cat("variance test p-value:", v$p.value, "\n")
        if (v$p.value < variancetest.alpha) {
            if (verbosity>2) cat('    REJECT')
            numFReject <- numFReject+1
            rejectedScalars <- append(rejectedScalars, nameAndValuesString)
            next
        }

        if (verbosity>2) cat('    ACCEPT\n')
        numAccept <- numAccept+1
    }

    if (verbosity>0) {
        cat("\nskip:", numSkip, "exact-match:", numExactMatch, "accept:", numAccept, "reject:", numReject, "F-reject:", numFReject, "\n\n")
        print(rejectedScalars)
    }
    
    numReject+numFReject == 0 & numExactMatch+numAccept > 0

}

#testScalars(scalars1, scalars2, meantest.alpha = 0.025, verbosity=1)
testScalars(scalars1, scalars2, meantest.alpha = 0.05, verbosity=3)

