# VM Layout


# SYSCALL

Register interfaces for system things at launch time, each with a determined id.

System calls so far : 

0 - Timer spawner
1 - System printer



# Interrupts

The system can have any number of interrupts, determined by the programmer. 
Labels with a name matching `interrupt_N` where `N` is any number `[0-uint64_t::max()]` 
will be read in and marked in the system as a memory location that can be called from an interrupt. 

Devices and system calls that perform interrupts will take in the interrupt number
it is required to call.


## Example 

```
.init fn_main
.code
interrupt_0:
  dirq  ; Disable interrupts
  nop
  eirq  ; Enable interrupts
  ret

interrupt_1:
  ret

interrupt_99:
  ret

fn_main:
  exit
```

The above example shows how, by the use of label naming, you can declare what interrupts you want to exist. Them being present will cause the
assembler to map the number in the label to the address in memory that the label is. This allows you to pass the number declared in the label name
to some external device that will interrupt when processing is complete. 

When an interrupt is fired it is similar to a call instruction being executed, this means that at the end of the section handling the interrupt, 
a `ret` should exist to return to wherever the code was interrupted from. 

Using `dirq` and `eirq` you can disable or enable interrupt requests. It is recommended at the beginning of an interrupt handle to disable interrupts
so you can be sure to process whatever is going on, though its not required. 

If an interrupt is fired externally while interrupts are disabled then the request will be ignored. It is up to the interrupter to decide if it wants to
try again later or not as interrupt requests are not queued... they are either accepted or denied.
