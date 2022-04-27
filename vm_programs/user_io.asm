.init fn_main

.string num_prompt "Enter a number : "
.string txt_prompt "Enter some text : "
.string str    "Hello world!"
.string str_0  "abcd"
.string str_1  " Josh"
.string str_2  "This is a test of the emergency alert system"
.string str_3  "!! Send'er bud"
.u8     a       200
.i8     b      -10
.u16    c       300
.i16    d      -300
.u32    e       70000
.i32    f       429496729
.u64    g       9000000000
.i64    h      -922337203685477580
.float  pi      3.14159
.float  npi    -3.14159

.code 

fn_print_strs:
  mov i0 @0
  mov i1 @2       ; Word size in bytes  
  sw i8 i0 i0     ; Store `0` into command slot 
  
  add i0 i0 i1    ; Increase slot index 
  sw i8 i0 x0     ; Indicate source slot is `0` (data directive)
  
  add i0 i0 i1  
  mov i2 &str     ; Store item offset in slot 
  sw i8 i0 i2 

  add i0 i0 i1  
  mov i2 @9       ; Indicate ascii print 
  sw i8 i0 i2

  add i0 i0 i1  
  mov i2 #str     ; Indicate length
  sw i8 i0 i2

  add i0 i0 i1  
  mov i2 @1       ; Send to stdout
  sw i8 i0 i2

  add i0 i0 i1
  mov i2 @1       ; Print new line
  sw i8 i0 i2

  push_qw i8      ; Load slot into i0 
  pop_qw i0 

  mov i1 @0       ; Load command offset

  syscall 2

  ret

fn_main:

  mov i0 @16    ; 16 Bytes for command slot 
  alloc i8 i0   ; Allocate slot and get id in i8 

  call fn_print_strs

  mov i0 @0
  exit