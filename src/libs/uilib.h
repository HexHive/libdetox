/**
 * @file uilib.c
 * Interface to user interface.
 *
 * Copyright (c) 2014-2015 ETH Zurich
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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

#ifdef ENABLE_UI_MESSAGES

#define UI_MSG 1
#define UI_NEW_PROCESS 2
#define UI_CLEANUP_PROCESS 3
#define UI_RET_VIOLATION 4
#define UI_JMP_VIOLATION 5
#define UI_CALL_VIOLATION 6

extern int uifd;

void init_ui();

void new_lwp(int pid);

void notify_ui(int type, char* message);

void cleanup_ui();

#endif
