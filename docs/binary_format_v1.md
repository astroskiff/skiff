

Constant Type IDs :
```
  u8 - 0x00, u16 - 0x01, u32 - 0x02, u64 - 0x03
  i8 - 0x10, i16 - 0x11, i32 - 0x12, i64 - 0x13
  float - 0x20
  string - 0x30
```

Constant encodings:
```
Non-string constants
  [constant id] [number of bytes required to store item]

String constants
  [constant id] [2 byte length field] [string bytes]
```

Binary layout:
```

Binary Compatibility DWORD
   0000 0000 0000 0000 | 0000 0000 0000 0000 

Number of constants QWORD
   0000 0000 0000 0000 | 0000 0000 0000 0000
   0000 0000 0000 0000 | 0000 0000 0000 0000

Filler WORD
   1111 1111 1111 1111

Constant encodings .... 

Filler WORD
   1111 1111 1111 1111

Instructions ....

```