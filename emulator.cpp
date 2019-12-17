#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <algorithm>

#define RAM_SIZE 4096
#define MEMORY_UNBLOCKED 0x0000
#define MEMORY_BLOCKED   0x0001

int main() {

	char memory[RAM_SIZE];
	char memory_map[RAM_SIZE];

	bool OK = true;

	uint8_t A = 0;
	char* PC = memory + 0x100;

	char buf[2];

	unsigned counter = 0;
	unsigned counter_ = 0;

	memset(memory, 0, RAM_SIZE);
	memset(memory_map, MEMORY_UNBLOCKED, RAM_SIZE);

	FILE* f = fopen("rom", "rb");
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fread(memory+0x100, 1, size, f);
	fclose(f);

	while (OK) {

		uint16_t instruction = (static_cast<uint16_t>(PC[1]) & 0x00FF) + (static_cast<uint16_t>(PC[0]) << 8);
		uint8_t  XX = instruction & 0x00FF;
		uint16_t XXX = instruction & 0x0FFF;
		char* _XXX_ = memory + XXX;
		char* _XXX_map = memory_map + XXX;

		//printf("\nOP:0x%04X A:0x%02X PC:0x%03x C:%u", instruction, A & 0xFF, PC-memory & 0xFFF, counter_);

		if (counter == 150)
			counter = 0;
		if (counter_ == 3876)
			int i = 0;
		counter++;
		counter_++;

		switch (PC[0] & 0xF0)
		{
		// -------------------------------
		// ARITHMETIC
		// -------------------------------
		case 0x00:
			switch (PC[0] & 0x0F)
			{
			case 0x0:
				A += XX;
				goto next_instruction;
			case 0x1:
				A = XX;
				goto next_instruction;
			case 0x2:
				A ^= XX;
				goto next_instruction;
			case 0x3:
				A |= XX;
				goto next_instruction;
			case 0x4:
				A &= XX;
				goto next_instruction;
			default:
				goto invalid_instruction;
			};
		case 0x80:
			A = *_XXX_;
			goto next_instruction;
		case 0xD0:
			if (*_XXX_map == MEMORY_UNBLOCKED)
				*_XXX_ ^= A;
			goto next_instruction;
		case 0xF0:
			if (*_XXX_map == MEMORY_UNBLOCKED)
				*_XXX_ = A;
			goto next_instruction;

		// -------------------------------
		// CONTROL FLOW
		// -------------------------------
		case 0x20:
			PC = memory + XXX;
			goto jump;
		case 0x30:
			if (A == 0x00) {
				PC = _XXX_;
				goto jump;
			}
			goto next_instruction;
		case 0x40:
			if (A == 0x01) {
				PC = _XXX_;
				goto jump;
			}
			goto next_instruction;
		case 0x50:
			if (A == 0xFF) {
				PC = _XXX_;
				goto jump;
			}
			goto next_instruction;
		case 0x60:
			if ((instruction & 0xFF00) != 0x6000) // TODO make not suck
				goto invalid_instruction;
			else if (A > XX) {
				A = 0xFF;
				goto next_instruction;
			}
			else if (A < XX) {
				A = 0x01;
				goto next_instruction;
			}
			else
				A = 0x00;
			goto next_instruction;
		case 0x70:
			if (A > *_XXX_) {
				A = 0xFF;
				goto next_instruction;
			}
			else if (A < *_XXX_) {
				A = 0x01;
				goto next_instruction;
			}
			else
				A = 0x00;
			goto next_instruction;
		// -------------------------------
		// SECURITY
		// -------------------------------
		case 0x90:
			*_XXX_map = MEMORY_BLOCKED;
			goto next_instruction;
		case 0xA0:
			*_XXX_map = MEMORY_UNBLOCKED;
			goto next_instruction;
		case 0xC0:
			if(*_XXX_map == MEMORY_UNBLOCKED)
				*_XXX_ ^= 0x42;
			goto next_instruction;
		default:
			break;
		}
	
		// -------------------------------
		// SPECIAL INSTRUCTIONS
		// -------------------------------
		switch (instruction)
		{
		case 0x1337: // print A
			buf[0] = A;
			buf[1] = '\0';
			printf(buf);
			goto next_instruction;
		case 0xBEEF: // reset
			PC = memory + 0x100;
			A = 0x42;
			goto jump;
		case 0xEEEE: // NOP
			goto next_instruction;
		default:
			break;
		}

	invalid_instruction:
		A--; // quirk
	next_instruction:
		PC += 2;
	jump:
		//printf("\n");
		continue;
	}
	
}