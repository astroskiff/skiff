; This check is just to verify a modulus implementation
;
;

.init fn_main
.debug 3
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

; Entry
;
fn_main:

  ; 10 % 2
  ; 
  mov i1 @10      ; LHS
  mov i2 @2       ; RHS
  mov i3 @0       ; Expectation
  call fn_modulus
  aseq i3 i0 


  ; 10 % 3
  ; 
  mov i1 @10      ; LHS
  mov i2 @3       ; RHS
  mov i3 @1       ; Expectation
  call fn_modulus
  aseq i3 i0 

  ; 0 % 0
  ; 
  mov i1 @0       ; LHS
  mov i2 @0       ; RHS
  mov i3 @0       ; Expectation
  call fn_modulus
  aseq i3 i0 

  ; 19872 % 7
  ; 
  mov i1 @19872   ; LHS
  mov i2 @7       ; RHS
  mov i3 @6       ; Expectation
  call fn_modulus
  aseq i3 i0 

  mov i0 @0
  exit