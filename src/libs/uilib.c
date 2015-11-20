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

#include "uilib.h"
#include "../fastbt/fbt_syscalls_impl.h"
#include "../fastbt/fbt_llio.h"
#include "../fastbt/fbt_libc.h"

#define O_WRONLY 0x1
#define FIFO "/tmp/trustedRT.fifo"

int uifd = -1;

void init_ui() {
	uifd = -1;
	fbt_open(FIFO, O_WRONLY, 0, uifd, "Could not open named pipe (init_ui: uilib.c)\n");
	notify_ui(UI_NEW_PROCESS, "New process started.");
}

void new_lwp(int pid) {
	if(uifd > 0 && pid) {
		fllprintf(uifd, "NEW|%d|New process started.\n", pid);
	}
}

void notify_ui(int type, char* message) {
	int pid;
	fbt_getpid(pid, "Could not get process ID (notify_ui: uilib.c)\n");
	
	if(uifd > 0 && pid) {
		switch(type) {
			case UI_MSG:
			fllprintf(uifd, "MSG|%d|%s\n", pid, message);
			break;
			case UI_NEW_PROCESS:
			fllprintf(uifd, "NEW|%d|%s\n", pid, message);
			break;
			case UI_CLEANUP_PROCESS:
			fllprintf(uifd, "CLEANUP|%d|%s\n", pid, message);
			break;
			case UI_RET_VIOLATION:
			fllprintf(uifd, "RETV|%d|%s\n", pid, message);
			break;
			case UI_JMP_VIOLATION:
			fllprintf(uifd, "JMPV|%d|%s\n", pid, message);
			break;
			case UI_CALL_VIOLATION:
			fllprintf(uifd, "CALLV|%d|%s\n", pid, message);
			break;
		}
	}
}

void cleanup_ui() {
	notify_ui(UI_CLEANUP_PROCESS, "Process cleanup.");
	int res;
	fbt_close(uifd, res, "Could not close named pipe (cleanup_ui: uilib.c)\n");
}

#endif
