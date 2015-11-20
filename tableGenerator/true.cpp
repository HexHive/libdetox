
#include <iostream>
#include <fstream>

#include "table_generator.h"

bool isLeaOp (const unsigned char* opcode, const instr& disasmInfo, std::string& action) {

    if(*opcode == 0x8D) {
		action = "action_lea";
		return false;
	}

	return true;
}

bool isMovOp (const unsigned char* opcode, const instr& disasmInfo, std::string& action) {

    if(*opcode == 0xC7) {
		action = "action_mov_imm_to_r";
		return false;
	}

	return true;
}

int main () {

	std::ofstream outputFile ("fbt_opcode_tables.h");

	addAnalysFunction(isLeaOp);
	addAnalysFunction(isMovOp);

	if (outputFile.is_open()) {
		generateTables(outputFile, "");
	}
}

