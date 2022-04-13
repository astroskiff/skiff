.init main
.debug 3
.float two    2.0
.float three  3.3
.float result 6.6

.code
integer_based:
  mov i0 @2             ; LHS
  mov i1 @5             ; RHS
  mov i2 @10            ; Expected
  mul i0 i0 i1          ; Perform multiplication
  aseq i0 i2            ; Ensure things are equal
  ret

float_based:
  mov i1 @0             ; Load constants from 0 slot
  mov i5 &two           ; At offset of data item
  lqw i1 i5 i5          ; Load LHS into i5 (retrieve data)
  mov i6 &three         ; At offset of data item
  lqw i1 i6 i6          ; Load RHS into i6 (retrieve data)
  mov i7 &result        ; At offset of data item
  lqw i1 i7 i7          ; Load expected result into i7 (retrieve data)
  mulf i5 i5 i6         ; Mul LHS and RHS and store in i5 
  aseq i5 i7            ; Ensure that the result meets expectations
  ret

main:
  call integer_based   ; Check integer based
  call float_based     ; Check float based
  mov i0 @0            ; Load up expected return value
  exit

