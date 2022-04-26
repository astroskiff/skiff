.init main
.code

check_lsh:
  mov i9 @512
  mov i0 @2
  mov i1 @8
  lsh i0 i0 i1
  aseq i9 i0
  ret 

check_rsh:
  mov i9 @2
  mov i0 @512
  mov i1 @8
  rsh i0 i0 i1
  aseq i9 i0
  ret 

check_and:
  mov i9 @1
  mov i0 @9
  mov i1 @3
  and i0 i0 i1
  aseq i9 i0
  ret

check_or:
  mov i9 @11
  mov i0 @9
  mov i1 @3
  or i0 i0 i1
  aseq i9 i0
  ret

check_xor:
  mov i9 @10
  mov i0 @9
  mov i1 @3
  xor i0 i0 i1
  aseq i9 i0
  ret

check_not:
  mov i0 @0
  mov i1 @1
  not i0 i0
  aseq i0 i1 
  ret 

main:
  call check_lsh
  call check_rsh
  call check_not
  call check_and
  call check_or
  mov i0 @0
  exit
