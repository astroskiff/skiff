.init main 


#macro A "mov i1 @1"
#macro B "add i0 i0 i1"
#macro C "mov i8 @11"

; Assert, a few nops for fun, and an exit 
#macro M_EXIT "aseq i8 i0" \
              "nop" \
              "nop" \
              "nop" \
              "nop" \
              "nop" \
              "nop" \
              "exit"

.code 

main:
  #A
  #B
  #B
  #B
  #B
  #B
  #B
  #B
  #B
  #B
  #B
  #B
  #C
  #M_EXIT
  