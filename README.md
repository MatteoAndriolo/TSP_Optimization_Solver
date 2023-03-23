This repository will contain our implementation of algorithms, presented during the OperationResearch2 course, used to solve the Travelling Salesman Problem (both exact, and heuristics)
We will also produce a thesis containing information about:
* Models
* implementations
* performance comparison

# Compilation
`make`

`make production` : without debug

# Execution
`bin/main` executable location

`-f data/...` input file

`-m nn.2opt` model/sequence of models to be executed in order, separated by `.`
* in this example is first executed nn then 2opt
* `nn`, `nng`(nn with grasp), `em`, `2opt`

`-n #` number of instances (defualt: `1`)

`-g x.y.z` sequence of integers (any number). They will be normalized in range `(0,1)`
* x is the probability for best, y for the second best, z for the thirs best, ...