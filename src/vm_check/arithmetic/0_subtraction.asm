.init main
.debug 3
.float RHS    2.5
.float LHS    3.0
.float result 0.5

.code
integer_based:
  mov i0 @15            ; LHS
  mov i1 @5             ; RHS
  mov i2 @10            ; Expected
  sub i0 i0 i1          ; Perform subtraction
  aseq i0 i2            ; Ensure things are equal
  ret

float_based:
  mov i1 @0             ; Load constants from 0 slot
  mov i5 &LHS           ; At offset of data item
  lqw i1 i5 i5          ; Load LHS into i5 (retrieve data)
  mov i6 &RHS           ; At offset of data item
  lqw i1 i6 i6          ; Load RHS into i6 (retrieve data)
  mov i7 &result        ; At offset of data item
  lqw i1 i7 i7          ; Load expected result into i7 (retrieve data)
  subf i5 i5 i6         ; Sub LHS from RHS and store in i5 
  aseq i5 i7            ; Ensure that the result meets expectations
  ret

main:
  call integer_based   ; Check integer based
  call float_based     ; Check float based
  mov i0 @0            ; Load up expected return value
  exit

