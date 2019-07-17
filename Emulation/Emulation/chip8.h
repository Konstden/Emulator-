#pragma once

class chip8 {
public:
	chip8();
	~chip8();

	bool drawFlag;				// for update the screen 
	void emulateCycle();
	void debugRender();
	bool loadApplication(const char * filename);
	unsigned char gfx[64 * 32];
	unsigned char key[16];

private: 
	unsigned short opcode;

	unsigned char memory[4096];
	unsigned char V[16];

	unsigned short I;
	unsigned short PC;

	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned short stack[16];
	unsigned short sp;

	void initialize();
};
