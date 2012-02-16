###
### Parametric multi-variate noninferiority test (based on intersection-union test (IUT))
### between dedicated access and dedicated access with TBF
###

.da.df <- .da_nh1_nf0_nv1.df
.da_tbf.df <- .da_dr100M_nh1_nf0_nv1_tbf.df

.dr.range <- c(0.1)
.n.range <- unique(.da.df$n)
.mr.range <- unique(.da_tbf.df$msr)
.bs.range <- unique(.da_tbf.df$bs)
## .measure = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]', 'decodable frame rate (Q)')
.measure = c('average session delay [s]', 'decodable frame rate (Q)')
.msr.range <- 1:length(.measure)

.zeros <- rep(0, length(.dr.range)*length(.n.range)*length(.mr.range)*length(.bs.range)*length(.msr.range))
.da_tbf.nit <- data.frame(dr=.zeros, n=.zeros, mr=.zeros, bs=.zeros, msr=.zeros, nif=.zeros) # 'nif' stands for noninferiority test
.idx <- 1   # initialize index variable for data frame .da_tbf.nif
.epsilon <- 0.1 # for confidence limit

for (.dr in .dr.range) {
    for (.n in .n.range) {
        for (.mr in .mr.range) {
            for (.bs in .bs.range) {

                ##     ## initialize variables
                ##     .dr.range <- sort(unique(subset(.da_N16.org, n==.n)$dr), decreasing=T)  # sort 'dr' in decreasing order
                ## .i <- 0
                ## .r_prev <- 10.0
                ## .r <- 10.0
                ## .p.value <- 0.0
                ## .p0.value <- 0.0
                ## ## .y <- subset(.hp.org, select=c('http.delay'), n==.n & tx==.tx)$http.delay
                ## ## .y <- subset(.hp.org, select=c('http.throughput'), n==.n & tx==.tx)[[1]]
                .y <- subset(.hp.org, select=c(.measure[.msr]), n==.n & tx==.tx)[[1]]
                ## .ecr <- NA

                ## while (.i+1 <= length(.dr.range)) {
                ##     .i <- .i+1
                ##     .r <- .dr.range[.i]
                ##     ## .x <- subset(.da_N16.org, select=c('http.delay'), dr==.r & n==.n)$http.delay
                ##     ## .x <- subset(.da_N16.org, select=c('http.throughput'), dr==.r & n==.n)[[1]]
                .x <- subset(.da_N16.org, select=c(.measure[.msr]), dr==.r & n==.n)[[1]]

                ## IUT extension of noninferiority testing
                for (.i in .msr.range) {
                    .pass <- FALSE
                    .x <- subset(.da_N16.org, select=c(.measure[.i]), dr==.r & n==.n)[[1]]
                    .y <- subset(.hp.org, select=c(.measure[.i]), n==.n & tx==.tx)[[1]]

                    ## Noninferiority testing based on one-sided confidence
                    ## interval with the following confidence limits:
                    ## - For delay: > -.epsilon*mean(.x)
                    ## - For DFR:   < .epsilon*mean(.x)
                    if ((sum(is.na(.x)) < length(.x)) & (sum(is.na(.y)) < length(.y))) {
                        if (.measure[.i] == 'decodable frame rate (Q)') {
                            if (min(.x) == max(.x)) {
                                if (min(.y) == max(.y)) {
                                    if (min(.x) == min(.y)) {
                                        ## non-inferior testing succeeds
                                        .pass <- TRUE
                                    }
                                }
                            }
                            else {
                                if (t.test(.x, .y, conf.level=0.9)$conf.int[2] < .epsilon*mean(.x, na.rm=T)) {
                                    ## non-inferior testing succeeds
                                    .pass <- TRUE
                                }
                            }
                        }
                        else {
                            if (t.test(.x, .y, conf.level=0.9)$conf.int[1] > -.epsilon*mean(.x, na.rm=T)) {
                                ## non-inferior testing succeds
                                .pass <- TRUE
                            }
                        }
                    }
                    if (.pass == FALSE) {
                        break
                    }
                }   # end of for() for IUT

                if (.pass == TRUE) {
                    .ecr <- .r
                    break
                }

                ## assign the result to the data frame and update the index variable
                .hp_ecr_parametric$msr[.idx] <- .msr
                .hp_ecr_parametric$n[.idx] <- .n
                .hp_ecr_parametric$tx[.idx] <- .tx
                .hp_ecr_parametric$ecr[.idx] <- .ecr
                .hp_ecr_parametric$p.value[.idx] <- .p0.value
                .idx <- .idx+1
            }   # end of for(.bs)
        }   # end of for(.mr)
    }   # end of for(.n)
}   # end of for(.dr)

