/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <stdio.h>
char *printff(char *buf, float f)
{
    char mybuf[5];
    int int_part;
    int float_part;
    int_part = (int)f;
    float_part = (int)( f - int_part ) * 100 ;
    sprintf(mybuf, "%d", float_part + 1000);
    if (mybuf[3] == '0')
    {
        // truncate unnecessary trailing zeroes
        if (mybuf[2] == '0')
            mybuf[2] = 0;
        else
            mybuf[3] = 0;
    }
    sprintf(buf, "%d.%s", int_part, mybuf + 1);
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
