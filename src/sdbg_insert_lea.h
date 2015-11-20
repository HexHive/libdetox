
#define LEAL 0x8D
#define NOP_OPCODE 0x90

#define BOOL int
#define TRUE 1
#define FALSE 0

static int sdbg_insert_leal(struct translate* ts, unsigned char **transl_instr_ptr){
  unsigned char *cur_instr = ts->cur_instr + ts->num_prefixes;

  if (hasModRMOp(ts->cur_instr_info->destFlags) ||
      hasModRMOp(ts->cur_instr_info->srcFlags)) {
    unsigned char modRM = *ts->first_byte_after_opcode;
    cur_instr = ts->first_byte_after_opcode + 1;
    if (MODRM_MOD(modRM) == 0x3) { /* operand is register */
    	  PRINT_DEBUG("operand is register! This is probably a string instruction! Todo!");
        return 1;
    } else {
      *(*transl_instr_ptr)++ = LEAL;
      *(*transl_instr_ptr)++ = modRM & 0xC7; /* REG = 000b -> EAX */

      BOOL has_sib = MODRM_RM(modRM) == 0x4;
      int sib = 0;
      if (has_sib) {  // copy the sib byte
        sib = *cur_instr++;
        *(*transl_instr_ptr)++ = sib;
      }

      switch (MODRM_MOD(modRM)) {
        case 1:
          /* disp8 */
          *(*transl_instr_ptr)++ = *cur_instr;
          break;
        case 0:

          // displacement (See modrm table in intel doc)
          if (MODRM_RM(modRM) == 5){
              *((int32_t*)(*transl_instr_ptr)) = *((int32_t*)cur_instr);
              (*transl_instr_ptr) += 4;
              break;
          }

          // (See sib table in intel doc)
          if (has_sib){
            if (SIB_BASE(sib) == 5){
                *((int32_t*)(*transl_instr_ptr)) = *((int32_t*)cur_instr);
                (*transl_instr_ptr) += 4;
                break;
            }
          }

          break;

        case 2:
          /* disp32 */
          *((int32_t*)(*transl_instr_ptr)) = *((int32_t*)cur_instr);
          (*transl_instr_ptr) += 4;
          break;
      }
    }
  } else {
    switch (*cur_instr) {
      case 0xA1: /* movl moffs32, %eax */
      case 0xA3: /* movl %eax, moffs32*/
        cur_instr++;
        *(*transl_instr_ptr)++ = LEAL;
        *(*transl_instr_ptr)++ = 0x5;
        *((int32_t*)(*transl_instr_ptr)) = *((int32_t*)cur_instr);
        (*transl_instr_ptr) += 4;
        break;
      default:
#if defined(DEBUG)
        llprintf("Instruction [%x %s] without ModR/M byte "
                 "is not supported yet\n",
                 *(ts->cur_instr + ts->num_prefixes),
                 ts->cur_instr_info->mnemonic);
#endif
        return 1;  // don't translate anything
    }
  }

  if (ts->num_prefixes == 1) {
    switch (*cur_instr) {
      case PREFIX_LOCK:
      case PREFIX_OP_SZ_OVR:
        /* ignore lock and operand size prefix because
         * the (effective) address is loaded not accessed */
        break;
      case PREFIX_ES_SEG_OVR:
      case PREFIX_CS_SEG_OVR:
      case PREFIX_SS_SEG_OVR:
      case PREFIX_DS_SEG_OVR:
      case PREFIX_FS_SEG_OVR:
      case PREFIX_GS_SEG_OVR:
      case PREFIX_REPNEZ:
      case PREFIX_MISC:
      case PREFIX_ADDR_SZ_OVR:
#if defined(DEBUG)
        llprintf("Prefix %x of instruction [%x %s] is not supported yet\n",
                 *ts->cur_instr,
                 *(ts->cur_instr + ts->num_prefixes),
                 ts->cur_instr_info->mnemonic);
#endif
        return 1; // don't translate anything
    }
  }

  return 0;
}
