.init fn_main
.debug 3

#macro SYSCALL_TIMER    "syscall 0" ; Timer call id '1'

.code 

fn_main:

  ; Create a 10 second timer
  mov i0 @10000

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
  exit

interrupt_8:
  dirq
  nop
  eirq
  ret

