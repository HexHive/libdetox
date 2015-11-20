/**
 * @file fbt_patching_filesystem.c
 * Patching thread implementation monitoring the filesystem for a patch
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 1134 $
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


#if defined(ONLINE_PATCHING_FILE) && defined(ONLINE_PATCHING)

#include <sys/inotify.h>

#include "../fbt_datatypes.h"
#include "../fbt_debug.h"
#include "../fbt_libc.h"
#include "../fbt_llio.h"
#include "../fbt_syscalls_impl.h"

#include "fbt_patching.h"
#include "fbt_patching_util.h"

#include <errno.h>

int fbt_online_patching_monitor_filesystem(void *shared_data) {
    DEBUG_START();

    PRINT_DEBUG_FUNCTION_START("fbt_online_patching_monitor_filesystem\n");

    struct thread_local_data *tld = ((struct shared_data *)shared_data)->patching_tld;


    char patch_path[BUFFER_SIZE];
    
    int len = fbt_get_patch_path(patch_path, 1024);    
    if (len < 0) {
        PRINT_DEBUG("Failed to get patch path.\n");
        return 0;
    }
    
    #define EVENT_SIZE (sizeof (struct inotify_event))
    
    #define BUF_LEN (1024 * (EVENT_SIZE + 16))

    char buf[BUF_LEN];
    
    /* Split up path patch path into directory and file name */
    char* patch_name = patch_path + len;
    while (patch_name != patch_path) {
        if (*patch_name == '/') {
            *patch_name = '\0';
            patch_name++;
            break;
        }
        patch_name--;
    }
    PRINT_DEBUG("patch_path = %s, patch_name = %s\n", patch_path, patch_name);
    
    PRINT_DEBUG("starting inotify\n");

    int fd = inotify_init();
    if (fd < 0) {
        PRINT_DEBUG("inotify_init failed.\n");
        return 1;
    }

    
    int wd;
    wd = inotify_add_watch(fd, patch_path, IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO);
    if (wd < 0) {
        PRINT_DEBUG("inotify_add_watch failed\n");
        return 1;
    }    


    while (1) {   
        int len, i = 0;
        fbt_read(fd, buf, BUF_LEN, len);
        
        if (len <= 0) {
            PRINT_DEBUG("read failed");
            return 0;
        }

        while (i < len) {
            struct inotify_event *event;
            event = (struct inotify_event *)&buf[i];
            
            if (event->mask & IN_DELETE_SELF) {
                PRINT_DEBUG("Self deleted.\n");
            }            
            
            if (event->len && fbt_strncmp(event->name, patch_name, BUFFER_SIZE) == 0) {
                /* We should now be able to read the patch file */
                if (event->mask & IN_CLOSE_WRITE) {
                    fbt_apply_local_patch(tld, 1);
                }
                
                if (event->mask & IN_MOVED_TO) {
                    fbt_apply_local_patch(tld, 1);
                }
            }
            i += EVENT_SIZE + event->len;
        }  
    }
    
    PRINT_DEBUG_FUNCTION_END("");
    
    DEBUG_END();

    return 0;
}
#endif