## plot results of multi-variate noninferiority testing
.hp_ecr_parametric.plots <- list()
.idx <- 1
for (.msr in .msr.range) {
    ## .p <- ggplot(data=.hp_ecr_parametric, aes(group=tx, linetype=factor(tx), x=n, y=ecr)) + geom_line()
    .p <- ggplot(data=subset(.hp_ecr_parametric, msr==.msr), aes(group=tx, colour=factor(tx), x=n, y=ecr)) + geom_line()
    ## .p <- .p + xlab("Number of Users per ONU (n)") + ylab("ECR [Gb/s]") + scale_linetype_manual(expression(N[tx]), values=c(1, 2, 4, 5, 6))
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab("ECR [Gb/s]") + scale_colour_discrete(expression(N[tx]))
    .p <- .p + geom_point(aes(shape=factor(tx)), size=.pt_size) + scale_shape_manual(expression(N[tx]), values=0:4)
    .p <- .p + opts(legend.key.size=unit(1.5, "lines"))
    .hp_ecr_parametric.plots[[.idx]] <- .p
    .idx <- .idx + 1

    ## .p <- ggplot(data=ddply(subset(.hp_ecr_parametric, select=c(1, 2), ecr>=10), .(n), .fun=min), aes(factor(n), weight=V1))
    ## .p <- .p + geom_bar(binwidth=0.6, fill="grey", colour="black")
    ## .p <- .p + xlab("Number of Users per ONU (n)") + ylab("Min. # of TXs to Achieve ECR of 10 Gb/s")
    ## .hp_ecr_parametric.plots[[2]] <- .p
    .ecr_target.range <- 5:10
    .zeros <- rep(0, length(.n.range)*length(.ecr_target.range))
    .hp_ecr_parametric.min_tx <-
        data.frame(n=rep(.n.range, length(.ecr_target.range)), ecr_target=rep(.ecr_target.range, each=length(.n.range)), tx=.zeros)
    .hp_ecr_parametric.min_tx$tx <-
        c(ddply(subset(.hp_ecr_parametric, select=c(1, 2), msr==.msr & ecr>=5), .(n), .fun=min)$V1,
          ddply(subset(.hp_ecr_parametric, select=c(1, 2), msr==.msr & ecr>=6), .(n), .fun=min)$V1,
          ddply(subset(.hp_ecr_parametric, select=c(1, 2), msr==.msr & ecr>=7), .(n), .fun=min)$V1,
          ddply(subset(.hp_ecr_parametric, select=c(1, 2), msr==.msr & ecr>=8), .(n), .fun=min)$V1,
          ddply(subset(.hp_ecr_parametric, select=c(1, 2), msr==.msr & ecr>=9), .(n), .fun=min)$V1,
          ddply(subset(.hp_ecr_parametric, select=c(1, 2), msr==.msr & ecr>=10), .(n), .fun=min)$V1)
    .p <-
        ggplot(data=.hp_ecr_parametric.min_tx, aes(group=ecr_target, colour=factor(ecr_target), x=n, y=tx)) + geom_line()
    ## .p <- .p + xlab("Number of Users per ONU (n)") + ylab("ECR [Gb/s]") + scale_linetype_manual(expression(N[tx]), values=c(1, 2, 4, 5, 6))
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(substitute(paste("min(", N[tx], ") to Achieve ECR of ", R[target], " Gb/s"))) + scale_colour_discrete(expression(R[target]))
    .p <- .p + geom_point(aes(shape=factor(ecr_target)), size=.pt_size) + scale_shape_manual(expression(R[target]), values=length(.ecr_target.range):1)
    .p <- .p + opts(legend.key.size=unit(1.5, "lines"))
    .hp_ecr_parametric.plots[[.idx]] <- .p
    .idx <- .idx+1
}   # end of for (.msr)
