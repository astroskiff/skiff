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

## Constant Type IDs :

These indicate to the reader how to interpret a segment of data when reading in a constant.

```
  u8 - 0x00, u16 - 0x01, u32 - 0x02, u64 - 0x03
  i8 - 0x10, i16 - 0x11, i32 - 0x12, i64 - 0x13
  float - 0x20
  string - 0x30
```

## Constant encodings:
```
Non-string constants
  [constant id] [number of bytes required to store item]

String constants
  [constant id] [8 byte length field] [string bytes]
```

## Section Table

```
[ Number of sections QWORD ]
[ Section Address QWORD ] [ Section Length QWORD ] [ Section Name String ]
[ Section Address QWORD ] [ Section Length QWORD ] [ Section Name String ]
[ Section Address QWORD ] [ Section Length QWORD ] [ Section Name String ]
...
```

# Executable Binary layout:
```

Binary compatibility DWORD

Debug Level BYTE

[Section Table]

Number of constants QWORD

[Encoded Constants]

Entry Address QWORD

Number of instructions QWORD

[Instructions]

```