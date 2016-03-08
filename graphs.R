#! /usr/bin/Rscript

## Gráficas
library(ggplot2)
library(plyr)
library(dplyr)
library(tidyr)
library(stringr)

## Read files
files <- list.files("./output")
files <- files[str_detect(files, ".csv")]
files <- paste0("./output/", files)

data  <- llply(files, read.csv)
names <- laply(files, function(t) t <- str_split(t, "/")[[1]][3])
names <- laply(names, function(t) t <- str_replace(t, ".csv",""))

## Associativity
# i = 1, 2, 3
for(i in 1:3){
    data_assoc <- gather(data[[i]][,-c(1,2)], cache_type, hit_rate, -associativity)
    png(paste0("./output/graphs/",names[i],".png"))
    ggplot(data = data_assoc, aes(x = associativity, y = hit_rate, col = cache_type)) +
        geom_point(size = 3) + geom_smooth(se = TRUE, span = .7, alpha=.3) +
        theme(panel.background = element_blank(),
              axis.title = element_text(colour = "#424242",
                                        face = "bold"),
              legend.text = element_text(colour = "#424242"),
              legend.title = element_text(colour = "#424242",
                                          face = "bold"),
              axis.text = element_text(colour = "#424242")) +
        scale_colour_manual(name   = "Cache",
                            labels = c("Datos", "Instrucciones"),
                            values = c("#1a237e", "#9e9e9e")) +
        xlab("Associatividad") + ylab("Tasa hit")
    dev.off()
}

## Block size
# i = 10, 11, 12
for(i in 10:12){
    data_assoc <- gather(data[[i]][,-c(1,3)], cache_type, hit_rate, -block_size)
    png(paste0("./output/graphs/",names[i],".png"))
    ggplot(data = data_assoc, aes(x = block_size, y = hit_rate, col = cache_type)) +
        geom_point(size = 3) + geom_smooth(se = FALSE, span = .7, alpha=.3) +
        theme(panel.background = element_blank(),
              axis.title = element_text(colour = "#424242",
                                        face = "bold"),
              legend.text = element_text(colour = "#424242"),
              legend.title = element_text(colour = "#424242",
                                          face = "bold"),
              axis.text = element_text(colour = "#424242")) +
        scale_colour_manual(name   = "Cache",
                            labels = c("Datos", "Instrucciones"),
                            values = c("#1a237e", "#9e9e9e")) +
        xlab("Tamaño de bloque") + ylab("Tasa hit")
    dev.off()
}

## Cache size
# i = 13, 14, 15
for(i in 13:15){
    data_assoc <- gather(data[[i]][,-c(2,3)], cache_type, hit_rate, -cache_size)
    png(paste0("./output/graphs/",names[i],".png"))
    ggplot(data = data_assoc, aes(x = cache_size, y = hit_rate, col = cache_type)) +
        geom_point(size = 3) + geom_smooth(se = FALSE, span = .7, alpha=.3) +
        theme(panel.background = element_blank(),
              axis.title = element_text(colour = "#424242",
                                        face = "bold"),
              legend.text = element_text(colour = "#424242"),
              legend.title = element_text(colour = "#424242",
                                          face = "bold"),
              axis.text = element_text(colour = "#424242")) +
        scale_colour_manual(name   = "Cache",
                            labels = c("Datos", "Instrucciones"),
                            values = c("#1a237e", "#9e9e9e")) +
        xlab("Tamaño de cache") + ylab("Tasa hit")
    dev.off()
}

## Bandwidth

## Write
write    <- rbind(data[[7]], data[[8]], data[[9]])
write$tot_traffic <- write[,5] + write[,6]
data_assoc <- gather(write[,-c(1,2,3,5,6)], cache_type, tot_traffic, -write_policy)
data_assoc$simul <- c(1,1,2,2,3,3)
## PNG
png(paste0("./output/graphs/write.png"))
ggplot(data = data_assoc, aes(x = simul, y = tot_traffic, col = write_policy)) +
    geom_point(size = 3) + geom_smooth(se = FALSE, span = .7, alpha=.3) +
    theme(panel.background = element_blank(),
          axis.title = element_text(colour = "#424242",
                                    face = "bold"),
          legend.text = element_text(colour = "#424242"),
          legend.title = element_text(colour = "#424242",
                                      face = "bold"),
          axis.text = element_text(colour = "#424242")) +
    scale_colour_manual(name   = "Política",
                        labels = c("Wb", "Wt"),
                        values = c("#1a237e", "#9e9e9e")) +
    xlab("Simul") + ylab("Tráfico")
dev.off()



## allocate
allocate <- rbind(data[[4]], data[[5]], data[[6]])
allocate$tot_traffic <- allocate[,5] + allocate[,6]
data_assoc <- gather(allocate[,-c(1,2,3,5,6)], cache_type, tot_traffic, -allocate_policy)
data_assoc$simul <- c(1,1,2,2,3,3)
## PNG
png(paste0("./output/graphs/allocate.png"))
ggplot(data = data_assoc, aes(x = simul, y = tot_traffic, col = allocate_policy)) +
    geom_point(size = 3) + geom_smooth(se = FALSE, span = .7, alpha=.3) +
    theme(panel.background = element_blank(),
          axis.title = element_text(colour = "#424242",
                                    face = "bold"),
          legend.text = element_text(colour = "#424242"),
          legend.title = element_text(colour = "#424242",
                                      face = "bold"),
          axis.text = element_text(colour = "#424242")) +
    scale_colour_manual(name   = "Política",
                        labels = c("Wb", "Wt"),
                        values = c("#1a237e", "#9e9e9e")) +
    xlab("Simul") + ylab("Tráfico")
dev.off()
