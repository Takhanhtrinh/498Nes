# 498NES 
<p>
  The purpose for this project to understand how an emulator works. This project
  implements NES emulator.
</p>

## Progress 
<p>
  All the opcodes work and tested except for the unofficial opcodes haven't
  implemented.
  I tested the opcodes based on the nestest.nes from wiki.nesdev.com. The
  emulator emulates the rom and compare the result with the log from Nintendulator.
  All the official opcodes passed except for the unofficial opcodes. 
  When you are running the testCPU, you will see the program prompts the input
  that means the emulator hits unofficial opcodes from nestest.nes, you can
  enter 1 to continue or enter 2 to exit. 

  NOTE: The ppu haven't implemented completely.
</p>

## Prerequisites 
<p>
To build this project, it is required to have Make and GCC compiler in your
system.
</p>
##Build testCPU 
  <p>make testCPU </p>
##Run testCPU 
  <p>./testCPU </p>

