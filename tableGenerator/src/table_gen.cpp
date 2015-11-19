
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>


#include "table_generator.h"
#include "i386_opcode.h"
#include "i386_opcode.map"


/* local types */
typedef std::list<analysFunction> functionList;
typedef enum opcode_info {
	OI_none,
	OI_opcodeExtendsIntoFPU,
	OI_opcodeExtendsIntoModRM,
	OI_opcodeIsInModRM,
} opcode_info_t;

/*
 * local functions (private)
 * these are used within the table generator
 */
void generateOpcodeTable(std::ofstream& out, const std::string &prefix, const std::string comment, unsigned int start, unsigned int length, unsigned int step);
instr_t getDisasmTableEntry (const unsigned char* byte, const instr_t* const table, const unsigned int opcodeSize, opcode_info_t &opcodeInfo);
std::string generateTableName(const std::string &prefix, const unsigned int opcode, const unsigned int opcodeSize);
std::string generateOperandString(const unsigned int operandFlags, const unsigned int implOperandFlags, bool isRelativeOffset);
std::string generateImplizitOperandString(const unsigned int implOperandFlags);
std::string uppercase(const std::string &str);
std::string imm2Flag(const unsigned int size);
bool is_modrm_table (const instr_t * const table);
bool is_fpu_table (const instr_t * const table);
instr_t* getComplementFPUTable(const instr_t * const table);

/* global vars */
functionList functions;



/**********************************************************\
 * INTERFACE
\**********************************************************/

