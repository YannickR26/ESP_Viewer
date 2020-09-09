#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>

class Display
{
public:
    Display() {}
    virtual ~Display() {}

    void setup();
    void handle();

    static void print(lv_log_level_t level, const char * file, uint32_t line, const char * func, const char * dsc);
    static void flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static bool touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

private:
    TFT_eSPI _tft;
    lv_disp_buf_t _disp_buf;
};

#if !defined(NO_GLOBAL_INSTANCES)
extern Display Disp;
#endif