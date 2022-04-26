; This program ensures that we can reserve memory by reserving 200 MiB of memory
; 1 MiB at a time while checking to ensure that each instruction involved in memory
; allocation succeeds. 
;
; Once the memory is reseved, the memory is iteratively freed in a similar manner.

.init main
.code 
main:
  mov i7 @1048576           ; 1 MiB
  mov i8 @0                 ; Counter
  mov i9 @200               ; Upper limit

allocate_top:
  mov op @0                 ; Clear op reg
  alloc i1 i7               ; Allocate 1 MiB and store slot number in i1
  aseq  x1 op               ; Ensure memory was allocated by checking op reg against 1 const
  aseq  i1 i8               ; Ensure that we are given the correct slot number
  add i8 i8 x1              ; Add one to counter 
  blt i8 i9 allocate_top    ; branch to allocate_top 

  mov i8 @0                 ; Counter
  
free_top:
  mov op @0                 ; Clear op reg
  free i8                   ; Free the memory
  aseq  x1 op               ; Ensure memory was freed by checking op reg against 1 const
  add i8 i8 x1              ; Add one to counter 
  blt i8 i9 free_top        ; branch to free_top 

  mov i0 @0                 ; Return code
  exit
