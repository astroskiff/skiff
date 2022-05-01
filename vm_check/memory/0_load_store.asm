.init main
.debug 3
.u8  a  250
.i8  b  120
.u16 c  300
.i16 d  900
.u32 e  70000
.i32 f  100000
.u64 g  9000000
.i64 h  90000002
.i8  i  -100
.code

load_data:
  ; Load words and ensure they match the value stated from memory
  mov i0 &a
  lw x0 i0 i9
  mov i0 @250
  aseq i0 i9

  mov i0 &b
  lw x0 i0 i9
  mov i0 @120
  aseq i0 i9

  mov i0 &c
  lw x0 i0 i9
  mov i0 @300
  aseq i0 i9

  mov i0 &d
  lw x0 i0 i9
  mov i0 @900
  aseq i0 i9

  ; Load double words and ensure they match the value stated from memory
  mov i0 &e
  ldw x0 i0 i9
  mov i0 @70000
  aseq i0 i9

  mov i0 &f
  ldw x0 i0 i9
  mov i0 @100000
  aseq i0 i9

  ; Load quad words and ensure they match the value stated from memory
  mov i0 &g
  lqw x0 i0 i9
  mov i0 @9000000
  aseq i0 i9

  mov i0 &h
  lqw x0 i0 i9
  mov i0 @90000002
  aseq i0 i9

  ; Load negative number 
  mov i0 &i
  lw x0 i0 i9
  mov i0 @156 ; The value represented as pos
  aseq i0 i9

  ret 

store_data:
  mov i9 @112       ; Expected value
  mov i0 &a         ; Address to spot in mem 
  shw x0 i0 i9      ; Store i9 into address 
  lhw x0 i0 i8      ; Load data from address to i8 
  aseq i9 i8        ; Ensure equal

  mov i9 @20        ; Expected value
  mov i0 &a         ; Address to spot in mem 
  sw x0 i0 i9       ; Store i9 into address 
  lw x0 i0 i8       ; Load data from address to i8 
  aseq i9 i8        ; Ensure equal

  mov i9 @900       ; Expected value
  mov i0 &a         ; Address to spot in mem 
  sdw x0 i0 i9      ; Store i9 into address 
  ldw x0 i0 i8      ; Load data from address to i8 
  aseq i9 i8        ; Ensure equal

  mov i9 @90000002  ; Expected value
  mov i0 &a         ; Address to spot in mem 
  sqw x0 i0 i9      ; Store i9 into address 
  lqw x0 i0 i8      ; Load data from address to i8 
  aseq i9 i8        ; Ensure equal

  ret

main:
  call load_data
  call store_data
  mov i0 @0 
  exit


  