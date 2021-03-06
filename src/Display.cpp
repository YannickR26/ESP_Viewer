#include "Display.h"

#include "Logger.h"

#include <lv_examples.h>

#define LVLG_SIZE_OF_BUFFER (LV_HOR_RES_MAX * 12)

static lv_color_t buf1[LVLG_SIZE_OF_BUFFER];
static lv_color_t buf2[LVLG_SIZE_OF_BUFFER];

/********************************************************/
/******************** Public Method *********************/
/********************************************************/

void Display::setup()
{
    lv_init();
    _tft.begin();        /* TFT init */
    _tft.setRotation(1); /* Landscape orientation */

#if LV_USE_LOG != 0
    lv_log_register_print_cb(Disp.print); /* register print function for debugging */
#endif

    // Initialize buffer for screen
    lv_disp_buf_init(&_disp_buf, buf1, buf2, LVLG_SIZE_OF_BUFFER);

    // Initialize the display
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    // disp_drv.hor_res = LV_VER_RES_MAX;
    // disp_drv.ver_res = LV_HOR_RES_MAX;
    disp_drv.flush_cb = flush;
    disp_drv.buffer = &_disp_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize the input device driver
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    lv_demo_widgets();
}

void Display::handle()
{
    lv_task_handler(); /* let the GUI do its work */
}

/********************************************************/
/******************** Static Method *********************/
/********************************************************/

#if LV_USE_LOG != 0
/* Serial debugging */
void Display::print(lv_log_level_t level, const char * file, uint32_t line,  const char * func, const char * dsc)
{
    Serial.printf("%s@%d->%s\r\n", file, line, dsc);
    Serial.flush();
}
#endif

/* Display flushing */
void Display::flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    Disp._tft.startWrite();
    Disp._tft.setAddrWindow(area->x1, area->y1, w, h);
    Disp._tft.pushColors(&color_p->full, w * h, true);
    Disp._tft.endWrite();

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing */
    lv_disp_flush_ready(disp);
}

/* Read the touchpad */
bool Display::touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;

    bool touched = Disp._tft.getTouch(&touchX, &touchY, 600);

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
        return false;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
    }

    if (touchX > LV_HOR_RES_MAX || touchY > LV_VER_RES_MAX)
    {
        Log.println("Y or y outside of expected parameters..");
        Log.print("y:");
        Log.print(touchX);
        Log.print(" x:");
        Log.print(touchY);
    }
    else
    {
        /* Set the coordinates */
        data->point.x = touchX;
        data->point.y = touchY;

        Log.print("Data x");
        Log.println(touchX);

        Log.print("Data y");
        Log.println(touchY);
    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

/********************************************************/
/******************** Private Method ********************/
/********************************************************/

#if !defined(NO_GLOBAL_INSTANCES)
Display Disp;
#endif
