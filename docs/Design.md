# VM Layout

## Registers:


|    Register  |  Byte code  |  Description  |
|----|----|----|----|
| x0 | 0x00 | `Read-only` '0' constant |
| x1 | 0x00 | `Read-only` '1' constant |
| ip | 0x02 | `Read-only` Instruction pointer |
| fp | 0x03 | `Read-only` Stack pointer |
| i0 | 0x10 | Integer Register |
| i1 | 0x11 | Integer Register |
| i2 | 0x12 | Integer Register |
| i3 | 0x13 | Integer Register |
| i4 | 0x14 | Integer Register |
| i5 | 0x15 | Integer Register |
| i6 | 0x16 | Integer Register |
| i7 | 0x17 | Integer Register |
| i8 | 0x18 | Integer Register |
| i9 | 0x19 | Integer Register |
| f0 | 0x20 | Floating Point Register |
| f1 | 0x21 | Floating Point Register |
| f2 | 0x22 | Floating Point Register |
| f3 | 0x23 | Floating Point Register |
| f4 | 0x24 | Floating Point Register |
| f5 | 0x25 | Floating Point Register |
| f6 | 0x26 | Floating Point Register |
| f7 | 0x27 | Floating Point Register |
| f8 | 0x28 | Floating Point Register |
| f9 | 0x29 | Floating Point Register | 


# ASM Directives

## .data

**Description:**
Should be the first thing listed in a file. Starts the data segment where all of the other directives should exist.

**Example:**
```
  .data
```

##  .init

**Description:**
Used to define the name of the first label to start execution from

**Example:**
```
  .init main
```

## .string

**Description:**
Defines a string 

**Example:**
```
  .string string_name "Body of the string"
```

Encoded strings in the data section take the form: 

```
	<unsigned 16-bit 'length'> <string data> 
```

## .i8 .. .i64, .u8 .. u64 

**Description:**
Defines a specifically sized integer

**Example:**
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

**Description:**
Defines a floating point number

**Example:**
```
  .float pi 3.14159
```

## .code

**Description:**
Indicates the start of 'code' space. No more directives shall follow this

**Example:**
```
  .code
```

# Symbol Usage

| Symbol | Description |
|----|----
| & | Address of constant / label
| $ | Value of constant
| # | Length of constant 
| @ | Raw 64-bit value

# Instructions

## Labels

**Format:** [label_name]:
**Description:** Labels mark locations within instructions, and have no direct encoding within the instruction space. 
**Example:** `my_label:`

## nop
**Opcode:** 0x00
**Instruction Layout:**
```
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `nop`
**Description:** A no-operation. This instruction does nothing.
**Example:**	`nop`

## exit
**Opcode** 0x01
**Instruction Layout:**
```
	[ ------------- Empty ---------- ]  [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0001 

	[ -------------- Return Code -------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `exit <signed 64-bit number>`
**Description:** Exit execution with a return code
**Example:**	`exit 1`

## blt
**Opcode** 0x02
**Instruction Layout:**
```
	[ Empty ]  [ LHS Reg ] [ RHS Reg ] [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0010 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `blt <register lhs> <register rhs> <address>`
**Description:** Compare two registers. These can be integer or float registers. Branch to given address iff `lhs < rhs`
**Example:**	`blt i0 i1 &label_name	; Branch to a label`

## bgt
**Opcode** 0x03
**Instruction Layout:**
```
	[ Empty ]  [ LHS Reg ] [ RHS Reg ] [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0011 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `bgt <register lhs> <register rhs> <address>`
**Description:** Compare two registers. These can be integer or float registers. Branch to given address iff `lhs > rhs`
**Example:**	`blt i0 i1 &label_name	; Branch to a label`

## beq
**Opcode** 0x04
**Instruction Layout:**
```
	[ Empty ]  [ LHS Reg ] [ RHS Reg ] [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0100 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `beq <register lhs> <register rhs> <address>`
**Description:** Compare two registers. These can be integer or float registers. Branch to given address iff `lhs = rhs`
**Example:**	`beq i0 i1 &label_name	; Branch to a label`
