#include"chip8.h"
#include <stdio.h>
#include <stdlib.h> 
#include <time.h> 
#include <SDL.h>

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

void chip8::initialize() {
		PC = 0x200; // PC on beginning of program's memory 
		opcode = 0; // Reset opcode
		I = 0;		// Reset Index Reg
		sp = 0;		// Reset stack pointer 

		// Clear entire display
		for (int i = 0; i < 2048; ++i)
			gfx[i] = 0;

		// Clear entire stack
		for (int i = 0; i < 16; ++i)
			stack[i] = 0;
		
		// Clear keys and register's -s values 
		for (int i = 0; i < 16; ++i)
			key[i] = V[i] = 0;

		// Load fontset 
		for (int i = 0; i < 80; ++i)
			memory[i] = chip8_fontset[i];

		delay_timer = 0;
		sound_timer = 0;

		drawFlag = true;

		srand(time(NULL));

		
	}

	void chip8::emulateCycle() {
		// Fetch opcode
		opcode = memory[PC] << 8 | memory[PC + 1];
		
		// Decode opcode 
		switch (opcode & 0xF000)
		{

		//OPCODES

		case 0x0000: 
			switch (opcode & 0x000F) {
			case 0x0000: // opcode equal disp_clear() in C
				for (int i = 0; i < 64 * 32; i++)
					gfx[i] = 0x0;	
				PC += 2;
			break;

			case 0x000E:
				--sp;
				PC = stack[sp];		// Put the stored PC back to the PC
				PC += 2;
			break;

			default:
				printf("Unknown opcode : %X\n", opcode);
			}
		break;

		case 0x1000:
			PC = opcode & 0x0FFF;	// goto opcode 
		break;

		case 0x2000: 
			stack[sp] = PC;
			++sp;
			PC = opcode & 0x0FFF;
		break;

		case 0x3000:
			if (V[(opcode & 0x0F00) >> 8] == 0x00FF)
				PC += 4;
			else
				PC += 2;

		case 0x4000:
			if (V[(opcode & 0x0F00) >> 8] != 0x00FF)
				PC += 4;
			else
				PC += 2;
		break;

		case 0x5000:
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				PC += 4;
			else
				PC += 2;
		break;

		case 0x6000:
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			PC += 2;
		break;

		case 0x7000:
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			PC += 2;
		break;

		case 0x8000:
			switch (opcode & 0x000F) {
			case 0x0000:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
				PC += 2;
			break;

			case 0x0001:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
				PC += 2;
			break;

			case 0x0002:
				V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
				PC += 2;
			break;

			case 0x0003:
				V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			case 0x0004:
				if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) // if VY > remaining space in the reg -> V[16] - carry reg setting
					V[0xF] = 1; // carry bit
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
				PC += 2;
			break;

			case 0x0005:
				if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
					V[0xF] = 1;
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			break;

			case 0x0006:
				V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
				V[(opcode & 0x0F00) >> 8] >>= 1;
				PC += 2;
			break;

			case 0x0007:
				if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F00) >> 4])
					V[0xF] = 1;
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
				PC += 2;
			break;

			case 0x000E: 
				V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
				V[(opcode & 0x0F00) >> 8] <<= 1;
				PC += 2;
			}
		case 0x9000:
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				PC += 4;
			else
				PC += 2;
		break;

		case 0xA000: // ANNN: sets I to address NNN
		// Execute opcode
			I = opcode & 0x0FFF;
			PC += 2;
			break;
		case 0xB000:
			PC = V[0] + (opcode & 0x0FFF);
		break;
			
		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			PC += 2;
		break;

		case 0xD000: // The opcode responsible for drawing to display  // !!!!!!!!!!!!!!!!!!!!
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++) {
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++) {
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline * 64)))] == 1)
							V[0xF] = 1;
							gfx[(x + xline + (y + yline) * 64)] ^= 1;
					}
				}
				drawFlag = true;
					PC += 2;

			}
		break;

		case 0xE000:
			switch (opcode & 0x000F) {
			case 0x000E:
				if (key[V[(opcode & 0x0F00) >> 8]] != 0)
					PC += 4;
				else
					PC += 2;
				break;

			case 0x0001:
				if (key[V[(opcode & 0x0F00) >> 8]] == 0)
					PC += 4;
				else
					PC += 2;
				break;

			default:
				printf("Unknown opcode: %X", opcode);

			}
		break;


		case 0xF000:
			switch (opcode & 0x00FF) {
			case 0x0007:
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				PC += 2;
			break;

			case 0x000A:
				bool KPressing = false;

				for (int i = 0; i < 16; ++i)
					if (key[i] != 0) {
						V[(opcode & 0x0F00) >> 8] = i;
						KPressing = true;
					}
				
				if (!KPressing) // return back to calling function and try again
					return;
				PC += 2;
			break;

			case 0x0015:
				delay_timer = V[(opcode & 0x0F00) >> 8];
				PC += 2;
			break;


			case 0x0018:
				sound_timer = V[(opcode & 0x0F00) >> 8];
				PC += 2;
			break;

			case 0x001E:
				if (V[(opcode & 0x0F00) >> 8] > (0xFF - I)) // !
					V[0xF] = 1;
				else
					V[0xF] = 0;	
				I += V[(opcode & 0x0F00) >> 8];
				PC += 2;
			break;

			case 0x0029:
				I = V[(opcode & 0x0F00) >> 8] * 0x5;
				PC += 2;
			break;

			case 0x0033:				// steal solution for translate in BCD 
				memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
				memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
				PC += 2;
			break;

			case 0x0055:
				for (int i = 0; V[i] != V[((opcode & 0x0F00) >> 8) + 0x1]; ++i) // !
					memory[I + i] = V[i];
				PC += 2;
			break;

			case 0x0065:
				for (int i = 0; V[i] <= V[(opcode & 0x0F00) >> 8]; ++i)
							V[i] = memory[I + i];
			break;

			}
			
		default:
			printf("Unknown opcode: 0x%X", opcode);
		}

		// Update timers 
		if (delay_timer > 0)
			--delay_timer;
		
		if (sound_timer > 0) {
			if (sound_timer == 1) {
				printf("BEEP");
			}
			--sound_timer; 

		}

	}