void generateTables(std::ofstream& out, std::string prefix)
{
	std::string line, PREFIX(uppercase(prefix));

	if (PREFIX == "") {
		PREFIX = "FBT";
	}


	out << "/***  !! THIS IS A GENERATED FILE !!  ***/" << std::endl;
	out << "/***  DO NOT MODIFY THIS FILE - ALL CHANGES WILL BE OVERWRITTEN WHEN A NEW VERSION IS GENERATED  ***/" << std::endl;
	out << std::endl << std::endl;
	out << "#ifndef " << PREFIX << "_OPCODE_TABLES_H" << std::endl;
	out << "#define " << PREFIX << "_OPCODE_TABLES_H" << std::endl;
	out << std::endl << std::endl;


	/* generate the modRM tables */
	out << "/*" << std::endl;
	out << " * tables for opcode extensions in ModR/M byte." << std::endl;
	out << " * The opcodes extend into the reg bits (5, 4, 3) of the ModR/M byte." << std::endl;
	out << " */" << std::endl << std::endl;

	generateOpcodeTable(out, prefix, "group 1 OPCODE 0x80", 0x8000, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 1 OPCODE 0x81", 0x8100, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 1 OPCODE 0x82", 0x8200, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 1 OPCODE 0x83", 0x8300, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 2 OPCODE 0xC0", 0xC000, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 2 OPCODE 0xC1", 0xC100, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 2 OPCODE 0xD0", 0xD000, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 2 OPCODE 0xD1", 0xD100, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 2 OPCODE 0xD2", 0xD200, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 2 OPCODE 0xD3", 0xD300, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 3 OPCODE 0xF6", 0xF600, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 3 OPCODE 0xF7", 0xF700, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 4 OPCODE 0xFE", 0xFE00, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 5 OPCODE 0xFF", 0xFF00, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 6 OPCODE 0x0F00", 0x0F0000, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 7 OPCODE 0x0F01", 0x0F0100, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 8 OPCODE Ox0FBA", 0x0FBA00, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 9 OPCODE Ox0FC7", 0x0FC700, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 12 OPCODE Ox0F71", 0x0F7100, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 13 OPCODE Ox0F72", 0x0F7200, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 14 OPCODE Ox0F73", 0x0F7300, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 15 OPCODE Ox0FAE", 0x0FAE00, 8, 0x08);
	generateOpcodeTable(out, prefix, "group 16 OPCODE Ox0F18", 0x0F1800, 8, 0x08);


	/* generate the fpu tables */
	out << std::endl << std::endl << "/*" << std::endl;
	out << " * tables for fpu opcodes." << std::endl;
	out << " */" << std::endl << std::endl;

	generateOpcodeTable(out, prefix, "fpu OPCODE 0xD8", 0xD800, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xD9", 0xD900, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xDA", 0xDA00, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xDB", 0xDB00, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xDC", 0xDC00, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xDD", 0xDD00, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xDE", 0xDE00, 256, 0x01);
	generateOpcodeTable(out, prefix, "fpu OPCODE 0xDF", 0xDF00, 256, 0x01);


	/* generate the one and two byte tables */
	out << std::endl << std::endl << "/*" << std::endl;
	out << " * tables for one and two byte opcodes." << std::endl;
	out << " */" << std::endl << std::endl;

	generateOpcodeTable(out, prefix, "table for two byte opcodes", 0x0F00, 256, 0x01);
	generateOpcodeTable(out, prefix, "table for one byte opcodes", 0x00, 256, 0x01);


	/* close the ifdef from the begining */
	out << "#endif  /* " << PREFIX << "_OPCODE_TABLES_H */" << std::endl << std::endl;

}


void addAnalysFunction(analysFunction f) {
	functions.push_back(f);
}



/**********************************************************\
 * IMPLEMENTATION
\**********************************************************/

void generateOpcodeTable(std::ofstream& out, const std::string &prefix, const std::string comment, unsigned int start, unsigned int length, unsigned int step)
{
	unsigned char buf[4];
	std::string opcodeFlags, opcodeTable, opcodeAction, opcodeComment, tableName;
	unsigned int opcodeSize, opcode, opcodeIdx, flags;
	bool isOpcodePrefix, isRelativeOffset;
	opcode_info_t opcodeInfo;

// DEBUG
//	std::cout << "generating table " << std::hex << start << std::endl;

	/* calculate the size of the opcodes in this table */
	if (start <= 0x000000FF) {
		opcodeSize = 1;
	} else if (start <= 0x0000FFFF) {
		opcodeSize = 2;
	} else if (start <= 0x00FFFFFF) {
		opcodeSize = 3;
	} else {
		opcodeSize = 4;
	}

// DEBUG
//	std::cout << "generating table name -> ";

	/* generate the table name */
	tableName = generateTableName(prefix, start >> 8, opcodeSize-1);

// DEBUG
//	std::cout << tableName << std::endl;

	/* output the the table header */
	out << "/** " << comment << " */" << std::endl;
	out << "struct ia32_opcode " << tableName << "[] = {" << std::endl;


	/* loop which generates all the entries of the table */
	for(opcodeIdx = 0; opcodeIdx < length; opcodeIdx++) {

		/* generate an empry line after 8 opcodes */
		if (opcodeIdx%8 == 0 && opcodeIdx > 2) {
			out << std::endl;
		}

		/* the current opcode */
		opcode = start + opcodeIdx*step;

// DEBUG
//		std::cout << std::hex << opcode << "     ";

		/* init the buffer with the current opcode */
		buf[0] = (opcode >> 8*(opcodeSize-1)) & 0xFF;
		buf[1] = (opcode >> 8*(opcodeSize-2)) & 0xFF;
		buf[2] = (opcode >> 8*(opcodeSize-3)) & 0xFF;
		buf[3] = (opcode >> 8*(opcodeSize-4)) & 0xFF;

// DEBUG
/*
		std::cout << std::hex << ((unsigned int)buf[0] >> 4);
		std::cout << std::hex << ((unsigned int)buf[0] & 0x0F) << " ";
		std::cout << std::hex << ((unsigned int)buf[1] >> 4);
		std::cout << std::hex << ((unsigned int)buf[1] & 0x0F) << " ";
		std::cout << std::hex << ((unsigned int)buf[2] >> 4);
		std::cout << std::hex << ((unsigned int)buf[2] & 0x0F) << " ";
		std::cout << std::hex << ((unsigned int)buf[3] >> 4);
		std::cout << std::hex << ((unsigned int)buf[3] & 0x0F) << " ";
		std::cout << std::endl;
*/

		/* get the disasm table entry for this instruction */
		instr disasmInst = getDisasmTableEntry(buf, table_opcode_onebyte, opcodeSize, opcodeInfo);

// DEBUG
//		std::cout << "entry found" << std::endl;


		isOpcodePrefix = (disasmInst.opcodeFlags & INS_PREFIX_MASK) != 0;

		/* get the flags together... */
		opcodeFlags = "ARG_NONE";


		/* first immediate arguments */
		if (isImmediateOperand(disasmInst.destFlags)) {
			opcodeFlags += " | " + imm2Flag(disasmInst.destFlags);
		}
		if (isImmediateOperand(disasmInst.srcFlags)) {
			opcodeFlags += " | " + imm2Flag(disasmInst.srcFlags);
		}
		if (isImmediateOperand(disasmInst.auxFlags)) {
			opcodeFlags += " | " + imm2Flag(disasmInst.auxFlags);
		}

		/* second modrm arguments */
		flags = 0;
		if (isModRMOperand(disasmInst.destFlags)) flags = disasmInst.destFlags;
		if (isModRMOperand(disasmInst.srcFlags))  flags = disasmInst.srcFlags;
		if (isModRMOperand(disasmInst.auxFlags))  flags = disasmInst.auxFlags;

		if (flags != 0) {
			opcodeFlags += " | ARG_RM";
		}

		/* last check for prefix and escapes */
		/* these overwrite any other flags */
		if (disasmInst.table != NULL) {
			if (opcodeInfo == OI_opcodeExtendsIntoModRM) {
				opcodeFlags = "OPCODE_RM";
			} else if (opcodeInfo == OI_opcodeExtendsIntoFPU) {
				opcodeFlags = "FPU_ESC";
			} else {
				opcodeFlags = "OPCODE_ESC";
			}
		} else if (isOpcodePrefix) {
			opcodeFlags = "PREFIX_ESC";
		}

		/* remove a ARG_NONE if it is followed by more arguments */
		if (opcodeFlags.find("ARG_NONE | ", 0) != std::string::npos) {
			opcodeFlags = opcodeFlags.substr(11);
		}


// DEBUG
//		std::cout << "flags fixed" << std::endl;


		/* fix a follow up table if there is any */
		if (disasmInst.table != NULL) {
			opcodeTable = generateTableName(prefix, opcode, opcodeSize);
		} else if (isOpcodePrefix) {
			opcodeTable = generateTableName(prefix, 0x00, 0);
		} else {
			opcodeTable = "0";
		}


// DEBUG
//		std::cout << "follow up table fixed" << std::endl;



		/* generate a comment */
		std::stringstream temp;
		opcodeComment = "";

		/* the opcode in HEX */
		if (opcodeInfo == OI_opcodeIsInModRM) {
			temp << std::hex << ((opcode >> 5) & 0x01);
			temp << std::hex << ((opcode >> 4) & 0x01);
			temp << std::hex << ((opcode >> 3) & 0x01);
		} else {
			opcodeComment += "0x";
			for (int i=opcodeSize-1; i>=0; i--) {
				temp << std::hex << ((opcode >> (i*8 + 4)) & 0x0F);
				temp << std::hex << ((opcode >> (i*8)) & 0x0F);
			}
		}
		opcodeComment += uppercase(temp.str());

		/* the MNEMONIC of this opcode */
		opcodeComment += " " + uppercase(disasmInst.mnemonic);

		/* the arguments or the predefined comment */
		if (disasmInst.comment != "") {
			opcodeComment += " " + disasmInst.comment;
		} else {
			isRelativeOffset = (disasmInst.opcodeFlags & INS_GROUP_MASK) == EXEC;

			if (disasmInst.srcFlags != NONE) {
				opcodeComment += " " + generateOperandString(disasmInst.srcFlags, disasmInst.implSrcFlags, isRelativeOffset);
			}
			if (disasmInst.auxFlags != NONE) {
				opcodeComment += ", " + generateOperandString(disasmInst.auxFlags, disasmInst.implAuxFlags, isRelativeOffset);
			}
			if (disasmInst.destFlags != NONE) {
				opcodeComment += ", " + generateOperandString(disasmInst.destFlags, disasmInst.implDestFlags, isRelativeOffset);
			}
		}


// DEBUG
//		std::cout << "comment generated" << std::endl;


		/* decide what action to use */
		opcodeAction = disasmInst.defaultAction;
		bool cont = true;
		functionList::iterator it = functions.begin();
		while (cont && it != functions.end()) {
			cont = (**it)(buf, disasmInst, opcodeAction);
			it++;
		}


// DEBUG
//		std::cout << "action fixed" << std::endl;


		// output the line.
		out << "\t{ ";
		out << opcodeFlags << ", " << opcodeTable << ", " << opcodeAction;
		if (opcodeIdx != length-1)
		  out << " },\t/* " << opcodeComment << " */" << std::endl;
		else
		  out << " }\t/* " << opcodeComment << " */" << std::endl;
	}

	out << "};" << std::endl << std::endl << std::endl;
}


bool is_modrm_table (const instr_t * const table)
{
	return (
			table == table_opcode_80_rm ||
			table == table_opcode_81_rm ||
			table == table_opcode_82_rm ||
			table == table_opcode_83_rm ||
			table == table_opcode_C0_rm ||
			table == table_opcode_C1_rm ||
			table == table_opcode_D0_rm ||
			table == table_opcode_D1_rm ||
			table == table_opcode_D2_rm ||
			table == table_opcode_D3_rm ||
			table == table_opcode_F6_rm ||
			table == table_opcode_F7_rm ||
			table == table_opcode_FE_rm ||
			table == table_opcode_FF_rm ||
			table == table_opcode_0F00_rm ||
			table == table_opcode_0F01_rm ||
			table == table_opcode_0F18_rm ||
			table == table_opcode_0F71_rm ||
			table == table_opcode_0F72_rm ||
			table == table_opcode_0F73_rm ||
			table == table_opcode_0FAE_rm ||
			table == table_opcode_0FBA_rm ||
			table == table_opcode_0FC7_rm
		);
}


bool is_fpu_table (const instr_t * const table)
{
	return (
			table == table_opcode_D8_fpu_rm ||
			table == table_opcode_D9_fpu_rm ||
			table == table_opcode_DA_fpu_rm ||
			table == table_opcode_DB_fpu_rm ||
			table == table_opcode_DC_fpu_rm ||
			table == table_opcode_DD_fpu_rm ||
			table == table_opcode_DE_fpu_rm ||
			table == table_opcode_DF_fpu_rm
		);
}

instr_t* getComplementFPUTable(const instr_t * const table)
{
	if (table == table_opcode_D8_fpu_rm) {
		return table_opcode_D8_fpu_others;
	}
	if (table == table_opcode_D9_fpu_rm) {
		return table_opcode_D9_fpu_others;
	}
	if (table == table_opcode_DA_fpu_rm) {
		return table_opcode_DA_fpu_others;
	}
	if (table == table_opcode_DB_fpu_rm) {
		return table_opcode_DB_fpu_others;
	}
	if (table == table_opcode_DC_fpu_rm) {
		return table_opcode_DC_fpu_others;
	}
	if (table == table_opcode_DD_fpu_rm) {
		return table_opcode_DD_fpu_others;
	}
	if (table == table_opcode_DE_fpu_rm) {
		return table_opcode_DE_fpu_others;
	}
	if (table == table_opcode_DF_fpu_rm) {
		return table_opcode_DF_fpu_others;
	}

	return NULL;
}


instr_t getDisasmTableEntry (const unsigned char* byte, const instr_t* table, const unsigned int opcodeSize, opcode_info_t &opcodeInfo)
{
	//  DEBUG: prints the arguments
	/*
	if(tableIdx == 0) std::cout << std::endl;
	std::cout << std::hex << (((unsigned int) *byte) >> 4);
	std::cout << std::hex <<  ((unsigned int)(*byte) & (unsigned int)15) << " ";
	std::cout << tableIdx << std::endl;
	*/

	unsigned int idx = *byte;

	opcodeInfo = OI_none;

	/* check if this a modrm table */
	bool opcodeIsInModRM = is_modrm_table(table);
	if (opcodeIsInModRM) {
		idx = (idx >> 3) & 7;
		opcodeInfo = OI_opcodeIsInModRM;

	/* check if this is a fpu table */
	} else if (is_fpu_table(table)) {
		/* the fpu tables are different depending if the byte is < 0xC0 */
		if (idx < 0xC0) {
			/* these tables are like modrm tables */
			idx = (idx >> 3) & 7;
		} else {
			/* the remaining entries are a normal table (thanks to intel for this great idea) */
			table = getComplementFPUTable(table);
			idx = idx - 0xC0;
		}
	}

	/* get the entry from the table */
	instr_t *entry = (instr_t*)table + idx;

	/* if this entry is no prefix and no escape or if it is the last byte of the opcode return it */
	if (entry->table == 0 || opcodeSize == 1) {
		if (is_modrm_table(entry->table)) {
			opcodeInfo = OI_opcodeExtendsIntoModRM;
		} else if (is_fpu_table(entry->table)) {
			opcodeInfo = OI_opcodeExtendsIntoFPU;
		}
		return *entry;
	}

	/* otherwise call recursive since this byte is either a escape or a prefix */
	return getDisasmTableEntry(byte+1, entry->table, opcodeSize-1, opcodeInfo);
}


std::string generateTableName(const std::string &prefix, const unsigned int opcode, const unsigned int opcodeSize)
{
	std::stringstream temp;
	std::string name;

	if (prefix != "") {
		name = prefix + "_opcode_table_";
	} else {
		name = "opcode_table_";
	}


	if (opcodeSize == 0) {
		name += "onebyte";
	} else if (opcode == 0x0F) {
		name += "twobyte";
	} else {
		for (int i=opcodeSize-1; i>=0; i--) {
			temp << std::hex << ((opcode >> (i*8 + 4)) & 0x0F);
			temp << std::hex << ((opcode >> (i*8)) & 0x0F);
		}
		name += uppercase(temp.str());

		if (opcode >= 0xD8 && opcode <= 0xDF) {
			name += "_fpu";
		} else {
			name += "_rm";
		}
	}

	return name;
}


std::string generateOperandString(const unsigned int operandFlags, const unsigned int implOperandFlags, bool isRelativeOffset)
{
	std::stringstream temp;
	std::string result = "";

	/* if the operand is implizit we output it as detailed as possible */
	if (isImplizitOperand(operandFlags)) {
		if ((operandFlags & CONST) != 0) {
			temp << implOperandFlags;
			result = "$" + temp.str();
		} else {
			result = generateImplizitOperandString(implOperandFlags);
		}

	/* if the operand is immediate output the size */
	} else if (isImmediateOperand(operandFlags)) {
		temp << operandSize(operandFlags)*8;
		if (isRelativeOffset) {
			result = "rel" + temp.str();
		} else if (mayOperandAccessMemory(operandFlags)) {
			result = "(imm" + temp.str() + ")";
		} else {
			result = "imm" + temp.str();
		}

	/* if the operand is in the modRM byte decide if it is a memory or a register or either one of them */
	} else if (isModRMOperand(operandFlags)) {
		temp << operandSize(operandFlags)*8;
		if (mayOperandAccessMemory(operandFlags)) {
			if (mayOperandAccessRegister(operandFlags)) {
				result = "r/m" + temp.str();
			} else {
				result = "m" + temp.str();
			}
		} else {
			result = "reg" + temp.str();
		}
	}

	return result;
}


std::string generateImplizitOperandString(const unsigned int implOperandFlags)
{
//DEBUG
//	std::cout << implOperandFlags << std::endl;

	/* tables with the names of the registers */
	std::string byte_register_names[]    = { "AL",    "CL",    "DL",    "BL",    "AH",    "CH",    "DH",    "BH"    };
	std::string word_register_names[]    = { "AX",    "CX",    "DX",    "BX",    "SP",    "BP",    "SI",    "DI"    };
	std::string dword_register_names[]   = { "EAX",   "ECX",   "EDX",   "EBX",   "ESP",   "EBP",   "ESI",   "EDI"   };
	std::string mmx_register_names[]     = { "MM0",   "MM1",   "MM2",   "MM3",   "MM4",   "MM5",   "MM6",   "MM7"   };
	std::string xmm_register_names[]     = { "XMM0",  "XMM1",  "XMM2",  "XMM3",  "XMM4",  "XMM5",  "XMM6",  "XMM7"  };
	std::string seg_register_names[]     = { "ES",    "CS",    "SS",    "DS",    "FS",    "GS",     "",     ""      };
	std::string test_register_names[]    = { "TR0",   "TR1",   "TR2",   "TR3",   "TR4",   "TR5",   "TR6",   "TR7"   };
	std::string fpu_register_names[]     = { "ST(0)", "ST(1)", "ST(2)", "ST(3)", "ST(4)", "ST(5)", "ST(6)", "ST(7)" };


	std::stringstream temp;
	std::string result;

	std::string *table = 0;

	switch (implOperandFlags & REG_TYPE_MASK) {
		case BYTE_REG:
			table = byte_register_names;
			break;
		case WORD_REG:
			table = word_register_names;
			break;
		case DWORD_REG:
			table = dword_register_names;
			break;
		case MMX_REG:
			table = mmx_register_names;
			break;
		case XMM_REG:
			table = xmm_register_names;
			break;
		case SEGMENT_REG:
			table = seg_register_names;
			break;
		case TEST_REG:
			table = test_register_names;
			break;
		case FPU_REG:
			table = fpu_register_names;
			break;
	}

	return table[implOperandFlags & REG_IDX_MASK];
}


std::string uppercase(const std::string &str)
{
	std::string result(str);

	for (unsigned int i = 0; i < str.length(); i++) {
		result[i]=toupper(str[i]);
	}

	return result;
}


std::string imm2Flag(const unsigned int flags)
{
	std::string result = "";
	switch (operandSize(flags)) {
		case 1:
			result = "ARG_B";
			break;
		case 2:
			result = "ARG_W";
			break;
		case 4:
			result = "ARG_D";
			break;
		case 6:
			result = "ARG_P";
			break;
		case 8:
			result = "ARG_Q";
			break;
		case 10:
			result = "ARG_S";
			break;
		default:
			break;
	}

	return result;
}



