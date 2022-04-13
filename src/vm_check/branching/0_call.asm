.init main
.code

dead_code:
  mov i0 @0         ; Load the return code
  ret               ; Return to caller 
  mov i0 @99        ; Ret is part of call this should be dead
  exit              ; This too should be kill

main:
  mov i0 @1         ; Should be overwritten by the call
  call dead_code    ; Call and set return code
  exit
