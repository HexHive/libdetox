/**
 * This file contains all the actions that are used in the bt to
 * translate each instruction.
 * Each action takes an instruction and 'translates' it into the code buffer
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
 *   Antonio Barresi <abarresi@student.ethz.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#ifndef FBT_ACTIONS_H
#define FBT_ACTIONS_H

#include "fbt_datatypes.h"


/* standard actions */
finalize_tu_t   action_none            (translate_struct_t *ts);
finalize_tu_t   action_copy            (translate_struct_t *ts);
finalize_tu_t   action_call            (translate_struct_t *ts);
finalize_tu_t   action_call_indirect   (translate_struct_t *ts);
finalize_tu_t   action_jcc             (translate_struct_t *ts);
finalize_tu_t   action_jmp             (translate_struct_t *ts);
finalize_tu_t   action_jmp_indirect    (translate_struct_t *ts);
finalize_tu_t   action_ret             (translate_struct_t *ts);
finalize_tu_t   action_sysenter        (translate_struct_t *ts);
finalize_tu_t   action_int             (translate_struct_t *ts);
finalize_tu_t   action_warn            (translate_struct_t *ts);
finalize_tu_t   action_fail            (translate_struct_t *ts);

#endif /* FBT_ACTIONS_H */
