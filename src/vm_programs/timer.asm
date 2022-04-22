; Running this program with `-l debug` or greater and after ~3 seconds the interrupt will
; fire and you will see "NOP" run down the screen a few times. Then, some time later the 
; program will exit with code 28

.init fn_main
.debug 3

#macro SYSCALL_TIMER    "syscall 0" ; Timer call id '1'

.code 

fn_main:

  ; Create a 10 second timer
  mov i0 @3000

  ; Have it send interrupt code 8
  mov i1 @8

  #SYSCALL_TIMER

  ; Ensure the timer was created
  aseq x1 op

  mov i0 @0
  mov i1 @1
  mov i2 @2147483646

  ; Loop for a very long time (0 -> int64_t max - 1)
l_loop_top:
  add i0 i0 i1
  blt i0 i2 l_loop_top

  ; Move 28 into i0 so we have some random return code
  mov i0 @28
  exit

interrupt_8:
  dirq
  nop
  nop
  nop
  nop
  nop
  eirq
  ret
  

