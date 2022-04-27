.init fn_main

.string num_prompt "Enter a number> "
.string txt_prompt "Enter some text (len 20 max)> "
.string out_prompt "You wrote> "
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

; r5 Type
; r6 Location
; r7 Length
; i3 source slot
fn_print_item:
  mov i0 @0
  mov i1 @2       ; Word size in bytes  
  sw i8 i0 i0     ; Store `0` into command slot 
  
  add i0 i0 i1    ; Increase slot index 
  sw i8 i0 i3     ; Indicate source slot is `0` (data directive)
  
  add i0 i0 i1  
  sw i8 i0 i6     ; Load Location

  add i0 i0 i1  
  sw i8 i0 i5     ; Load type

  add i0 i0 i1  
  sw i8 i0 i7     ; Load length

  add i0 i0 i1  
  mov i2 @1       ; Send to stdout
  sw i8 i0 i2

  add i0 i0 i1
  mov i2 @1       ; Print new line
  sw i8 i0 i2

  push_qw i8      ; Load command slot into i0 
  pop_qw i0 

  mov i1 @0       ; Load command offset

  syscall 2
  ret

fn_print_items:
  mov i3 @0     ; Source slot 0

  mov i5 @9     ; ASCII
  mov i6 &str   ; Address
  mov i7 #str   ; Len
  call fn_print_item

  mov i6 &str_0   ; Address
  mov i7 #str_0   ; Len
  call fn_print_item

  mov i6 &str_1   ; Address
  mov i7 #str_1   ; Len
  call fn_print_item

  mov i6 &str_2   ; Address
  mov i7 #str_2   ; Len
  call fn_print_item

  mov i6 &str_3   ; Address
  mov i7 #str_3   ; Len
  call fn_print_item

  mov i5 @0       ; U8
  mov i6 &a       ; Address
  mov i7 #a       ; Len
  call fn_print_item

  mov i5 @1       ; I8
  mov i6 &b       ; Address
  mov i7 #b       ; Len
  call fn_print_item

  mov i5 @2       ; U16
  mov i6 &c       ; Address
  mov i7 #c       ; Len
  call fn_print_item

  mov i5 @3       ; I16
  mov i6 &d       ; Address
  mov i7 #d       ; Len
  call fn_print_item

  mov i5 @4       ; U32
  mov i6 &e       ; Address
  mov i7 #e       ; Len
  call fn_print_item

  mov i5 @5       ; I32
  mov i6 &f       ; Address
  mov i7 #f       ; Len
  call fn_print_item

  mov i5 @6       ; U64
  mov i6 &g       ; Address
  mov i7 #g       ; Len
  call fn_print_item

  mov i5 @7       ; I64
  mov i6 &h       ; Address
  mov i7 #h       ; Len
  call fn_print_item

  mov i5 @8       ; Float
  mov i6 &pi      ; Address
  mov i7 #pi      ; Len
  call fn_print_item

  mov i5 @8       ; Float
  mov i6 &npi     ; Address
  mov i7 #npi     ; Len
  call fn_print_item
  ret

; r5 Type
; r9 Destination
; r7 Length
fn_get_item:
  mov i0 @0
  mov i1 @2       ; Word size in bytes  
  sw i8 i0 x1     ; Store `1` into command slot 
  
  add i0 i0 i1    ; Increase slot index 
  sw i8 i0 i9     ; Indicate input buffer (i6)
  
  add i0 i0 i1  
  sw i8 i0 x0     ; Load offset to start placing in buffer (0)

  add i0 i0 i1  
  sw i8 i0 i5     ; Load type

  add i0 i0 i1  
  sw i8 i0 i7     ; Load length

  push_qw i8      ; Load command slot into i0 
  pop_qw i0 

  mov i1 @0       ; Load command offset

  syscall 2
  ret

fn_get_items:
  ; prompt
  mov i5 @9            ; ASCII
  mov i6 &txt_prompt   ; Address
  mov i7 #txt_prompt   ; Len
  call fn_print_item

  mov i5 @9             ; Get a string
  mov i7 @20            ; String of length 20
  call fn_get_item

  asne x0 op            ; Ensure we got something
  push_qw op            ; Save for printing it 

  mov i3 @0
  mov i5 @9            ; ASCII
  mov i6 &out_prompt   ; Address
  mov i7 #out_prompt   ; Len
  call fn_print_item

  mov i3 @2            ; Source for printing input
  mov i5 @9            ; ASCII
  mov i6 @0            ; Address

  pop_qw i7 

  call fn_print_item

  ret

fn_main:

  mov i0 @16    ; 16 Bytes for command slot 
  alloc i8 i0   ; Allocate slot and get id in i8 

  call fn_print_items

  mov i0 @1024  ; Allocate an input buffer
  alloc i9 i0 
  call fn_get_items

  mov i0 @0
  exit