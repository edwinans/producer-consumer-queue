# producer-consumer-queue
Multithread and thread-safe implementation of a producer-consumer system in C and Java


## Compile & Run
### C
`gcc -pthread main.c -o main.o`  
`./main.o`

### Java
> `javac Tapis.java`  
> 
> `java Tapis`  
> or  
>`java Tapis [n] [m] [t]`

where  
n = number of producers\
m = number of consumers\
t = number of targets to be produced by each procuers\
=> number of products = m x t