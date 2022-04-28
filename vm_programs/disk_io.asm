.init fn_main

.string file_data "This is some file data that should get written to file!"
.string file_path "/tmp/file.txt"

.code 


fn_create_fd:
  mov i0 @24
  alloc i8 i0         ; Make some room for a command 

  mov i0 @0           ; Index into command 
  mov i2 @8           ; Inc value
  mov i3 @0           ; Command to create file descriptor '0'
  sqw i8 x0 i3  

  add i0 i0 i2        ; Move over 8 to load next parameter
  
  sqw i8 i0 x0        ; Indicate that the source slot is slot '0'

  add i0 i0 i2        ; Move over 8 to load next parameter

  mov i3 &file_path
  sqw i8 i0 i3        ; Load address of file_path 

  add i0 i0 i2        ; Move over 8 to load next parameter

  mov i3 #file_path
  sqw i8 i0 i3        ; Load string length of file_path 

  syscall 2           ; Call the disk device

  aseq x1 op          ; Ensure that it worked 

                      ; File descriptor is now in i0
  ret 

l_copy_path_string:



  ret

fn_write_to_disk:



  ret

fn_read_from_disk:

  mov i0 @4096
  alloc i9 i0   ; Create an input buffer 
  ret

fn_main:
  call fn_create_fd




  exit