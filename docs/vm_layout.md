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
