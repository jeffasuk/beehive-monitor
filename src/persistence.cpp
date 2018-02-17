/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include "globals.h"


/* The following macro caters for all the various int types in persistent data.
    Although using a macro doesn't reduce code size, it does make modifications/bug fixes easier.
*/
#define SET_NEW_SIMPLE_VALUE(VAR_TYPE) \
{ \
    VAR_TYPE newval; \
    VAR_TYPE *current_value = (VAR_TYPE*)p_settable->value; \
    newval = atoi(value_str); \
    if (*current_value != newval) \
    { \
        changed = 1; \
        *current_value = newval; \
    } \
}


void strdupWithFree(const char *src, char **dst_p)
{
    if (*dst_p)
    {
        free(*dst_p);
    }
    if (src > 0 && *src)
    {
        *dst_p = strdup(src);
    }
    else
    {
        *dst_p = 0;
    }
}

uint8_t setPersistentValue(const char *name, const char *value_str)
{
    uint8_t changed = 0;
    PERSISTENT_INFO *p_settable;
    PERSISTENT_STRING_INFO *pers_str_ptr;
    for (p_settable = persistents; p_settable->name; ++p_settable)
    {
        if (!strcmp(name, p_settable->name))
        {
#ifndef QUIET
            Serial.print("set ");
            Serial.print(name);
            Serial.print("=");
            Serial.println(value_str);
#endif
            switch (p_settable->type)
            {
              case PERS_INT8:
                SET_NEW_SIMPLE_VALUE(int8_t);
                return changed;
              case PERS_UINT8:
                SET_NEW_SIMPLE_VALUE(uint8_t);
                return changed;
              case PERS_INT16:
                SET_NEW_SIMPLE_VALUE(int16_t);
                return changed;
              case PERS_UINT16:
                SET_NEW_SIMPLE_VALUE(uint16_t);
                return changed;
              case PERS_INT32:
                SET_NEW_SIMPLE_VALUE(int32_t);
                return changed;
              case PERS_UINT32:
                SET_NEW_SIMPLE_VALUE(uint32_t);
                return changed;
              case PERS_FLOAT:
                SET_NEW_SIMPLE_VALUE(float);
                return changed;
            }
            return 0;   // found it, but it wasn't a supported type (unlikely!)
        }
    }
    // try string values
    for (pers_str_ptr = persistent_strings; pers_str_ptr->name; ++pers_str_ptr)
    {
        if (!strcmp(name, pers_str_ptr->name))
        {
#ifndef QUIET
            Serial.print("set ");
            Serial.print(name);
            Serial.print("='");
            Serial.print(value_str);
            Serial.println("'");
#endif
            if (value_str && *value_str)
            {
                // we have a new value
                if (*(pers_str_ptr->value) && **(pers_str_ptr->value))
                {
                    // both have values, so compare them
                    if (strcmp(value_str, *(pers_str_ptr->value)))
                    {
                        strdupWithFree(value_str, pers_str_ptr->value);
                        return 1;
                    }
                    // they were the same
                    return 0;
                }
                // didn't have an old value
                strdupWithFree(value_str, pers_str_ptr->value);
                return 1;
            }
            // got a null or empty value
            if (*(pers_str_ptr->value) && **(pers_str_ptr->value))
            {
                // blank out old value
                free(*(pers_str_ptr->value));
                *(pers_str_ptr->value) = 0;
                return 1;
            }
        }
    }
    return changed; // Almost certainly 0 if we got here.
}
