;
;   This is a WIP - not yet working. 
;

.init fn_main
.debug 3
.u8       number    0
.string   string_0  " is prime "
.string   string_1  "  "

#macro SYSCALL_PRINTER    "syscall 0" ; Printer id '0'
#macro PRINTER_LOAD_U8    "mov i3 @0"
#macro PRINTER_LOAD_ASCII "mov i3 @9"
#macro EXIT_SUCCESS       "mov i0 @0" \
                          "exit"

.code 

; MODULUS
; i0 = i1 % i2
; Saves and restores used registers
fn_modulus:

  ; Save register states 
  push_qw i1
  push_qw i2
  push_qw i3
  push_qw i4

  ; Check for 0 RHS
  mov i0 @0
  beq i0 i2 l_modulus_complete

  ; Perform modulous operation
	div i3 i1 i2
  mov i4 @0
	beq i3 i4 l_modulous_is_zero
	jmp l_modulus_is_non_zero

l_modulous_is_zero:
  mov i0 @0
  jmp l_modulus_complete

l_modulus_is_non_zero:
  mul i3 i2 i3
  sub i0 i1 i3
  jmp l_modulus_complete

l_modulus_complete:

  ; Restore saved registers
  pop_qw i4
  pop_qw i3
  pop_qw i2
  pop_qw i1
  ret

; IS_PRIME
; Check if item in i0 is prime, result in i0
;
fn_is_prime:
  mov i7 @2
  mov i8 @3

  ; item == 2 || item == 3
  beq i0 i7 l_item_is_prime
  beq i0 i8 l_item_is_prime

  ; item <= 1
  blt i0 x1 l_item_is_not_prime
  beq i0 x1 l_item_is_not_prime

  ; Copy 'n' in question into i1 for call to mod
  push_qw i0 
  pop_qw  i1 
  
  ; n % 2 == 0
  mov i2 @2
  call fn_modulus
  beq i0 x0 l_item_is_not_prime

  ; n % 3 == 0
  mov i2 @3
  call fn_modulus
  beq i0 x0 l_item_is_not_prime

  mov i2 @5 ; i 
  mov i3 @6
l_primality_loop_top_0:

  debug 0

  ; 'n' to check is still in i1 
  ; 'i' is i2, which is also the parameter to mod

  ; n % i == 0 ?
  call fn_modulus
  beq x0 i0 l_item_is_not_prime

  ; Save 'i'
  push_qw i2 

  ; n % (i+2) == 0 ? 
  mov i8 @2
  add i2 i2 i8  ; i+2

  call fn_modulus
  beq x0 i0 l_item_is_not_prime

  ; restore 'i'
  pop_qw  i2

  ; i += 6
  add i2 i2 i3 

  ; i * i <= 'n'
  mul i8 i2 i2 
  blt i8 i1 l_primality_loop_top_0
  beq i8 i1 l_primality_loop_top_0
  
  ; Fallthrough on completion - item is prime

l_item_is_prime:
  mov i0 @1
  ret

l_item_is_not_prime:
  mov i0 @0
  ret 

; PRINT_RESULT
; i0 : 1 = prime, 0 = not prime
; Assumes that memory slot '1' is being used 
;   with an offset of 0 for printer data
fn_print_result:

  ; Save is_prime to stack while number is printed
  push_qw i0

  mov i0 @0
  mov i1 &number
  mov i2 #number
  mov i4 @0
  #PRINTER_LOAD_U8
  #SYSCALL_PRINTER
  aseq x1 op        ; Ensure it worked

  ; Restore the is_prime
  pop_qw i0

  ; If it isn't prime branch to correct spot
  beq x0 i0 l_load_is_not_prime

  mov i0 @0             ; Slot 0
  mov i1 &string_0      ; Offset
  mov i2 #string_0      ; len
  #PRINTER_LOAD_ASCII   ; ASCII
  mov i4 @1             ; New line
  #SYSCALL_PRINTER
  aseq x1 op        ; Ensure it worked
  jmp l_end

l_load_is_not_prime:
  mov i0 @0             ; Slot 0
  mov i1 &string_1      ; Offset
  mov i2 #string_1      ; len
  #PRINTER_LOAD_ASCII   ; ASCII
  mov i4 @1             ; New line
  #SYSCALL_PRINTER
  aseq x1 op        ; Ensure it worked

l_end:
  ret

; MAIN
fn_main_old:
  mov i0 @0
  mov i1 @100

l_main_primality_check_loop_top:

  ; Store 'n' in memory
  sw x0 x0 i0

  ; Save loop counter and limit 
  push_qw i0
  push_qw i1

  ; i0 (counter) is being checked
  call fn_is_prime
  call fn_print_result

  ; Restore counter and limit 
  pop_qw i1
  pop_qw i0

  ; Add one to counter
  mov i2 @1
  add i0 i0 i2

  ; Check loop condition
  blt i0 i1 l_main_primality_check_loop_top

  #EXIT_SUCCESS

fn_main:

  mov i0 @5
  call fn_is_prime

  exit