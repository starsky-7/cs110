/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#include "translate.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "tables.h"
#include "translate_utils.h"


static const PseudoHandler pseudo_handlers[] = {
    {"beqz", transform_beqz}, {"bnez", transform_bnez}, {"li", transform_li},
    {"mv", transform_mv},     {"j", transform_j},       {"jr", transform_jr},
    {"jal", transform_jal},   {"jalr", transform_jalr}, {"lw", transform_lw},
};

/* 
Fields per entry:
  - const char* name;         -- instr name
  - InstrType instr_type;     -- instr format, e.g. R_type
  - uint8_t opcode;
  - uint8_t funct3;
  - uint8_t funct7;           -- funct7 or partial imm
  - ImmType imm_type;         -- imm type (see translate_utils.h)
*/
static const InstrInfo instr_table[] = {
    // R-type instructions
    {"add", R_TYPE, 0x33, 0x0, 0x00, IMM_NONE},

    {"sub", R_TYPE, 0x33, 0x0, 0x20, IMM_NONE},
    {"xor", R_TYPE, 0x33, 0x4, 0x00, IMM_NONE},
    {"or", R_TYPE, 0x33, 0x6, 0x00, IMM_NONE},
    {"and", R_TYPE, 0x33, 0x7, 0x00, IMM_NONE},
    {"sll", R_TYPE, 0x33, 0x1, 0x00, IMM_NONE},
    {"srl", R_TYPE, 0x33, 0x5, 0x00, IMM_NONE},
    {"sra", R_TYPE, 0x33, 0x5, 0x20, IMM_NONE},
    {"slt", R_TYPE, 0x33, 0x2, 0x00, IMM_NONE},
    {"sltu", R_TYPE, 0x33, 0x3, 0x00, IMM_NONE},
    {"mul", R_TYPE, 0x33, 0x0, 0x01, IMM_NONE},
    {"mulh", R_TYPE, 0x33, 0x1, 0x01, IMM_NONE},
    {"div", R_TYPE, 0x33, 0x4, 0x01, IMM_NONE},
    {"rem", R_TYPE, 0x33, 0x6, 0x01, IMM_NONE},

    // I-type instructions
    {"addi", I_TYPE, 0x13, 0x0, 0x00, IMM_12_SIGNED},
    {"xori", I_TYPE, 0x13, 0x4, 0x00, IMM_12_SIGNED},
    {"ori", I_TYPE, 0x13, 0x6, 0x00, IMM_12_SIGNED},
    {"andi", I_TYPE, 0x13, 0x7, 0x00, IMM_12_SIGNED},
    {"slli", I_TYPE, 0x13, 0x1, 0x00, IMM_5_UNSIGNED},
    {"srli", I_TYPE, 0x13, 0x5, 0x00, IMM_5_UNSIGNED},
    {"srai", I_TYPE, 0x13, 0x5, 0x20, IMM_5_UNSIGNED},
    {"slti", I_TYPE, 0x13, 0x2, 0x00, IMM_12_SIGNED},
    {"sltiu", I_TYPE, 0x13, 0x3, 0x00, IMM_12_SIGNED},
    {"lb", I_TYPE, 0x03, 0x0, 0x00, IMM_12_SIGNED},
    {"lh", I_TYPE, 0x03, 0x1, 0x00, IMM_12_SIGNED},
    {"lw", I_TYPE, 0x03, 0x2, 0x00, IMM_12_SIGNED},
    {"lbu", I_TYPE, 0x03, 0x4, 0x00, IMM_12_SIGNED},
    {"lhu", I_TYPE, 0x03, 0x5, 0x00, IMM_12_SIGNED},
    {"jalr", I_TYPE, 0x67, 0x0, 0x00, IMM_12_SIGNED},
    {"ecall", I_TYPE, 0x73, 0x0, 0x00, IMM_NONE},

    // S-type instructions
    {"sb", S_TYPE, 0x23, 0x0, 0x00, IMM_12_SIGNED},
    {"sh", S_TYPE, 0x23, 0x1, 0x00, IMM_12_SIGNED},
    {"sw", S_TYPE, 0x23, 0x2, 0x00, IMM_12_SIGNED},

    // SB-type instructions
    {"beq", SB_TYPE, 0x63, 0x0, 0x00, IMM_13_SIGNED},
    {"bne", SB_TYPE, 0x63, 0x1, 0x00, IMM_13_SIGNED},
    {"blt", SB_TYPE, 0x63, 0x4, 0x00, IMM_13_SIGNED},
    {"bge", SB_TYPE, 0x63, 0x5, 0x00, IMM_13_SIGNED},
    {"bltu", SB_TYPE, 0x63, 0x6, 0x00, IMM_13_SIGNED},
    {"bgeu", SB_TYPE, 0x63, 0x7, 0x00, IMM_13_SIGNED},

    // U-type instructions
    {"lui", U_TYPE, 0x37, 0x0, 0x00, IMM_20_UNSIGNED},
    {"auipc", U_TYPE, 0x17, 0x0, 0x00, IMM_20_UNSIGNED},

    // UJ-type instructions
    {"jal", UJ_TYPE, 0x6f, 0x0, 0x00, IMM_21_SIGNED},
};

