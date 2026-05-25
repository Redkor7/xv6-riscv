#include "kernel/types.h"
#include "user/user.h"

int is_leap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void print_date(uint64 unsec) {
    long nsec = (long)unsec; 
    
    long sec = nsec / 1000000000;
    int frac = nsec % 1000000000;

    if (frac < 0) {
        frac += 1000000000;
        sec -= 1;
    }

    int year = 1970;
    long days = sec / 86400;
    int rem_sec = sec % 86400;

    if (rem_sec < 0) {
        rem_sec += 86400;
        days -= 1;
    }

    if (days >= 0) {
        while (days >= (is_leap(year) ? 366 : 365)) {
            days -= is_leap(year) ? 366 : 365;
            year++;
        }
    } else {
        while (days < 0) {
            year--;
            days += is_leap(year) ? 366 : 365; 
        }
    }

    int month = 0;
    int dim;
    while (days >= (dim = (month == 1 && is_leap(year)) ? 29 : days_in_month[month])) {
        days -= dim;
        month++;
    }

    int day = days + 1;
    int hour = rem_sec / 3600;
    int min = (rem_sec % 3600) / 60;
    int s = rem_sec % 60;

    printf("%d-%s%d-%s%d %s%d:%s%d:%s%d.",
           year,
           (month + 1 < 10) ? "0" : "", month + 1,
           (day < 10) ? "0" : "", day,
           (hour < 10) ? "0" : "", hour,
           (min < 10) ? "0" : "", min,
           (s < 10) ? "0" : "", s);
    
    int divisor = 100000000;
    while(divisor > 0) {
        printf("%d", (frac / divisor) % 10);
        divisor /= 10;
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    uint64 time_ns;
    
    if (rtc_time(&time_ns) < 0) {
        fprintf(2, "date: failed to get rtc time\n");
        exit(1);
    }
    
    print_date(time_ns);
    exit(0);
}