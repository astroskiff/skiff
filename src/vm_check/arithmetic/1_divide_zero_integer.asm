.init main
.debug 3

.code
main:
  mov i0 @15            ; LHS
  mov i1 @0             ; RHS
  div i0 i0 i1          ; Perform divide by zero
  mov i0 @0             ; Should panic before this
  exit
