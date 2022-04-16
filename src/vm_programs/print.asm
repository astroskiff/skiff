.init main

.string str  "Hello world!"
.u8     a     200
.i8     b    -10
.u16    c     300
.i16    d    -300
.u32    e     70000
.i32    f     429496729
.u64    g     9000000000
.i64    h    -922337203685477580
.float  pi    3.14159
.float  npi  -3.14159

; Convenience macros so we can see what the values `mean`

#macro SYSCALL_PRINTER    "syscall 0" ; Printer id '0'

#macro PRINTER_LOAD_U8    "mov i3 @0"
#macro PRINTER_LOAD_I8    "mov i3 @1"
#macro PRINTER_LOAD_U16   "mov i3 @2"
#macro PRINTER_LOAD_I16   "mov i3 @3"
#macro PRINTER_LOAD_U32   "mov i3 @4"
#macro PRINTER_LOAD_I32   "mov i3 @5"
#macro PRINTER_LOAD_U64   "mov i3 @6"
#macro PRINTER_LOAD_I64   "mov i3 @7"
#macro PRINTER_LOAD_FLOAT "mov i3 @8"
#macro PRINTER_LOAD_ASCII "mov i3 @9"

#macro PRINTER_DO_NEWLINE "mov i4 @1" ; Tell printer to add newline
#macro PRINTER_SLOT_ZERO  "mov i0 @0" ; Tell printer to use memory slot 0 for data

.code 

; Assumes i1 i2 and i3 are already populated
; with offset, length, and print type 
;
print_item:
  #PRINTER_SLOT_ZERO
  #PRINTER_DO_NEWLINE
  #SYSCALL_PRINTER
  aseq x1 op        ; Ensure it worked
  ret
 
; Print the string
;
print_str:
  mov i1 &str       ; Offset
  mov i2 #str       ; Length of item in words 
  #PRINTER_LOAD_ASCII
  call print_item
  ret

; Print all the integers
;
print_ints:
  mov i1 &a         ; Offset
  mov i2 #a         ; Length of item in words 
  #PRINTER_LOAD_U8
  call print_item

  mov i1 &b         ; Offset
  mov i2 #b         ; Length of item in words 
  #PRINTER_LOAD_I8
  call print_item

  mov i1 &c         ; Offset
  mov i2 #c         ; Length of item in words 
  #PRINTER_LOAD_U16
  call print_item

  mov i1 &d         ; Offset
  mov i2 #d         ; Length of item in words 
  #PRINTER_LOAD_I16
  call print_item

  mov i1 &e         ; Offset
  mov i2 #e         ; Length of item in words 
  #PRINTER_LOAD_U32
  call print_item

  mov i1 &f         ; Offset
  mov i2 #f         ; Length of item in words 
  #PRINTER_LOAD_I32
  call print_item

  mov i1 &g         ; Offset
  mov i2 #g         ; Length of item in words 
  #PRINTER_LOAD_U64
  call print_item

  mov i1 &h         ; Offset
  mov i2 #h         ; Length of item in words 
  #PRINTER_LOAD_I64
  call print_item
  ret

print_floats:
  mov i1 &pi         ; Offset
  mov i2 #pi         ; Length of item in words 
  #PRINTER_LOAD_FLOAT
  call print_item

  mov i1 &npi         ; Offset
  mov i2 #npi         ; Length of item in words  
  #PRINTER_LOAD_FLOAT
  call print_item
  ret

main:
  call print_str
  call print_ints
  call print_floats
  mov i0 @0
  exit

