/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>

#include "libfreevec.h"

#ifdef HAVE_ALTIVEC_H
unsigned char *strfill(unsigned char *s, int len,char fill)
{
    memset(s, fill, len);
    *(s+len-1) = '\0';
    return(s);
}
#endif
