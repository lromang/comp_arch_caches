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
# i = 4, 5, 6
for(i in 4:6){
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
# i = 7, 8, 9
for(i in 7:9){
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
