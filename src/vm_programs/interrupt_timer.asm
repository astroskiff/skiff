; This program kicks off three timers that will trigger interrupts.
; A 3, 5, and 60 second timer. Each timer prints a string stating 
; which one is interrupting. 3, and 5 restart themselves until the 
; 60 second timer fires. Once 60 second timer fires the program exits. 
; Main keeps an infinite loop going while this all occurs.

.init fn_main
.debug 3

.string str_interrupt_0   "3 second interrupt fired"
.string str_interrupt_1   "5 second interrupt fired"
.string str_interrupt_4   "60 second interrupt fired - exiting"

#macro SYSCALL_TIMER      "syscall 0" ; Timer call id '0'
#macro SYSCALL_PRINTER    "syscall 1" ; Printer id '1'
#macro PRINTER_LOAD_ASCII "mov i3 @9"
#macro PRINTER_DO_NEWLINE "mov i4 @1" ; Tell printer to add newline
#macro PRINTER_SLOT_ZERO  "mov i0 @0" ; Tell printer to use memory slot 0 for data

.code 

; Assumes i1 i2 and i3 are already populated
; with offset, length, and print type 
;
fn_print_item:
  #PRINTER_SLOT_ZERO
  #PRINTER_DO_NEWLINE
  #SYSCALL_PRINTER
  aseq x1 op        ; Ensure it worked
  ret

; Save registers used for printing 
;
fn_save_registers:
  push_qw i0
  push_qw i1
  push_qw i2
  push_qw i3
  push_qw i4
  ret

; Restore the registers used for printing
;
fn_restore_registers:
  pop_qw i4
  pop_qw i3
  pop_qw i2
  pop_qw i1
  pop_qw i0
  ret

; Declare that we will be using `0` as an interrupt code
; and the code to handle it
;
interrupt_0:
  dirq                          ; Disable interrupts
  call fn_save_registers        ; Save the current state of the registers

  ; indicate that the interrupt was fired 
  ;
  mov i1 &str_interrupt_0       ; Offset
  mov i2 #str_interrupt_0       ; Length of item in words 
  #PRINTER_LOAD_ASCII
  call fn_print_item

  ; Re-queue the interrupt 
  ;
  call fn_create_three_sec_timer 

  call fn_restore_registers     ; Restore the registers
  eirq                          ; Enable interrupts
  ret                           ; Return to wherever we interrupted from

; Declare that we will be using `1` as an interrupt code
; and the code to handle it
;
interrupt_1:
  dirq                          ; Disable interrupts
  call fn_save_registers        ; Save the current state of the registers

  ; indicate that the interrupt was fired 
  ;
  mov i1 &str_interrupt_1       ; Offset
  mov i2 #str_interrupt_1       ; Length of item in words 
  #PRINTER_LOAD_ASCII
  call fn_print_item

  ; Re-queue the interrupt 
  ;
  call fn_create_five_sec_timer 

  call fn_restore_registers     ; Restore the registers
  eirq                          ; Enable interrupts
  ret                           ; Return to wherever we interrupted from

; Declare that we will be using `4` as an interrupt code
; and the code to handle it
;
interrupt_4:
  dirq                          ; Disable interrupts

  ; indicate that the interrupt was fired 
  ;
  mov i1 &str_interrupt_4       ; Offset
  mov i2 #str_interrupt_4       ; Length of item in words 
  #PRINTER_LOAD_ASCII
  call fn_print_item

  ; Exit
  mov i0 @0
  exit

; Create a 3 second timer
;
fn_create_three_sec_timer:
  mov i0 @3000    ; Load 3000ms (3 seconds) as parameter to timer
  mov i1 @0       ; Indicate we want completion interrupt to go to interrupt `0`
  #SYSCALL_TIMER  ; Call timer 
  aseq x1 op      ; Ensure that the timer was created
  ret

; Create a 5 second timer
;
fn_create_five_sec_timer:
  mov i0 @5000    ; Load 5000ms (5 seconds) as parameter to timer
  mov i1 @1       ; Indicate we want completion interrupt to go to interrupt `1`
  #SYSCALL_TIMER  ; Call timer 
  aseq x1 op      ; Ensure that the timer was created
  ret

; Create a 60 second timer
;
fn_create_60_sec_timer:
  mov i0 @60000    ; Load 60000ms (60 seconds) as parameter to timer
  mov i1 @4       ; Indicate we want completion interrupt to go to interrupt `4`
  #SYSCALL_TIMER  ; Call timer 
  aseq x1 op      ; Ensure that the timer was created
  ret

; Main
;
fn_main:
 call fn_create_three_sec_timer
 call fn_create_five_sec_timer
 call fn_create_60_sec_timer

  ; Infinite loop
l_loop_top:
  beq x0 x0 l_loop_top
  exit