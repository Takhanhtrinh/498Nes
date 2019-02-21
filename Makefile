CC = gcc
FLAG = -g 
HELPER = helper.c
CPU = cpu.c 
MEM = mem.c 
ROM = rom.c
PPU = ppu.c
DIS = testDisassembler.c
CONTROLLER = controller.c

READ_ROOM = testReadRoom.c
TEST_CPU = testCPU.c

cpu.o: $(CPU)
	$(CC) -c $(FLAG) $(CPU) -o cpu.o -DDEBUG
mem.o: $(MEM) 
	$(CC) -c $(FLAG) $(MEM) -o mem.o
rom.o: $(ROM) 
	$(CC) -c $(FLAG) $(ROM) -o rom.o 
ppu.o: $(PPU) 
	$(CC) -c $(FLAG) $(PPU) -o ppu.o 
helper.o: $(HELPER) 
	$(CC) -c $(FLAG) $(HELPER) -o helper.o
controller.o: $(CONTROLLER) 
	$(CC) -c $(FLAG) $(CONTROLLER) -o controller.o
testDisassembler: $(DIS) cpu.o mem.o rom.o ppu.o helper.o
	$(CC) $(FLAG) $(DIS) -o testDisassembler cpu.o mem.o rom.o ppu.o helper.o
testReadRoom: $(READ_ROOM) cpu.o mem.o rom.o ppu.o
	$(CC) $(FLAG) $(READ_ROOM) -o testReadRoom cpu.o mem.o rom.o ppu.o
testCPU: $(TEST_CPU) helper.o mem.o cpu.o rom.o ppu.o controller.o
	$(CC) $(FLAG) $(TEST_CPU) -o testCPU helper.o mem.o cpu.o rom.o ppu.o controller.o
clean: 
	rm *.o testCpu testCpu
