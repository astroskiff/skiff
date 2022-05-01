.init main
.debug 3
.code 

main:
  aseq x0 sp  ; Ensure the stack pointer is 0

  ; QWORD
  mov i1 @2147483650
  push_qw i1

  mov i9 @8   ; Check SP updated
  aseq i9 sp

  ; DWORD
  mov i2 @33000
  push_dw i2

  mov i9 @12  ; Check SP updated
  aseq i9 sp

  ; WORD
  mov i3 @420
  push_w i3

  mov i9 @14  ; Check SP updated
  aseq i9 sp

  ; BYTE
  mov i4 @42
  push_hw i4

  mov i9 @15  ; Check SP updated - Word pushed, not just byte (+2)
  aseq i9 sp

  ; Pop off values and ensure they are the same 

  pop_hw i0 
  aseq i4 i0  ; Ensure the same value

  mov i9 @14  ; Check SP updated
  aseq i9 sp

  pop_w i0 
  aseq i3 i0  ; Ensure the same value

  mov i9 @12  ; Check SP updated
  aseq i9 sp

  pop_dw i0 
  aseq i2 i0  ; Ensure the same value

  mov i9 @8   ; Check SP updated
  aseq i9 sp

  pop_qw i0 
  aseq i1 i0  ; Ensure the same value

  mov i9 @0   ; Check SP updated
  aseq i9 sp

  mov i0 @0   ; Load return code 
  exit
