.init fn_main
.debug 3
.string prompt    "Read in data : "
.string file_data "This is some file data that should get written to file!"
.string file_path "/tmp/file.txt"

.code 

; Constructs write flags into i9
;
fn_construct_write_flags:

  ; Verbose to show similarities with c++ 
  ;
  ;   std::fstream::out | std::fstream::app 
  ;
  mov i9 @16          ; output flag
  mov i5 @1           ; append 
  or i9 i5 i9         ; combine flags
  ret

; Constructs write flags into i9
;
fn_construct_read_flags:
  mov i9 @8           ; read flag
  ret

; Create a fd for file_path
;
fn_create_fd:

  ; Create space for command
  ;
  mov i0 @1024
  alloc i8 i0         ; Make some room for a command 

  ; Setup command word for 'create'
  ;
  mov i0 @0           ; Index into command 
  mov i2 @2           ; Inc value
  mov i3 @0           ; Command to create file descriptor '0'
  sw i8 x0 i3

  ; Setup source slot
  ;
  add i0 i0 i2        ; Move over 2 to load next parameter
  mov i2 @8           ; Inc value
  sqw i8 i0 x0        ; Indicate that the source slot is slot '0'
  add i0 i0 i2        ; Move over 8 to load next parameter

  ; Load file location and length
  ;
  mov i3 &file_path
  sqw i8 i0 i3        ; Load address of file_path 
  add i0 i0 i2        ; Move over 8 to load next parameter

  mov i3 #file_path
  sqw i8 i0 i3        ; Load string length of file_path 

  ; Setup system call
  ;
  mov i0 @1
  mov i1 @0
  syscall 2           ; Call the disk device

  aseq x1 op          ; Ensure that it worked 

  free i8             ; Free the command slot memory

                      ; File descriptor is now in i0
  ret 

; Open fd in i0
;   - Expects flags in i9
;
fn_open_fd:

  ; Create space for command
  ;
  mov i1 @1024
  alloc i8 i1         ; Make some room for a command 

  ; Setup command word for 'open'
  ;
  mov i1 @0           ; Index into command 
  mov i3 @1           ; Command to open file descriptor '1'
  sw i8 x0 i3         ; Store command word 
  mov i2 @2           ; Inc value
  add i1 i1 i2        ; Move over 2 to load next parameter

  ; Load File descriptor into command 
  ;
  sqw i8 i1 i0 
  mov i2 @8           ; Inc value
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Build flags and load
  ;
  sw i8 i1 i9         ; store flags in command 

  ; Setup system call
  ;
  mov i0 @1
  mov i1 @0
  syscall 2           ; Call the disk device

  aseq x1 op          ; Ensure that it is opened

  free i8             ; Free the command slot memory
  ret

; Write data to disk - expects fd in i0
;
fn_write_to_disk:

  ; Create space for command
  ;
  mov i1 @1024
  alloc i8 i1         ; Make some room for a command 

  ; Setup command word for 'open'
  ;
  mov i1 @0           ; Index into command 
  mov i3 @3           ; Command to write file descriptor '3'
  sw i8 x0 i3         ; Store command word 
  mov i2 @2           ; Inc value
  add i1 i1 i2        ; Move over 2 to load next parameter

  ; Load File descriptor into command 
  ;
  sqw i8 i1 i0 
  mov i2 @8           ; Inc value
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Load source slot 
  ;
  sqw i8 i1 x0        ; Indicate slot 0
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Load source slot offset
  ;
  mov i3 &file_data
  sqw i8 i1 i3
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Load source length 
  ;
  mov i3 #file_data
  sqw i8 i1 i3
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Setup system call
  ;
  mov i0 @1
  mov i1 @0
  syscall 2           ; Call the disk device

  aseq x1 op          ; Ensure that it is opened

  free i8             ; Free the command slot memory
  ret

; Close fd in i0
;
fn_close_file:

  ; Create space for command
  ;
  mov i1 @1024
  alloc i8 i1         ; Make some room for a command 

  ; Setup command word for 'close'
  ;
  mov i1 @0           ; Index into command 
  mov i3 @2           ; Command to close file descriptor '2'
  sw i8 x0 i3         ; Store command word 
  mov i2 @2           ; Inc value
  add i1 i1 i2        ; Move over 2 to load next parameter

  ; Load File descriptor into command 
  ;
  sqw i8 i1 i0 
  mov i2 @8           ; Inc value
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Setup system call
  ;
  mov i0 @1
  mov i1 @0
  syscall 2           ; Call the disk device

  aseq x1 op          ; Ensure that it is closed

  free i8             ; Free the command slot memory
  ret

fn_read_from_disk:

  ; Create space for command
  ;
  mov i1 @1024
  alloc i8 i1         ; Make some room for a command 

  ; Setup command word for 'open'
  ;
  mov i1 @0           ; Index into command 
  mov i3 @4           ; Command to read file descriptor '4'
  sw i8 x0 i3         ; Store command word 
  mov i2 @2           ; Inc value
  add i1 i1 i2        ; Move over 2 to load next parameter

  ; Load File descriptor into command 
  ;
  sqw i8 i1 i0 
  mov i2 @8           ; Inc value
  add i1 i1 i2        ; Move over 8 to load next parameter


  mov i3 #file_data   ; Allocate a new slot the size of the string
  alloc i3 i3

  ; Load source slot 
  ;
  sqw i8 i1 i3        ; Indicate newly alloced slot
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Load source slot offset
  ;
  mov i3 @0
  sqw i8 i1 i3
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Load source length 
  ;
  mov i4 #file_data
  sqw i8 i1 i4
  add i1 i1 i2        ; Move over 8 to load next parameter

  ; Setup system call
  ;
  mov i0 @1
  mov i1 @0
  syscall 2           ; Call the disk device

  mov i4 #file_data
  aseq i4 op          ; Ensure that all data was read in

  ; i3 contains address of slot with data read in

  debug 1

  free i8             ; Free the command slot memory
  ret

fn_main:
  call fn_create_fd   ; Create a file descriptor
  push_qw i0          ; Push it to the stack

  ; Open file for writing
  ;
  call fn_construct_write_flags 
  call fn_open_fd

  pop_qw i0           ; Pop fd
  push_qw i0          ; Push it back to the stack
  call fn_write_to_disk ; Write file_data out to disk

  pop_qw i0           ; Pop fd
  call fn_close_file

  call fn_create_fd   ; Create a file descriptor
  push_qw i0          ; Push it to the stack

  ; Open file for reading
  ;
  call fn_construct_read_flags 
  call fn_open_fd

  pop_qw i0
  call fn_read_from_disk
  exit