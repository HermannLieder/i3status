// vim:ts=4:sw=4:expandtab
#include <config.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>

#include "i3status.h"

#define STRING_SIZE 50

static bool local_timezone_init = false;
static const char *local_timezone = NULL;
static const char *current_timezone = NULL;

int get_wd(void)
{
    int d, m, y;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    d = tm.tm_mday;
    m = tm.tm_mon + 1;
    y = tm.tm_year + 1900;

    return (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
}

char* k_substitution(const char * string_time)
{
    char* token;
    char kanji[] = "月";
    

    switch(get_wd())    
    {                   
        case 0:
        strcpy(kanji, "日");
        break;

        case 1:
        strcpy(kanji, "月");
        break;

        case 2:
        strcpy(kanji, "火");
        break;

        case 3:
        strcpy(kanji, "水");
        break;

        case 4:
        strcpy(kanji, "木");
        break;

        case 5:
        strcpy(kanji, "金");
        break;

        case 6:
        strcpy(kanji, "土");
        break;
    }
    
    char string_out[STRING_SIZE] = "";
    token = strtok(string_time, "K");
    
    while(token != NULL)
    {
        //printf("loop entered\n");
        strcat(string_out, token);
        token = strtok(NULL, "K");
        if(token != NULL)
            strcat(string_out, kanji);
    }
    
    return sstrdup(string_out);
}

void set_timezone(const char *tz) {
    if (!local_timezone_init) {
        /* First call, initialize. */
        local_timezone = getenv("TZ");
        local_timezone_init = true;
    }
    if (tz == NULL || tz[0] == '\0') {
        /* User wants localtime. */
        tz = local_timezone;
    }
    if (tz != current_timezone) {
        if (tz) {
            setenv("TZ", tz, 1);
        } else {
            unsetenv("TZ");
        }
        current_timezone = tz;
    }
    tzset();
}

void print_time(yajl_gen json_gen, 
                char *buffer, 
                const char *title, 
                const char *format, 
                const char *tz, 
                const char *locale, 
                const char *format_time, 
                bool hide_if_equals_localtime, 
                time_t t) 
{
    char *outwalk = buffer;
    struct tm local_tm, tm;

    if (title != NULL)
        INSTANCE(title);

    set_timezone(NULL);
    localtime_r(&t, &local_tm);

    set_timezone(tz);
    localtime_r(&t, &tm);

    // When hide_if_equals_localtime is true, compare local and target time to display only if different
    time_t local_t = mktime(&local_tm);
    double diff = difftime(local_t, t);
    if (hide_if_equals_localtime && diff == 0.0) {
        goto out;
    }

    if (locale != NULL) {
        setlocale(LC_ALL, locale);
    }

    char string_time[STRING_SIZE];

    if (format_time == NULL) {
        outwalk += strftime(buffer, 4096, format, &tm);
    } else {
        strftime(string_time, sizeof(string_time), format_time, &tm);
        placeholder_t placeholders[] = {
            {.name = "%time", .value = string_time}};

        const size_t num = sizeof(placeholders) / sizeof(placeholder_t);
        buffer = format_placeholders(format_time, &placeholders[0], num);
    }

    if (locale != NULL) {
        setlocale(LC_ALL, "");
    }

out:
    *outwalk = '\0';
    buffer = k_substitution(buffer);
    OUTPUT_FULL_TEXT(buffer);
    free(buffer);
}
