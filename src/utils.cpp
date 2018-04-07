/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <stdio.h>
#include <string.h>

// I want %f, but sprintf on ESP doesn't have that capability
char *printff(char *buf, float f)
{
    char *dot_pos;
    if (f < 1.0 || f > 999.0)
    {
        // out of range
        *buf = 0;
        return buf;
    }
    sprintf(buf, "%d", int(f * 100 + 0.005));
    dot_pos = buf + strlen(buf);
    *(dot_pos+1) = 0;
    *dot_pos     = *(dot_pos-1);
    *(dot_pos-1) = *(dot_pos-2);
    *(dot_pos-2) = '.';
    return buf; // not strictly necessary, as caller has buf already, but a programming convenience.
}

static const char hexdig[] = "0123456789ABCDEF";

char *formatAddr(char *buf, unsigned char  addr[8])
{
    int i;
    char *p;
    for (i=0, p=buf; i < 8; ++i)
    {
        *p++ = hexdig[(addr[i] >> 4) & 0xf];
        *p++ = hexdig[addr[i] & 0xf];
    }
    *p = 0;
    return buf; // as for printff
}
