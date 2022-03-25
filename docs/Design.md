
### Use

A vm that should be easy to target from a high level language. 

*libskiff* should offer all interfaces and whatever is necessary to build, write-out, and verify skiff binaries.

skiff itsself will be an application to execute skiff binaries


# VM Layout

## Registers:

### General purpose registers

There are 3 primary register 'types' integer, float, and string.
These registers are for basic operations of the primitive types. 
There exists 10 registers for each given type. These registers are as follows:

```
     int registers: i0, i1, i2 ... i9
   float registers: f0, f1, f2 ... f9
  string registers: s0, s1, s2 ... s9
```

Bytecode values of each register:

```
  i0 : 0x00 | i1 : 0x01 | i2 : 0x02 | i3 : 0x03 | i4 : 0x04
  i5 : 0x05 | i6 : 0x06 | i7 : 0x07 | i8 : 0x08 | i9 : 0x09

  f0 : 0x10 | f1 : 0x11 | f2 : 0x12 | f3 : 0x13 | f4 : 0x14
  f5 : 0x15 | f6 : 0x16 | f7 : 0x17 | f8 : 0x18 | f9 : 0x19

  s0 : 0x20 | s1 : 0x21 | s2 : 0x22 | s3 : 0x23 | s4 : 0x24
  s5 : 0x25 | s6 : 0x26 | s7 : 0x27 | s8 : 0x28 | s9 : 0x29
```

As can be ssen above, each register of a type is prefixed by a letter which represents
the type of data it holds

### Read-only const registers

```
  r0 : 0x30   - Always contains '0'
  r1 : 0x31   - Always contains '1'
  ip : 0x32   - Current instruction address
```

### Read-only Device registers

```
  kb  : 0x40  - Keyboard input register (string data)
  > More will be added in the future to support disk/network i/o
```

### Operation registers

```
  fc : 0x50   - Fail check. '1' is written to this integer register if a conversion 
                operation fails (stoi, etc)
```

# ASM Directives

## .data

Should be the first thing listed in a file. Starts the data segment where all of the other directives should exist.

Example:

```
  .data
```

##  .init

Used to define the name of the first label to start execution from

Example:

```
  .init main
```

## .string

Defines a string 

Example:

```
  .string string_name "Body of the string"
```

## .i8 .. .i64, .u8 .. u64 

Defines a specifically sized integer

Example:

```
  .i8 int_one 42
  .i16 int_two 33
  .i32 int_three 99
  .i64 int_four 594

  .u8 unsigned_one 0
  .u16 unsigned_two 1
  .u32 unsigned_three 255
  .u64 unsigned_four 99000
```

## .float 

Defines a floating point number

Example:

```
  .float pi 3.14159
```

### Variable Directive Encoding

Each directive containing a variable will contain 2 bytes indicating byte length of the item,
followed by the bytes of the item in memory. This is to ensure that strings can be read in
from memory easily, but at the cost of a 2-byte overhead with all constants.

## .code

Indicates the start of 'code' space. No more directives shall follow this

Example:

```
  .code
```

# ASM Instructions

## Labels

*not _technically_ an instruction*

Lables mark areas within the code that can be accessed via various commands (more below)

Example:

```

some_label_name:

```

## nop

No operation 

Example:

```
  nop
```

Bytecode:

```
    0x00
  1 byte  
```

This instruction does nothing.

## mov

Move data into a register

Example:

```
  mov <register> <variable>
  mov <register> <register>
```

Any of the integer, float, or string registers can be placed as the first argument.
The second argument, the variable, will take one of three forms:

```
  mov i0 $var   ; Moves the value of 'var' into integer register 0
  mov i0 #var   ; Moves the length of 'var' (in bytes) into integer register 0
  mov i0 &value ; Moves the memory location of 'var' into integer register 0
  mov i0 i9     ; Moves the value from integer register 9 into integer register 0
```

Read-only registers are not be allowed for the first argument of a 'mov' instruction
but they are a valid second argument.

Bytecode:

```
'value' variant 

    0x01   | <register> |  <variable address>
  1 byte   |  1 byte    |      8 bytes 
  

'length' variant

    0x02   | <register> | <variable length>
  1 byte   |  1 byte    |      8 bytes

'address' variant 

    0x03   | <register> |  <variable address>
  1 byte   |  1 byte    |      8 bytes

'register' variant

    0x04   | <register> | <register>
  1 byte   |  1 byte    |  1 byte  
```

## jmp

Jump to a label unconditionally

Example:

```
  jmp label_name
```

Bytecode:

```
    0x05   | <label address>
  1 byte   |    8 bytes
```













## itof

Convert data in a float register into an integer.

Example:

```
  itof i0 f0 
```

Bytecode:

```
  0x06   | <int register> | <float register>
  1 byte |     1 byte     |      1 byte
```

The first argument must be an integer register. The second must be a float register.

## itos

Convert data in an int register into a string.

Example:

```
  itof i0 f0 
```

Bytecode:

```
  0x07   | <float register> | <int register>
  1 byte |     1 byte       |   1 byte
```

The first argument must be a float register. The second must be an integer register.















## ftoi

Convert data in an int register into an float.

Example:

```
  itof i0 f0 
```

Bytecode:

```
  0x07   | <float register> | <int register>
  1 byte |     1 byte       |   1 byte
```

The first argument must be a float register. The second must be an integer register.

























