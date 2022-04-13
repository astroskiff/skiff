.init main
.code

killing_floor:
  aseq x0 x1                 ; Can never be true (constant 0, constant 1)
  ret

main:
  blt x1 x0 killing_floor    ; None of these should hit
  bgt x0 x1 killing_floor
  beq x0 x1 killing_floor

  blt x0 x1 spot_one
  jmp killing_floor          ; Should jump over

spot_one:
  bgt x1 x0 spot_two
  jmp killing_floor          ; Should jump over

spot_two:
  beq x0 x0 spot_three
  jmp killing_floor          ; Should jump over

spot_three:
  mov i0 @0
  exit
