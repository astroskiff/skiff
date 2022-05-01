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
#macro SYSCALL_USERIO     "syscall 1" ; Printer id '1'
.code 

; i5 Type
; i6 Location
; i7 Length
; i3 source slot
fn_print_item:
  mov i0 @1024  ; 16 Bytes for command slot 
  alloc i8 i0   ; Allocate slot and get id in i8 

  mov i0 @0
  mov i1 @2       ; Word size in bytes  
  sw i8 i0 i0     ; Store `0` into command slot 
  
  add i0 i0 i1    ; Increase slot index 
  sqw i8 i0 i3    ; Indicate source slot is `0` (data directive)
  
  mov i1 @8       ; Q Word size in bytes  

  add i0 i0 i1  
  sqw i8 i0 i6     ; Load Location

  add i0 i0 i1  
  sw i8 i0 i5     ; Load type

  mov i1 @2       ; Word size in bytes  

  add i0 i0 i1  
  sqw i8 i0 i7    ; Load length

  mov i1 @8       ; Q Word size in bytes  

  add i0 i0 i1  
  mov i2 @1       ; Send to stdout
  sw i8 i0 i2

  mov i1 @2       ; Word size in bytes  

  add i0 i0 i1
  mov i2 @1       ; Print new line
  sw i8 i0 i2

  push_qw i8      ; Load command slot into i0 
  pop_qw i0 

  mov i1 @0       ; Load command offset

  #SYSCALL_USERIO ; Call timer 
  free i8
  ret

; Save registers used for printing 
;
fn_save_registers:
  push_qw i0
  push_qw i1
  push_qw i2
  push_qw i3
  push_qw i4
  push_qw i5
  push_qw i6
  push_qw i7
  push_qw i8
  push_qw i9
  ret

; Restore the registers used for printing
;
fn_restore_registers:
  pop_qw i9
  pop_qw i8
  pop_qw i7
  pop_qw i6
  pop_qw i5
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

  mov i3 @0
  mov i5 @9                 ; ASCII
  mov i6 &str_interrupt_0   ; Address
  mov i7 #str_interrupt_0   ; Len
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
  mov i3 @0
  mov i5 @9                 ; ASCII
  mov i6 &str_interrupt_1   ; Address
  mov i7 #str_interrupt_1   ; Len
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
  mov i3 @0
  mov i5 @9                 ; ASCII
  mov i6 &str_interrupt_4   ; Address
  mov i7 #str_interrupt_4   ; Len
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