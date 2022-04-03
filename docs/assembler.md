# assembler

The assembler is baked into libskiff for handling asm code. 

```

+-----------------+          +-------------+-------------+
|                 |          |             |             |
|                 |          |  Lib        |  Bin        |
|  Instruction    |          |  Generator  |  Generator  |
|                 |          |             |             |
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
|    Caller      |
|                |
+----------------+

```