unsigned transform_beqz(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args != 2) return 0;
  char *new_arg[] = {args[0], "x0", args[1]};
  int result = add_to_block(blk, "beq", new_arg, 3);
  /* === end === */
  return result == -1 ? 0 : 1;
}

unsigned transform_bnez(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args!= 2) return 0;
  char *new_arg[] = {args[0], "x0", args[1]};
  int result = add_to_block(blk, "bne", new_arg, 3);
  /* === end === */
  return result == -1 ? 0 : 1;
}

/* Hint:
  - make sure that the number is representable by 32 bits. (Hint: the number
      can be both signed or unsigned).
  - if the immediate can fit in the imm field of an addiu instruction, then
      expand li into a single addi instruction. Otherwise, expand it into
      a lui-addi pair.

  venus has slightly different translation rules for li, and it allows numbers
  larger than the largest 32 bit number to be loaded with li. You should follow
  the above rules if venus behaves differently.
*/
unsigned transform_li(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) return 0;
  // long int* ac_num = malloc(sizeof(long int));
  char* end_ptr;
  long int num_64 = strtol(args[1], &end_ptr, 0);
  if (num_64 < -2147483648 || num_64 > 2147483647) return 0;
  int num_32 = strtol(args[1], &end_ptr, 0);
  if (*end_ptr != '\0') {
    return -1; /* not a valid number */
  }
  // num_32 = (num_32 & 0xFFFFFFFF);
  uint32_t upper = (num_32 >> 12) & 0xFFFFF;
  int32_t lower = num_32 & 0xFFF;
  // printf("%d\t%d\n", upper, lower);
  if ((num_32 >= -2048) && (num_32 <= 2047)){
    char* new_args[3];
    char lownum[32];
    if (lower > 2047) lower -= 4096;
    snprintf(lownum, 32, "%d", lower);
    new_args[0] = args[0];
    new_args[1] = "x0";
    new_args[2] = lownum;
    int a = add_to_block(blk, "addi", new_args, 3);
    return a == -1 ? 0 : 1;

  }else{
    // under zero
    if (lower >= 2048) {
      upper += 1;
      lower -= 4096;
      // printf("%d\t%d\tjin\n", upper, lower);
    }
    char* lui_args[2];
    char* addi_args[3];
    char luinum[32];
    char addinum[32];
    lui_args[0] = args[0];
    addi_args[0] = args[0];
    snprintf(luinum, 32, "%u", upper);
    lui_args[1] = luinum;
    snprintf(addinum, 32, "%d", lower);
    addi_args[1] = args[0];
    addi_args[2] = addinum;
    int b = add_to_block(blk, "lui", lui_args, 2);
    if ( b < 0 ) return 0;
    int a = add_to_block(blk, "addi", addi_args, 3);
    return a < 0 ? 1 : 2;
  }
  /* === end === */
  return 0;
}

/* Hint:
  - your expansion should use the fewest number of instructions possible.
 */
unsigned transform_mv(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args != 2) return 0;
  char *new_arg[] = {args[0], args[1], "0"};
  int result = add_to_block(blk, "addi", new_arg, 3);
  

  /* === end === */
  return result == -1 ? 0 : 1;
}

