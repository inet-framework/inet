###
### Functions map line rates and TBF paramenters to configuration number
###

## configurations for unshaped cases
config <- function(dr, ur) {
    tmp <- rep(0, length(dr))
    for (i in 1:length(dr)) {
        if (dr[i] == 100 & ur[i] == 100) { # access line rate of 100 Mb/s
            config_number <- 1
        } else if (dr[i] == 1 & ur[i] == 1) { # access line rate of 1 Gb/s
            config_number <- 2
        } else {
            cat(sprintf("dr = %d and ur = %d\n", dr[i], ur[i]))
            stop("config(): Unknown value of 'dr' and 'ur'.")
        }
        tmp[i] <- paste('U', as.character(config_number), sep='')
    }
    return(tmp)
}

## configurations for shaped cases
config_tbf <- function(dr, ur, mr, bs) {
    tmp <- rep(0, length(dr))
    for (i in 1:length(dr)) {
        if (dr[i] == 100 & ur[i] == 100) { # access line rate of 100 Mb/s
            config_number1 <- 1
            if (mr[i] == 2 & bs[i] == 1) {
                config_number2 <- 1
            } else if (mr[i] == 2 & bs[i] == 10) {
                config_number2 <- 2
            } else if (mr[i] == 2 & bs[i] == 100) {
                config_number2 <- 3
            } else if (mr[i] == 10 & bs[i] == 1) {
                config_number2 <- 4
            } else if (mr[i] == 10 & bs[i] == 10) {
                config_number2 <- 5
            } else if (mr[i] == 10 & bs[i] == 100) {
                config_number2 <- 6
            } else if (mr[i] == 20 & bs[i] == 1) {
                config_number2 <- 7
            } else if (mr[i] == 20 & bs[i] == 10) {
                config_number2 <- 8
            } else if (mr[i] == 20 & bs[i] == 100) {
                config_number2 <- 9
            } else {
                cat(sprintf("mr = %d and bs = %d\n", mr[i], bs[i]))
                stop("config(): Unknown value of 'mr' and 'bs'.")
            }
        } else if (dr[i] == 1 & ur[i] == 1) { # access line rate of 1 Gb/s
            config_number1 <- 2
            if (mr[i] == 30 & bs[i] == 10) {
                config_number2 <- 1
            } else if (mr[i] == 30 & bs[i] == 100) {
                config_number2 <- 2
            } else if (mr[i] == 30 & bs[i] == 1000) {
                config_number2 <- 3
            } else if (mr[i] == 60 & bs[i] == 10) {
                config_number2 <- 4
            } else if (mr[i] == 60 & bs[i] == 100) {
                config_number2 <- 5
            } else if (mr[i] == 60 & bs[i] == 1000) {
                config_number2 <- 6
            } else if (mr[i] == 90 & bs[i] == 10) {
                config_number2 <- 7
            } else if (mr[i] == 90 & bs[i] == 100) {
                config_number2 <- 8
            } else if (mr[i] == 90 & bs[i] == 1000) {
                config_number2 <- 9
            } else {
                cat(sprintf("mr = %d and bs = %d\n", mr[i], bs[i]))
                stop("config(): Unknown value of 'mr' and 'bs'.")
            }
        } else {
            cat(sprintf("dr = %d and ur = %d\n", dr[i], ur[i]))
            stop("config(): Unknown value of 'dr' and 'ur'.")
        }
        tmp[i] <- paste('S', as.character(config_number1), as.character(config_number2), sep='')
    }   # end of for()
    return(tmp)
}
