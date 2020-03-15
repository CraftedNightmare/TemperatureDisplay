#!/bin/bash


CC = SDCC
CCFLAGS = -mstm8 --std-c99 --code-loc 0x008000 --stack-loc 0x1FE --data-loc 0x0 --debug --max-allocs-per-node 9999
CCHEXFLAGS = --out-fmt-ihx $(CCFLAGS)
CCELFFLAGS = --out-fmt-elf $(CCFLAGS)


all: build


build:
	$(CC) $(CCHEXFLAGS) main.c 
	$(CC) $(CCELFFLAGS) main.c 
	cp main.elf Debug/main.elf
	cp main.ihx Debug/main.hex


	
	
