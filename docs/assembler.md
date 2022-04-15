# assembler

The assembler is baked into libskiff for handling asm code. 

```

+-----------------+
|                 |
|                 |
|  Instruction    |
|                 |
|  Generator      |          +-------------+-------------+
|                 |          |                           |
|                 |          |  Binary Generators        |
+--^-----------+--+          |                           |
   |           |             +-----^-------------+-------+
   |          Ins                  |             |
   |        Bytecode               |             |
   |           |                   |             |
+--+-----------v--+            Bytecode          |
|                 |                |             |
|                 +----------------+         Generated
|  Assembler      |                          Binary
|                 |                              |
|                 |                              |
|                 <------------------------------+
+--^----------+---+
   |          |
   |          |
 File       Binary
   |          |
+--+----------v--+
|                |
|    Caller      +------------> output.bin
|   (skiffd)     |
+-----^----------+
      |
      |
   input.asm



```