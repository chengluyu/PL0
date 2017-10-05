<p align="center"><img src="https://raw.githubusercontent.com/chengluyu/PL0/master/pl0.png" width=200></p>

-----

## Introduction

This repository holds my implementation of PL/0 language interpreter. This project is intended for the experiment of my compiler theory course in my university.

You can see the full grammar (written in BNF) of common PL/0 language at [Wikipedia](https://en.wikipedia.org/wiki/PL/0). However, the implementation in this repository is slightly different from the common one. This is mainly because the experiment introduces more requirements at the base of the original version, which is listed below:

1. Procedures can be nested, which indicates that you can define a sub-procedure in a procedure. The inner procedures do have the access to symbols that defined in outer procedures.
2. The parser should not generate abstract syntax trees, instead it should generate byte codes directly. 
3. A virtual machine should be done in order to simulate the execution of byte codes.
