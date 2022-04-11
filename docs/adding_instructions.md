1) Define instruction in markodwn docs && in instructions.hpp
2) Create an entry in the instruction_generator that will be leveraged by the assembler
3) Add instruction to assembler. Create function for handling the instruction, and add it to the regex->func map.
4) Add a test for the instruction in the instruction_generator tests
5) Create an object to represent the instruction in the execution_context / visitor pattern
6) Add the instruction to the vm loader
7) Add handling new method from step 5 to vm, and add its functionality 