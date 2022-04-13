.init main
.debug 3
.float LHS    10.0
.float RHS    0.0

.code
main:
  mov i1 @0             ; Load constants from 0 slot
  mov i5 &LHS           ; At offset of data item
  lqw i1 i5 i5          ; Load LHS into i5 (retrieve data)
  mov i6 &RHS           ; At offset of data item
  lqw i1 i6 i6          ; Load RHS into i6 (retrieve data)
  divf i5 i5 i6         ; Div LHS by RHS and store in i5 
  mov i0 @0             ; Should die before this
  exit