unsigned transform_j(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args!= 1) return 0;
  char *new_arg[] = {"x0", args[0]};
  int result = add_to_block(blk, "jal", new_arg, 2);
  /* === end === */
  return result == -1 ? 0 : 1;
}

unsigned transform_jr(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args!= 1) return 0;
  char *new_arg[] = {"x0", args[0], "0"};
  int result = add_to_block(blk, "jalr", new_arg, 3);


  /* === end === */
  return result == -1 ? 0 : 1;
}

/* Hint:
  - Since handler is selected by instruction name, be careful about
    pseudo/regular instruction name collisions
 */
unsigned transform_jal(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args == 2) {
    int result = add_to_block(blk, "jal", args, num_args);
    return result == -1 ? 0 : 1;
  }
  if(num_args == 1) {
    char *new_arg[] = {"x1", args[0]};
    int result = add_to_block(blk, "jal", new_arg, 2);
    return result == -1 ? 0 : 1;
    }
  /* === end === */
  return 0;
}

/* Hint:
  - Since handler is selected by instruction name, be careful about
    pseudo/regular instruction name collisions
 */
unsigned transform_jalr(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args == 1){
    char *new_arg[] = {"x1", args[0], "0"};
    int result = add_to_block(blk, "jalr", new_arg, 3);
    return result == -1 ? 0 : 1;
  }
  if(num_args == 3){
    int result = add_to_block(blk, "jalr", args, num_args);
    return result == -1 ? 0 : 1;
  }
  
  else {
    int result = add_to_block(blk, "jalr", args, num_args);
    return result == -1 ? 0 : 1; 
}
  /* === end === */
}

/* Hint:
 * - You should leave the label AS IS and resolve it in pass 2.
 */
unsigned transform_lw(Block* blk, char** args, int num_args) { // fixme:
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args == 3) {
    int result = add_to_block(blk, "lw", args, num_args);
    return result < 0 ? 0 : 1;
  } else if (num_args == 2) {
    int result = add_to_block(blk, "auipc", args, num_args);
    if (result < 0) return 0;
    char *new_arg[3] = {args[0], args[1], args[0]};
    int b = add_to_block(blk, "lw", new_arg, 3);
    return b < 0 ? 1 : 2;
  } else {
    return 0;
  }
  /* === end === */
  
}

/* Traverse pseudo_handlers table to select corresponding handler by NAME */
const PseudoHandler* find_pseudo_handler(const char* name) {
  for (size_t i = 0; i < sizeof(pseudo_handlers) / sizeof(pseudo_handlers[0]);
       i++) {
    if (strcmp(name, pseudo_handlers[i].name) == 0) {
      return &pseudo_handlers[i];
    }
  }
  return NULL;
}

