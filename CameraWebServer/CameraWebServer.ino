#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>

// ======================================================
// انتخاب مدل دوربین از داخل فایل board_config.h
// ======================================================
#include "board_config.h"

// ======================================================
// مشخصات شبکه WiFi
// ======================================================
const char *ssid = "Autorep";
const char *password = "FIRA2026";

// اعلان (Prototype) توابعی که در فایل‌های دیگر تعریف شده‌اند.
void startCameraServer();
void setupLedFlash();

void setup() {

  // راه‌اندازی ارتباط سریال برای نمایش پیام‌های دیباگ
  Serial.begin(115200);

  // فعال کردن خروجی‌های دیباگ ESP32
  Serial.setDebugOutput(true);

  Serial.println();

  //======================================================
  // ساختار تنظیمات دوربین
  //======================================================
  camera_config_t config;

  // انتخاب کانال PWM برای تولید سیگنال کلاک دوربین
  config.ledc_channel = LEDC_CHANNEL_0;

  // انتخاب تایمر PWM
  config.ledc_timer = LEDC_TIMER_0;

  //======================================================
  // اتصال پایه‌های دیتای دوربین
  //======================================================
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  // پایه تولید کلاک خارجی دوربین
  config.pin_xclk = XCLK_GPIO_NUM;

  // پایه کلاک پیکسل
  config.pin_pclk = PCLK_GPIO_NUM;

  // پایه همگام‌سازی عمودی تصویر
  config.pin_vsync = VSYNC_GPIO_NUM;

  // پایه همگام‌سازی افقی تصویر
  config.pin_href = HREF_GPIO_NUM;

  // خطوط ارتباطی SCCB (مشابه I2C)
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  // پایه خاموش/روشن کردن دوربین
  config.pin_pwdn = PWDN_GPIO_NUM;

  // پایه ریست سخت‌افزاری دوربین
  config.pin_reset = RESET_GPIO_NUM;

  //======================================================
  // تنظیمات عملکرد دوربین
  //======================================================

  // فرکانس کلاک دوربین (20MHz)
  config.xclk_freq_hz = 20000000;

  // رزولوشن اولیه تصویر (UXGA = 1600×1200)
  config.frame_size = FRAMESIZE_UXGA;

  // فرمت خروجی تصویر JPEG
  // این حالت مناسب استریم تصاویر در وب است.
  config.pixel_format = PIXFORMAT_JPEG;

  // اگر تشخیص چهره یا پردازش تصویر نیاز باشد می‌توان از RGB565 استفاده کرد.
  //config.pixel_format = PIXFORMAT_RGB565;

  // تصویر جدید فقط زمانی گرفته شود که بافر خالی باشد.
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  // ذخیره بافر تصویر داخل حافظه PSRAM
  config.fb_location = CAMERA_FB_IN_PSRAM;

  // کیفیت JPEG
  // عدد کمتر یعنی کیفیت بیشتر و حجم بالاتر
  config.jpeg_quality = 12;

  // تعداد بافرهای تصویر
  config.fb_count = 1;

  //======================================================
  // اگر PSRAM موجود باشد تنظیمات بهینه‌تر اعمال می‌شود.
  //======================================================
  if (config.pixel_format == PIXFORMAT_JPEG) {

    // بررسی وجود حافظه PSRAM
    if (psramFound()) {

      // افزایش کیفیت JPEG
      config.jpeg_quality = 10;

      // استفاده از دو بافر تصویر
      config.fb_count = 2;

      // همیشه جدیدترین تصویر دریافت شود.
      config.grab_mode = CAMERA_GRAB_LATEST;

    } else {

      // اگر PSRAM وجود نداشته باشد
      // رزولوشن کاهش پیدا می‌کند تا حافظه کافی باشد.
      config.frame_size = FRAMESIZE_SVGA;

      // ذخیره بافر در حافظه داخلی DRAM
      config.fb_location = CAMERA_FB_IN_DRAM;
    }

  } else {

    // بهترین تنظیمات برای پردازش تصویر و تشخیص چهره
    config.frame_size = FRAMESIZE_240X240;

#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif

  }

#if defined(CAMERA_MODEL_ESP_EYE)

  // فعال کردن مقاومت Pull-up برای کلیدهای برد ESP-EYE
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

#endif

  //======================================================
  // راه‌اندازی دوربین
  //======================================================
  esp_err_t err = esp_camera_init(&config);

  // بررسی موفق بودن راه‌اندازی
  if (err != ESP_OK) {

    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // دریافت اشاره‌گر به سنسور دوربین
  sensor_t *s = esp_camera_sensor_get();

  //======================================================
  // تنظیمات اختصاصی سنسور OV3660
  //======================================================
  if (s->id.PID == OV3660_PID) {

    // برگرداندن تصویر به حالت صحیح
    s->set_vflip(s, 1);

    // کمی افزایش روشنایی
    s->set_brightness(s, 1);

    // کاهش اشباع رنگ
    s->set_saturation(s, -2);
  }

  //======================================================
  // برای افزایش سرعت ارسال تصویر،
  // رزولوشن اولیه روی QVGA قرار داده می‌شود.
  //======================================================
  if (config.pixel_format == PIXFORMAT_JPEG) {

    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)

  // اصلاح جهت تصویر برای برخی بردها
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);

#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)

  // اصلاح جهت تصویر
  s->set_vflip(s, 1);

#endif

//======================================================
// اگر پایه LED تعریف شده باشد، فلش دوربین راه‌اندازی می‌شود.
//======================================================
#if defined(LED_GPIO_NUM)
  setupLedFlash();
#endif

  //======================================================
  // اتصال به شبکه WiFi
  //======================================================
  WiFi.begin(ssid, password);

  // جلوگیری از Sleep شدن ماژول WiFi
  // برای داشتن استریم پایدارتر
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");

  // منتظر اتصال به شبکه
  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  //======================================================
  // راه‌اندازی وب‌سرور دوربین
  //======================================================
  startCameraServer();

  //======================================================
  // نمایش آدرس IP برای مشاهده تصویر در مرورگر
  //======================================================
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {

  //======================================================
  // در این برنامه حلقه اصلی کاری انجام نمی‌دهد.
  // تمام پردازش‌ها داخل تسک وب‌سرور اجرا می‌شوند.
  //======================================================
  delay(10000);
}