#ifndef RTC_H
#define RTC_H

#include <zephyr/drivers/rtc.h>

typedef struct alarm_ctx {
    uint16_t alarm_id;
    uint16_t alarm_mask;
    struct rtc_time alarm_time;
    rtc_alarm_callback callback;
    void *user_data;
} alarm_ctx_t;

int init_rtc();
int set_rtc_date_time(struct rtc_time *tm);
int get_rtc_date_time(struct rtc_time *tm);
void print_rtc_date_time(void);
int set_rtc_alarm(alarm_ctx_t alarm_ctx);

#endif // RTC_H
