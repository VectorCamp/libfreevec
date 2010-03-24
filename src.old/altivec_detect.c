/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This routine is used in libfreevec under permission from original     *
 *   author David Woodhouse, as found in the URLs:                         *
 *   http://david.woodhou.se/ffmpeg-altivecdetect.patch                    *
 *   and                                                                   *
 *   http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2008-March/043886.html *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/auxvec.h>
#include <asm/cputable.h>

int detect_altivec() {
    static int available = -1;
    int new_avail = 0;
    char fname[64];
    unsigned long buf[64];
    ssize_t count;
    pid_t pid;
    int fd, i;
        
    if (available != -1)
	    return available;

    pid = getpid();
    snprintf(fname, sizeof(fname)-1, "/proc/%d/auxv", pid);

    fd = open(fname, O_RDONLY);
    if (fd < 0)
	    goto out;
 more:
    count = read(fd, buf, sizeof(buf));
    if (count < 0)
	    goto out_close;

    for (i=0; i < (count / sizeof(unsigned long)); i += 2) {
	    if (buf[i] == AT_HWCAP) {
	    new_avail = !!(buf[i+1] & PPC_FEATURE_HAS_ALTIVEC);
		    goto out_close;
	    } else if (buf[i] == AT_NULL) {
		    goto out_close;
	    }
    }

    if (count == sizeof(buf))
	    goto more;
 out_close:
    close(fd);
 out:
    available = new_avail;
    return available;
}

