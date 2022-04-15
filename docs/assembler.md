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

## Macros

Not really an instruction, but a way to ease the burden of writing asm programs, a macro takes the following forms:

```
#macro PRINT_CODE_ASCII "mov i3 @9"
#macro M_EXIT "mov i0 @0" \
              "exit"
```

Once the macros are created they can be used anywhere within the code as in the following example:

```
.init main
#macro M_EXIT "mov i0 @0" \
              "exit"
.code
main:
	#M_EXIT
```

which is equivalent to : 

```
.init main
.code
main:
	mov i0 @0
	exit
```