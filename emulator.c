#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define RAM_SIZE 4096
#define MEMORY_UNBLOCKED 0x0000
#define MEMORY_BLOCKED   0x0001

int main(int argc, char **argv) {

	char memory[RAM_SIZE];
	char memory_map[RAM_SIZE];

	int OK = 1;

	uint8_t A = 0;
	char* PC = memory + 0x100u;

	memset(memory, 0, RAM_SIZE);
	memset(memory_map, MEMORY_UNBLOCKED, RAM_SIZE);

	FILE* f = fopen("rom", "rb");
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	assert(fread(memory + 0x100u, 1, size, f) > 0);
	fclose(f);

	while (OK == 1) {

		uint16_t instruction = ((uint16_t)(PC[1]) & 0x00FFu) + ((uint16_t)(PC[0]) << 8);
		uint8_t  XX = instruction & 0x00FFu;
		uint16_t XXX = instruction & 0x0FFFu;
		char* _XXX_ = memory + XXX;
		char* _XXX_map = memory_map + XXX;

		/*printf("\nOP:0x%04X A:0x%02X PC:0x%03x C:%u", instruction, A & 0xFF, static_cast<unsigned>(PC-memory & 0xFFF), counter_);*/

		switch (PC[0] & 0xF0u)
		{
		
		/* -------------------------------*/
		/* ARITHMETIC                     */
		/* -------------------------------*/
		case 0x00u:
			switch (PC[0] & 0x0Fu)
			{
			case 0x0u:
				A += XX;
				goto next_instruction;
			case 0x1u:
				A = XX;
				goto next_instruction;
			case 0x2u:
				A ^= XX;
				goto next_instruction;
			case 0x3u:
				A |= XX;
				goto next_instruction;
			case 0x4u:
				A &= XX;
				goto next_instruction;
			default:
				goto invalid_instruction;
			};
		case 0x80u:
			A = *_XXX_;
			goto next_instruction;
		case 0xD0u:
			if (*_XXX_map == MEMORY_UNBLOCKED)
				*_XXX_ ^= A;
			goto next_instruction;
		case 0xF0u:
			if (*_XXX_map == MEMORY_UNBLOCKED)
				*_XXX_ = A;
			goto next_instruction;

		/* -------------------------------*/
		/* FLOW CONTROL                   */
		/* -------------------------------*/
		case 0x20u:
			PC = memory + XXX;
			goto jump;
		case 0x30u:
			if (A == 0x00) {
				PC = _XXX_;
				goto jump;
			}
			goto next_instruction;
		case 0x40u:
			if (A == 0x01) {
				PC = _XXX_;
				goto jump;
			}
			goto next_instruction;
		case 0x50u:
			if (A == 0xFF) {
				PC = _XXX_;
				goto jump;
			}
			goto next_instruction;
		case 0x60u:
			if ((instruction & 0xFF00) != 0x6000)
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
		case 0x70u:
			if (A > * _XXX_) {
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
			
		/* -------------------------------*/
		/* SECURITY 	                  */
		/* -------------------------------*/
		case 0x90u:
			*_XXX_map = MEMORY_BLOCKED;
			goto next_instruction;
		case 0xA0u:
			*_XXX_map = MEMORY_UNBLOCKED;
			goto next_instruction;
		case 0xC0u:
			if (*_XXX_map == MEMORY_UNBLOCKED)
				*_XXX_ ^= 0x42;
			goto next_instruction;
		default:
			break;
		}

		/* -------------------------------*/
		/* SPECIAL INSTRUCTIONS           */
		/* -------------------------------*/
		switch (instruction)
		{
		case 0x1337: /* PRINT A */
			putchar(A);
			fflush( stdout );
			goto next_instruction;
		case 0xBEEF: /* RESET */
			PC = memory + 0x100;
			A = 0x42;
			goto jump;
		case 0xEEEE: /* NOP */
			goto next_instruction;
		default:
			break;
		}

	invalid_instruction:
		A--;
	next_instruction:
		PC += 2;
	jump:
	if(PC >= memory + RAM_SIZE)
		return 0;
	continue;
	}

	return 0;
}