/* Writes instructions during the assembler's first pass to BLK. The case
   for pseudo-instructions will be handled by handlers, but you need to
   write code to complete these TRANSFORM functions, as well as dealing
   with general instructions. Your pseudoinstruction expansions should not
   have any side effects.

   BLK is the intermediate instruction block you should write to,
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are
   valid, since that will be checked in part two.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(Block* blk, const char* name, char** args,
                        int num_args) {
  /* Deal with pseudo-instructions */
  const PseudoHandler* handler = find_pseudo_handler(name);
  if (handler) {
    return handler->transform(blk, args, num_args);
  }
  /* What about general instructions? */
  /* IMPLEMENT ME */
  /* === start === */
  // for (size_t i = 0; i < sizeof(instr_table)/sizeof(instr_table[0]); i++) {
  //   const InstrInfo* info = &instr_table[i];
  //   if (strcmp(name, info->name) == 0) {
  //     switch (info->instr_type) {
  //       case R_TYPE:
  //         if (num_args!= 3) return 0;
  //         break;
  //       case SB_TYPE:
  //         if (num_args != 3) return 0;
  //         break;
  //       case I_TYPE:
  //         if (strcmp(name, "ecall") == 0) {
  //           if (num_args != 0) return 0;
  //         } else if (num_args != 3) {
  //           return 0;
  //         }
  //         break;
  //       case S_TYPE:
  //         if (num_args != 2) return 0;
  //         break;
  //       case U_TYPE:
  //         if (num_args != 2) return 0;
  //         break;
  //       case UJ_TYPE: // fixme:也许在pseduo中已经处理过了？
  //         if (num_args < 1 || num_args > 2) return 0;
  //         break;
  //       default:
  //         return 0;
  //     }
  int result = add_to_block(blk, name, args, num_args);
  
  /* === end === */
  return result < 0 ? 0 : 1;
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step.

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write
   anything to OUTPUT but simply return -1. venus may be a useful resource for
   this step.

   All regular instruction information is given at `instr_table`.

   Note the use of helper functions. Consider writing your own! If the function
   definition comes afterwards, you must declare it first (see translate.h).

   Returns 0 on success and -1 on error.
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args,
                   uint32_t addr, SymbolTable* symtbl) {
  for (size_t i = 0; i < sizeof(instr_table) / sizeof(instr_table[0]); i++) {
    const InstrInfo* info = &instr_table[i];
    if (strcmp(name, info->name) == 0) {
      switch (info->instr_type) {
        case R_TYPE:
          return write_rtype(output, info, args, num_args);
        case I_TYPE:
          return write_itype(output, info, args, num_args, addr, symtbl);
        case S_TYPE:
          return write_stype(output, info, args, num_args);
        case SB_TYPE:
          return write_sbtype(output, info, args, num_args, addr, symtbl);
        case U_TYPE:
          return write_utype(output, info, args, num_args, addr, symtbl);
        case UJ_TYPE:
          return write_ujtype(output, info, args, num_args, addr, symtbl);
      }
    }
  }
  return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  int rd, rs1, rs2;
  uint32_t instruction = 0;

  if(num_args!= 3) return -1;
  rd = translate_reg(args[0]);
  rs1 = translate_reg(args[1]);
  rs2 = translate_reg(args[2]);
  
  if(rd == -1 || rs1 == -1 || rs2 == -1) return -1;
  instruction = (info->opcode) + (rd << 7) + (info->funct3 << 12) + (rs1 << 15) + (rs2 << 20) + (info->funct7 << 25);
  write_inst_hex(output, instruction);
  /* === end === */
  return 0;
}

/* Hint:
  - Number of ARGS and immediate range of each I_type instruction may
  vary. Refer to RISC-V green card and design proper encoding rules.
  - Some instruction(s) expanded from pseudo ones may has(have) unresolved
  label(s). You need to take that special case into consideration. Refer to
  write_sbtype for detailed relative address calculation.
 */
int write_itype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  long imm;
  uint32_t instruction = 0;

  if (strcmp("ecall", info->name) == 0){
    if (num_args != 0) {return -1;}
    instruction |= (0x000 & 0xFFF) << 20; // imm
    instruction |= (0x00 & 0x1F) << 15; // rs1
    instruction |= (0x00 & 0x1F) << 7; // rd
  } else if (num_args == 3) {
    int lint = 0;
    if ((info->name[0] == 'l'))
    {
      lint = 1;
    }
    int rd = translate_reg(args[0]);
    if (rd == -1) {return -1;}
    int rs1 = translate_reg(args[1+lint]);
    if (rs1 == -1) {return -1;}  

    imm = -4;      
    long int offset = 0;      
    if (lint == 1){
      imm = get_addr_for_symbol(symtbl, info->name);
    }

    // 不是load指令
    if (strcmp(info->name, "jalr") == 0) {
      imm = get_addr_for_symbol(symtbl, args[2]);
    }
    if (imm < 0) {
      int imme = translate_num(&imm, args[2-lint], info->imm_type);
      if (imme == -1) {return -1;}
      offset = imm;
    } else {
      offset = imm - addr;
      // printf("%ld\n", offset);
    }
    // if (strcmp(info->name, "jalr") == 0) printf("%s\t%s\t%ld\t%s\t\n",info->name, args[0], offset, args[2]);
    if (strcmp(info->name, "srai") == 0) instruction |= 0x40 << 24;
    instruction |= (offset & 0xFFF) << 20; // imm
    instruction |= (rs1 & 0x1F) << 15; // rs1
    instruction |= (rd & 0x1F) << 7; // rd
  } else {

    return -1;
  }

  instruction |= (info->funct3 & 0x7) << 12; // funct3
  instruction |= (info->opcode & 0x7F); // opcode
  write_inst_hex(output, instruction);
  /* === end === */
  return 0;
}

