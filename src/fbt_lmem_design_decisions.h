/**
 * @file fbt_lmem.h
 * Datatypes used for communication with selDebug
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Enrico Kravina <enrico.kravina@gmail.com>
 *
 * $Date: 2012-01-18 12:17:00 +0100 (mer, 18 gen 2012) $
 * $LastChangedDate: 2012-01-18 12:17:00 +0100 (mer, 18 gen 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1184 $
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


#ifndef FBT_DESIGN_DECISIONS_H_
#define FBT_DESIGN_DECISIONS_H_

/**
 * About the 'flags' protocol.
 *
 * Somehow we need a protocol for passing the 'flags' register
 * from the end of one basic block to the beginning of a next one.
 * It is convenient to have one single protocol for the whole
 * translated program since this allows any basic block to jump
 * to any basic block without worrying about protocol incompatibilities.
 * The two main options are
 * 1.) Eflags is saved in tld at the end of each basic block
 *     iff at least one of the possible jump targets reads the flags
 *     before overwriting them.
 * 2.) Eflags is kept in the flags register, (again...)
 *     iff at least one of the possible jump targets reads the flags
 *     before overwriting them.
 *
 * Consider the second option, which imo may be the best one:
 *
 * - if the last instruction is a conditional, then the flags need to
 *   be in eflags anyway (OPTIMAL)
 * - if the last instruction is a call/ret we normally use the heuristic
 *   that assumes that eflags isn't read across function boundaries
 *   (OPTIMAL)
 * - if the last instruction is an indirect jump we accept that we may
 *   do one extra flag manipulation
 *   (MAY BE SUBOPTIMAL) -> but again we assume flags not used -> (OPTIMAL)
 * - if the last instruction is a jmp then one may worry about the
 *   following instruction sequence:
 *
 *    sets flags
 *    mov  // instrumentation destroys flags
 *    mov  // instrumentation destroys flags
 *    mov  // instrumentation destroys flags
 *    jmp ->
 *    -> mov // instrumentation destroys flags
 *    mov // instrumentation destroys flags
 *    reads flags
 *
 *   with the 'saving protocol' we get
 *
 *    sets flags
 *    SAVE FLAGS
 *    mov  // instrumentation destroys flags
 *    mov  // instrumentation destroys flags
 *    mov  // instrumentation destroys flags
 *    jmp ->
 *    ...
 *    -> mov // instrumentation destroys flags
 *    mov // instrumentation destroys flags
 *    RESTORE FLAGS
 *    reads flags
 *
 *   while with the 'keep in eflags protocol we would get
 *
 *    sets flags
 *    SAVE FLAGS
 *    mov  // instrumentation destroys flags
 *    mov  // instrumentation destroys flags
 *    mov  // instrumentation destroys flags
 *    RESTORE FLAGS
 *    jmp ->
 *    ...
 *    SAVE FLAGS
 *    -> mov // instrumentation destroys flags
 *    mov // instrumentation destroys flags
 *    RESTORE FLAGS
 *    reads flags
 *
 *  (SEEMS SUBOPTIMAL -> But we can counter this)
 *
 *	The countermeasure is to just continue to inline
 *	after a 'jmp'.
 *	The obvious downside of this countermeasure is increases
 *	memory consumption. But our design is generous about memory
 *	anyway, striving for good time performance.
 *	The good side-effect about this approach is that the linearized
 *	code paths get longer thereby offering more optimization
 *	opportunities.
 *
 */


#endif /* FBT_DESIGN_DECISIONS_H_ */
