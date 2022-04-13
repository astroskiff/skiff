.init main
.code

leave:
  mov i0 @0
  exit

main:
  mov i0 @99
  jmp leave
  exit
