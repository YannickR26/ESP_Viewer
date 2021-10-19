#if defined(ESP8266)
#include <LittleFS.h>
#define c_FS LittleFS
#elif defined(ESP32)
#include <SPIFFS.h>
#define c_FS SPIFFS
#endif

#include "Display.h"

#include "Logger.h"

#include <lv_examples.h>


#define LVLG_SIZE_OF_BUFFER (LV_HOR_RES_MAX * 10)

static lv_color_t buf1[LVLG_SIZE_OF_BUFFER];
// static lv_color_t buf2[LVLG_SIZE_OF_BUFFER];

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL true

/********************************************************/
/******************** Public Method *********************/
/********************************************************/

void Display::setup()
{
  lv_init();
  _tft.begin();        /* TFT init */
  _tft.setRotation(1); /* Landscape orientation */
  _tft.fillScreen(TFT_BLACK);

  calibrate();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(Disp.print); /* register print function for debugging */
#endif

  // Initialize buffer for screen
  lv_disp_buf_init(&_disp_buf, buf1, NULL, LVLG_SIZE_OF_BUFFER);

  // Initialize the display
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
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

void Display::calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!c_FS.begin()) {
    Serial.println("Formating file system");
    c_FS.format();
    c_FS.begin();
  }

  // check if calibration file exists and size is correct
  if (c_FS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      c_FS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = c_FS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    _tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    _tft.fillScreen(TFT_BLACK);
    _tft.setCursor(20, 0);
    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);

    _tft.println("Touch corners as indicated");

    _tft.setTextFont(1);
    _tft.println();

    if (REPEAT_CAL) {
      _tft.setTextColor(TFT_RED, TFT_BLACK);
      _tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    _tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    _tft.setTextColor(TFT_GREEN, TFT_BLACK);
    _tft.println("Calibration complete!");

    Log.println("Calibration complete :");
    for (int i=0; i<5 ; i++)
      Log.print(", 0x" + String(calData[i], HEX));
    Log.println();

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

/********************************************************/
/******************** Static Method *********************/
/********************************************************/

#if LV_USE_LOG != 0
/* Serial debugging */
void Display::print(lv_log_level_t level, const char *file, uint32_t line, const char *func, const char *dsc)
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

  bool touched = Disp._tft.getTouch(&touchX, &touchY, 300);

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
    Log.println("x or y outside of expected parameters..");
    Log.print("y:");
    Log.print(touchX);
    Log.print(" x:");
    Log.println(touchY);
  }
  else
  {
    /* Set the coordinates */
    data->point.x = touchX;
    data->point.y = touchY;

    // Log.print("TouchScreen x: ");
    // Log.print(touchX);

    // Log.print(", y: ");
    // Log.println(touchY);
  }

  return false; /*Return `false` because we are not buffering and no more data to read*/
}

/********************************************************/
/******************** Private Method ********************/
/********************************************************/

#if !defined(NO_GLOBAL_INSTANCES)
Display Disp;
#endif