void chip8::debugRender() {
	// Draw image 
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			if (gfx[(y * 64) + x] == 0)
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}

bool chip8::loadApplication(const char * filename) {
	initialize();

	printf("Loading filename: %s", filename);

	//Open file 
	FILE * pointerFile = fopen(filename, "rb"/* open binary file for reading */);
	if (pointerFile == nullptr) {
		fputs("File error", stderr);
		return false;
	}

	//Checking file size
	fseek(pointerFile, 0, SEEK_END); // seek to End of the file 
	long lsize = ftell(pointerFile); // tell number number of bytes from the beginning to the pointer
	rewind(pointerFile);			 // as fseek(pointerFile, 0, SEEK_SET)
	printf("File size: %d\n", int(lsize));

	// Allocate memory to contain the whole buffer
	char * buffer = (char *) malloc(sizeof(char) * lsize);

	if (buffer == nullptr) {
		fputs("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer 
	size_t result = fread(buffer, 1, lsize, pointerFile);
	if(result != lsize) {
		fputs("Reading error", stderr);
		return false;
}
	// Copy buffer into chip 8 memory 
	if ((4096 - 512) > lsize /* check that enough memory from 0x200 to the end*/) {
		for (int i = 0; i < lsize; ++i)
			memory[512 + i] = buffer[i];
	}
	else
		printf("Error: too much place for pr, no in memory");
	
	fclose(pointerFile); // close file descriptor 
	free(buffer);		// deallocate buffer
	
	return true;
}	