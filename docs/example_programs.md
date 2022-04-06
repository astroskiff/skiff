

## Modulus

This program will mod two numbers loaded in `main` and the resulting exit code will be the result.

```
.init main
.code

; MODULUS
; i0 = i1 % i2
; Uses i0 - i4
modulus:
	div i3 i1 i2
  mov i4 @0
	beq i3 i4 moduiz
	jmp moduinz
moduiz:
    mov i0 @0
		jmp moddone
moduinz:
		mul i3 i2 i3
		sub i0 i1 i3
		jmp moddone
moddone:
    ret

main:
  mov i1 @13
  mov i2 @5
  call modulus
  exit


```