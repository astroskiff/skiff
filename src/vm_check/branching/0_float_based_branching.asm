.init main

.float lhs 1.0
.float rhs 2.0
.code

load_values:
  mov i1 @0             ; Load constants from 0 slot
  mov i5 &lhs           ; At offset of data item
  lqw i1 i5 i0          ; Load LHS into i0 (retrieve data)
  mov i5 &rhs           ; At offset of data item
  lqw i1 i5 i1          ; Load RHS into i1 (retrieve data)
  ret

killing_floor:
  aseq x0 x1                 ; Can never be true (constant 0, constant 1)
  ret

main:
  call load_values

  bltf i1 i0 killing_floor    ; None of these should hit
  bgtf i0 i1 killing_floor
  beqf i0 i1 killing_floor

  bltf i0 i1 spot_one
  jmp killing_floor          ; Should jump over

spot_one:
  bgtf i1 i0 spot_two
  jmp killing_floor          ; Should jump over

spot_two:
  beqf i0 i0 spot_three
  jmp killing_floor          ; Should jump over

spot_three:
  mov i0 @0
  exit