int write_stype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args!= 2) return -1;
  int rs1, rs2;
  long imm;
  uint32_t instruction = 0;
  ImmType imm_type = info->imm_type;

  rs1 = translate_reg(args[2]);
  rs2 = translate_reg(args[0]);
  if(rs1 == -1 || rs2 == -1) return -1;

  int imm_check = translate_num(&imm, args[1], imm_type);
  if(imm_check == -1) return -1;

  instruction = info->opcode + ((imm & 0x0000001F) << 7) + (info->funct3 << 12) + (rs1 << 15) + (rs2 << 20) + ((imm >> 5) << 25);
  write_inst_hex(output, instruction);

  /* === end === */
  return 0;
}

/* Hint:
  - the way for branch to calculate relative address. e.g. bne
     bne rs rt label
   assume the byte_offset(addr) of label is L,
   current instruction byte_offset(addr) is A
   the relative address I for label satisfy:
     L = A + I
   so the relative addres is
     I = L - A
*/
int write_sbtype(FILE* output, const InstrInfo* info, char** args,
                 size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args!= 3) return -1;
  int rs1, rs2;
  uint32_t instruction = 0;
  ImmType imm_type = info->imm_type;

  rs1 = translate_reg(args[0]);
  rs2 = translate_reg(args[1]);
  if(rs1 == -1 || rs2 == -1) return -1;
  
  int64_t L = get_addr_for_symbol(symtbl, args[2]);
  if(L == -1) return -1;

  int64_t A = addr;
  int64_t I = L - A;
  if(is_valid_imm(I, imm_type) == 0){
    return -1;
  }
  instruction = info->opcode + (((I & 0x00000800) >> 11) << 7) + (((I & 0x0000001E) >> 1) << 8) + (info->funct3 << 12) + (rs1 << 15) + (rs2 << 20) + (((I & 0x000007E0) >> 5) << 25) + (((I & 0x00001000) >> 12) << 31);
  write_inst_hex(output, instruction);
  /* === end === */
  return 0;
}

/* Hint:
  - Some instruction(s) expanded from pseudo ones may has(have) unresolved
  label(s). You need to take that special case into consideration. Refer to
  write_sbtype for detailed relative address calculation.
 */
int write_utype(FILE* output, const InstrInfo* info, char** args,
  size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */

  uint32_t instruction = 0;
  long int imm;      
  long int offset = 0;          
  if (num_args != 2) {return -1;}
  int rd = translate_reg(args[0]);
  if (rd == -1) {return -1;}
  imm = get_addr_for_symbol(symtbl, args[1]);
  if (strcmp(info->name, "lui") == 0) {
    int imme = translate_num(&imm, args[1], info->imm_type);
    if (imme == -1) {return -1;}
    offset = imm;
  } 
  else {
    offset = imm - addr;
    if (offset < -2048 || offset > 2047) {
      offset = (offset >> 12) + 1;
    } 
    else {
      offset = 0;
    }
  }

  instruction |= (offset & 0xFFFFF) << 12;          // rs1
  instruction |= (rd & 0x1F) << 7; // rd
  instruction |= (info->opcode & 0x7F);       // opcode
  write_inst_hex(output, instruction);



  /* === end === */
  return 0;
  }

/* In this project there is no need to relocate labels,
   you may think about the reasons. */
int write_ujtype(FILE* output, const InstrInfo* info, char** args,
                 size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  if(num_args!= 2) return -1;
  int rd;
  uint32_t instruction = 0;
  ImmType imm_type = info->imm_type;
  rd = translate_reg(args[0]);
  if(rd == -1) return -1;

  int64_t L = get_addr_for_symbol(symtbl, args[2]);
  if(L == -1) return -1;

  int64_t A = addr;
  int64_t I = L - A;
  if(is_valid_imm(I, imm_type) == 0){
    return -1;
  }

  instruction = info->opcode + (rd << 7) + (I & 0x000FF000) + (((I & 0x00000800) >> 11) << 20) + (((I & 0x0000007FE) >> 1) << 21) + (((I & 0x00100000) >> 20) << 31);
  write_inst_hex(output, instruction);
  /* === end === */
  return 0;
}
