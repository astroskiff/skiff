# VM Layout

## Registers:


|    Register  |  Byte code  |  Description  |
|----|----|----|
| x0 | 0x00 | `Read-only` '0' constant |
| x1 | 0x01 | `Read-only` '1' constant |
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

##  .init

**Description:**
*Required* Used to define the name of the first label to start execution from. 

**Example:**
```
  .init main
```

## .debug  

| Level | Meaning |
|----|----|
| 0 | No debug, same as not being defined |
| 1 | Minimal debug |
| 2 | Moderate debug |
| 3 | Extreme debug |

**Description:**
Indicates to the VM that debug mode should be enabled at a specified level. This directive is not required, but when set will force output debug messages
at runtime.

**Example:**
```
  .debug 1
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
	[ ------------- Empty ---------- ]  [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
	[ ------------------ Empty ----------------- ]
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

	[ ------------------ Empty ----------------- ]
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

**Format:** `blt <register lhs> <register rhs> <label>`
**Description:** Compare two registers. These can be integer or float registers. Branch to given address iff `lhs < rhs`
**Example:**	`blt i0 i1 label_name	; Branch to a label`
**Note:** In the future this instruction should be updated to allow the
currently empty byte to be a `variant` byte, and allow registers to be
used in the stead of labels. That way we can branch further than u32

## bgt
**Opcode** 0x03
**Instruction Layout:**
```
	[ Empty ]  [ LHS Reg ] [ RHS Reg ] [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0011 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `bgt <register lhs> <register rhs> <label>`
**Description:** Compare two registers. These can be integer or float registers. Branch to given address iff `lhs > rhs`
**Example:**	`blt i0 i1 label_name	; Branch to a label`
**Note:** In the future this instruction should be updated to allow the
currently empty byte to be a `variant` byte, and allow registers to be
used in the stead of labels. That way we can branch further than u32

## beq
**Opcode** 0x04
**Instruction Layout:**
```
	[ Empty ]  [ LHS Reg ] [ RHS Reg ] [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0100 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `beq <register lhs> <register rhs> <label>`
**Description:** Compare two registers. These can be integer or float registers. Branch to given address iff `lhs = rhs`
**Example:**	`beq i0 i1 label_name	; Branch to a label`
**Note:** In the future this instruction should be updated to allow the
currently empty byte to be a `variant` byte, and allow registers to be
used in the stead of labels. That way we can branch further than u32

## jmp
**Opcode** 0x05
**Instruction Layout:**
```
	[ ------------- Empty ---------- ]  [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0101 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `jmp <label>`
**Description:** Unconditionally jump to a label.
**Example:**	`jmp label_name`
**Note:** In the future this instruction should be updated to allow one
of the currently empty byte to be a `variant` byte, and allow registers 
to be used in the stead of labels. That way we can branch further than u32

## call
**Opcode** 0x06
**Instruction Layout:**
```
	[ ------------- Empty ---------- ]  [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0110 
	
	[ ---------------- Address ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `call <label>`
**Description:** Unconditionally go to a label, pushing the next address to the call stack. This will allow a return to the next instruction from the call destination.
**Example:**	`call label_name`

## ret
**Opcode** 0x07
**Instruction Layout:**
```
	[ ------------- Empty ---------- ]  [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0111 
	
	[ ------------------ Empty ----------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `ret`
**Description:** Return to the next address in the call stack. If the call stack is empty, execution will be halted.
**Example:**	`ret`

## mov
**Opcode** 0x08
**Instruction Layout:**
```
	[ ----- Empty ------ ]  [ Dest ]   [ Opcode ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 1000 
	
	[ ------------------ Value ---------------- ]
	0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 
```

**Format:** `mov <register> <source>`
**Description:** Move some data into a given register.

**Example:**	
```
	mov i0 &my_int	; Address of constant
	mov i0 &label	; Label address
	mov i0 #my_int	; Length of constant
	mov i0 @33		; Move raw value into i0
```

## add
**Opcode** 0x09
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1001 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `add <dest register> <lhs register> <rhs register>`
**Example:**	`add i0 i0 i1`
**Description:** Adds the value of two integer registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## sub
**Opcode** 0x0A
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1010 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `sub <dest register> <lhs register> <rhs register>`
**Example:**	`sub i0 i0 i1`
**Description:** Subtracts the value of two integer registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## div
**Opcode** 0x0B
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1011 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `div <dest register> <lhs register> <rhs register>`
**Example:**	`div i0 i0 i1`
**Description:** Divides the value of two integer registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## mul
**Opcode** 0x0C
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1100 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `mul <dest register> <lhs register> <rhs register>`
**Example:**	`mul i0 i0 i1`
**Description:** Multiplies the value of two integer registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## addf
**Opcode** 0x0D
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1101 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `addf <dest register> <lhs register> <rhs register>`
**Example:**	`addf f0 f0 f1`
**Description:** Adds the value of two floating point registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## subf
**Opcode** 0x0E
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1110 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `subf <dest register> <lhs register> <rhs register>`
**Example:**	`subf f0 f0 f1`
**Description:** Subtracts the value of two floating point registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## divf
**Opcode** 0x0F
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 1111 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `divf <dest register> <lhs register> <rhs register>`
**Example:**	`divf f0 f0 f1`
**Description:** Divides the value of two floating point registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## mulf
**Opcode** 0x10
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0000 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `mulf <dest register> <lhs register> <rhs register>`
**Example:**	`mulf f0 f0 f1`
**Description:** Multiplies the value of two floating point registers.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## lsh
**Opcode** 0x11
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0001 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `lsh <dest register> <lhs register> <rhs register>`
**Example:**	`lsh i0 i0 i1`
**Description:** Left shifts the value of a register by the value of another.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## rsh
**Opcode** 0x12
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0010 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `rsh <dest register> <lhs register> <rhs register>`
**Example:**	`rsh i0 i0 i1`
**Description:** Right shifts the value of a register by the value of another.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## and
**Opcode** 0x13
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0011 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `and <dest register> <lhs register> <rhs register>`
**Example:**	`and i0 i0 i1`
**Description:** Logical AND the value of a register by the value of another.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## or
**Opcode** 0x14
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0100 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `or <dest register> <lhs register> <rhs register>`
**Example:**	`or i0 i0 i1`
**Description:** Logical OR the value of a register by the value of another.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## xor
**Opcode** 0x15
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0101 
	
	[ Dest Reg ] [ LHS Reg ] [ RHS Reg ] [ Empty ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `xor <dest register> <lhs register> <rhs register>`
**Example:**	`xor i0 i0 i1`
**Description:** Logical XOR the value of a register by the value of another.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 

## not
**Opcode** 0x16
**Instruction Layout:**
```
	[ ------------- Empty ----------- ] [ Opcode ]
	0000 0000  | 0000 0000 | 0000 0000 | 0001 0110
	
	[ Dest Reg ] [  Reg   ] [ ------ Empty ----- ]
	0000 0000  | 0000 0000 | 0000 0000 | 0000 0000 
```
**Format:** `not <dest register> <register>`
**Example:**	`not i0 i0`
**Description:** Logical NOT the value of a register.
**Note:** Future expansion of this instruction is expected such that raw values could be encoded and a variant byte could be introduced to indicate that its not a register-only instruction. This would have potential execution speed ramifications. 
