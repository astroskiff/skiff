# skiff
[![CircleCI](https://circleci.com/gh/astroskiff/skiff/tree/main.svg?style=svg)](https://circleci.com/gh/skiff-vm/skiff/tree/main)
[![MIT License](https://img.shields.io/apm/l/atomic-design-ui.svg?)](https://github.com/skiff-vm/skiff/blob/main/LICENSE)

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

## About

Skiff is a reduced instruction set (RISC) register-based virtual machine (VM). This VM uses a non-standard memory model that allows
assigning and freeing memory very easily. 

While skiff is intended to be utilized by a higher level language
as a back end there currently exists an awesome assembler that makes development a breeze.

## Cool things in Skiff

**Assembler Macros**

It seems silly, but it's true and super helpful. Heres an example: 

```
.init main
#macro M_EXIT "mov i0 @0" \
              "exit"
.code
main:
	#M_EXIT
```

A bit contrived, but it gets the point across. Anything that exists in the integer register `i0` will be returned by the VM as the return code. Here, we create a macro that loads `0` into `i0` and issues the exit instruction. As can bee seen here macros can be multi-lined and are pretty much just drop-and-replace code pieces. This becomes quite
helpful when it comes to interacting with devices with the VM (more on that later).

**Extensibility**

Since the machine is virtual, it would be a shame if it couldn't be easilt extended. Good thing it can! Using some interfaces the machine can
be extended such that external modules can be invoked via the assembly code. A great example of this is in `machine/system/print.hpp`. Using this
a printer has been created that can dump out strings and other datums from memory. Interacting with printer in skiff asm is a great example of where macros become useful and how external calls are created / called. Check out `src/vm_programs/print.asm` to see how macros make the code more readable and how to utilize the printer.

**Interrupts**

When it comes to interrupting execution and switching tasks Skiff has a pretty neat variable-number of interrupts that are declared by the programmer. In a lot of real systems there are a set number of interrupts all with their own predetermined purpose, but with Skiff you can set the number of interrupts simply by declaring a label like so:

```

.init main
.code

interrupt_0:
  ret

interrupt_6:
  ret

interrupt_2:
  ret

main:
  exit

```

In this example we declare 3 labels (0, 6, and 2) with a name of the form "interrupt_N". The assembler will identify these labels as a declaration of 3 interrupts with their respective ids and encode into the binary their locations in memory. This will enable the user to then
inform devices "attached" to the VM which interrupt number to utilize in its operation. In this way the vm has a variable number of interrupts. These interrupts are even thread safe so a device could operate in a different thread entirely and interrupt the main context at its leisure. 

**Slotted Memory**

Standard systems have a heap and a stack that are arbitrarily defined in physical memory. Emulating this would have been one way to work with memory in the VM, but it was decided that making operations simpler would be preferred. Instead, the memory model in skiff operates as follows:

0) A number of bytes is requested for allocation via the `alloc` instruction
1) The requested number of bytes is allocated and an id is returned into a specified slot
2) Using this id `store` and `load` instructions can perform their respective operations on data
3) When the memory is no longer needed the `free` instruction can be utilized to mark the memory for reuse.

An example:

```

+---+   +---+---+---+---+
|   |   |   |   |   |   |
| 0 +-->|   |   |   |   |
|   |   |   |   |   |   |
+---+   +---+---+---+---+

When the program starts, any data declared via directives is slotted into memory id 0

If the `alloc` command is ran twice, with the first requestin 7 bytes, and the second requesting 2, it will return ids 1 and 2 repectively.

+---+   +---+---+---+---+
|   |   |   |   |   |   |
| 0 +-->|   |   |   |   |
|   |   |   |   |   |   |
+---+   +---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   |   |
| 1 +-->|   |   |   |   |   |   |   |
|   |   |   |   |   |   |   |   |   |
+---+   +---+---+---+---+---+---+---+
|   |   |   |   |
| 2 +-->|   |   |
|   |   |   |   |
+---+   +---+---+

Using the given ids `store` and `load` could move memory from/to registers from any of the above slots (0-2). Once memory is no longer required, `free` could be ran. For the sake of example lets `free` slot 1.

free_slot_one:
  mov i0 @1   ; Load 1 into integer register 0
  free i0     ; Request memory from slot 1 be freed

+---+   +---+---+---+---+
|   |   |   |   |   |   |
| 0 +-->|   |   |   |   |
|   |   |   |   |   |   |
+---+   +---+---+---+---+
|   |
| 1 |
|   |
+---+   +---+---+
|   |   |   |   |
| 2 +-->|   |   |
|   |   |   |   |
+---+   +---+---+

Slot 1 is no longer pointing to anything and the id is marked by the system for reuse. Now the next time `alloc` is called the id 1 will be utilized for storage. Lets say for example we request memory to allocate 10 bytes for us as so:

allocate_ten_bytes:
  mov i1 @10    ; Load 10 into integer register 1
  alloc i0 i1   ; Request an allocation of 10 bytes and store the resulting id into integer register 0
  ret

Memory will now be as follows:

+---+   +---+---+---+---+
|   |   |   |   |   |   |
| 0 +-->|   |   |   |   |
|   |   |   |   |   |   |
+---+   +---+---+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   |   |   |   |   |
| 1 +-->|   |   |   |   |   |   |   |   |   |   |
|   |   |   |   |   |   |   |   |   |   |   |   |
+---+   +---+---+---+---+---+---+---+---+---+---+
|   |   |   |   |
| 2 +-->|   |   |
|   |   |   |   |
+---+   +---+---+

With the number `1` residing in `i0`

```

The slotted memory model not only makes keeping track of variable memory super simple, but it enables communication to devices really easy.
Slots can be declared of a particular size, then a device can be instructed to use only that slot as an input or output buffer!


## Building Skiff

Skiff doesn't rely on any external libraries except threads so building it is pretty straight forward. Ensure that you have a C++ 20 compiler (I use clang 13.1.6) and that should be all you need.

The recommended way to build skiff is as follows:
```
cd src
mkdir build && cd build
cmake ../
make -j5
```

Utilizing ccmake is also a good option so you can configure things with ease. 


## Assembling / running a skiff binary

Using your newly compiled `skiff` binary to assemble a skiff binary is straight forward. 

`./skiff -a my_program.asm -o my_program.bin` 

This will assemble a binary from the file `my_program.asm` (supplied by you) and output it to the bianry file `my_program.bin`.

Runing the program is even easier:

`./skiff my_program.bin`


If you add on `-l trace` to the end of a skiff command you can see all of the internal logging. 
Check out `./skiff -h` for more assembler / run-time options and log levels.

## Development

If you're interested in helping develop skiff, check out `contributing.md` in the root directory of this repo.

## Documentation 

For documentation on the VM layout, assembler, and instructions, check out the `docs/` directory.

## Misc. Information

Development Tracking : https://skiffvm.youtrack.cloud/

Source for the Skiff-VM icon: <a target="_blank" href="https://icons8.com/icon/VLNrBkECYkdq/paper-ship">Paper Ship</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>
