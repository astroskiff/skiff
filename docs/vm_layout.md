# VM Layout

*Notes*

Exit code will be loaded from `i0` 

# Memory

NOTE: This section is IDEA only

## Stack

```
  _________________________________
  |                                |
  |     Program Stack              |
  |                                |
  |________________________________|
          /|\
            |________ Grows and shrinks as-per instructed

```

Standard push / pop instructions
```
push_w  i0
push_dw i1
push_qw i2
```
```
pop_w  i0
pop_dw i1
pop_qw i2
```

### Storing / Loading from specfic regions in the stack

If one of the following failes `op` register will contain a `0`, and `1` if it works

Stack store
```
ss_w   <dest> <source>
ss_dw  <dest> <source>
ss_qw  <dest> <source>
```
Stack load
```
sl_w   <dest> <source>
sl_dw  <dest> <source>
sl_qw  <dest> <source>
```

## Heap

```
  'Heap' memory is bucketed into fixed-length data
   
  Buckets     Memory
  [-----]
  [  0  ] -> [ _ _ _ _ _ _ _ _ _ _ _ _ _ _ ]
  [ 16  ] -> [  _ _ _ _ _ _ ]
  [ 10  ] -> [ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _  _ ]
  [ 44  ] -> [ _ _ ]
  [  5  ] -> [ _ _ _ _ _ _ _ _ _ _ _ ]
  [-----]

```

Each 'bucket' is an id that points to the memory

  `mreq i0 @1024 ; Request 1024 bytes of memory, and store the bucket id in i0`


If mreq failes `op` register will contain a `0`, and `1` if it works

Releasing the bucket will remove all associated data and free the id for reuse

  `mrel i0      ; Free the bucket id in i0`

If mrel failes `op` register will contain a `0`, and `1` if it works


```
  mov i3 @21
  mov i2 @0
  sw  i0 i2 i3   ; Store word from i3 into i0 at offset i2 - Pulls lowest 2 bytes
  sdw i0 i2 i3   ; Store double word - lowest 4 bytes
  sqw i0 i2 i3   ; Store quad word - 8 bytes

  <memory slot> <offset> <data>

  lw i3 i0 i2   ; <destination> <memory slot> <offset>
  ...
  ..
  ..
```
*/