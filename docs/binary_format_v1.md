# Binary pipeline

```
+------------------------+
|                        |
|   Assembler            |
|                        |
+------------------------+
|                        |
|   Bytecode Generator   |
|                        |
+-----------+------------+
            |
        Produces
            |
       +----v----+
       |         |
       | Binary  |
       |         |
       +----+----+
            |
       Is loaded by
            |
       +----v----+
       |         |
       | Loader  +----------------------+
       |         |                      |
       +----+----+                      |
            |                           |
       Creates a                   Creates a
            |                           |
            |                           |
       +----v----------------+   +------v--------+
       |                     |   |               |
       | Decoded Instruction |   |  Data Block   |
       |     List            |   |               |
       +----+----------------+   +------+--------+
            |                           |
            |      Is given to          |
            +--------------+------------+
                           |
                           |
                    +------v-----------+
                    |                  |
                    |   Skiff Virtual  |
                    |                  |
                    |     Machine      |
                    |                  |
                    +------------------+
```

The above demonstrates the means by which a binary is created, loaded, and turned into instructions. 
Once the `loader` creates an instruction list the binary representation of the instruction is no 
longer of consequence. This barrier at created by the loader ensures that different binary types 
could potentially be folded into the correct instruction set and encoding for a given vm. 

*Note:* The `data` encodings within the binary are not guaranteed to be representative of their 
encodings within the memory of the virtual machine. 


## Compatibility DWORD

A 32-bit unsigned word that indicates what version of binary format is being read in. This will ensure that
the execution of an old binary that may have an invalid instruction set isn't attempted. 

The value here is split up into 4 - 8 byte chunks that indicate the following:

```
   Byte 1: 
      0xFF Indicates if the binary is experimental and may crash. 
      0x00 Indicates the binary is stable.
   Byte 2:
      Major
   Byte 3:
      Minor
   Byte 4:
      Patch
```

## Data encodings:

All data must be at minumum `word` size (2 bytes). Any sub-word sized data must be prefixed with a `0x00` byte. 
The following are the types understood by the vm:
```
  u8, u16, u32, u64
  i8, i16, i32, i64
  float,
  string
```

All data encoded will simply be its byte representation at the aforementioned 2-byte padded minimum with the exception of strings.
Strings are encoded as follows:

```
   [8 byte length field] [un-padded chars] [potential 1-byte padding]
```

The string must be divisible by 2 so it can be read by the word. 
If the string is odd, a null byte `0x00` will be added to ensure that it is of even length.

## Section Table

```
[ Number of sections QWORD ]
[ Section Address QWORD ] [ Section Length QWORD ] [ Section Name String ]
[ Section Address QWORD ] [ Section Length QWORD ] [ Section Name String ]
.
.
.
[ Section Address QWORD ] [ Section Length QWORD ] [ Section Name String ]
...
```

## Interrupt Table
```
[ Number of interrupts QWORD ]
[ Interrupt Number QWORD ] [ Interrupt Address QWORD ]
[ Interrupt Number QWORD ] [ Interrupt Address QWORD ]
.
.
.
[ Interrupt Number QWORD ] [ Interrupt Address QWORD ]

```


# Executable Binary layout:
```

Binary compatibility DWORD

Debug Level BYTE

[Section Table]

[Interrupt Table]

Number of constants QWORD

[Encoded Constants]

Entry Address QWORD

Number of bytes constituting instructions QWORD

[Instructions]

```