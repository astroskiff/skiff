

```

.data
.init main

.float pi 3.14159
.i32 value 42
.string hw "hellow world"

.code


main:

  mov i0 $value ; Move '42' into integer register 0 
  mov i0 #value ; Move length (in bytes) of value into register 0
  mov i0 &value ; Move memory location of value into register 0

  nop

  
  mov i0 $value
  mov f0 $pi
```
