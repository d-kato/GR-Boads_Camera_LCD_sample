
#include "mbed.h"
#include "EasyAttach_CameraAndLCD.h"

/**** User Selection *********/
/** JPEG out setting **/
#define JPEG_SEND              (1)                 /* Select  0(JPEG images are not output to PC) or 1(JPEG images are output to PC on USB(CDC) for focusing the camera) */
#define JPEG_ENCODE_QUALITY    (75)                /* JPEG encode quality (min:1, max:75 (Considering the size of JpegBuffer, about 75 is the upper limit.)) */
#define VFIELD_INT_SKIP_CNT    (0)                 /* A guide for GR-LYCHEE.  0:60fps, 1:30fps, 2:20fps, 3:15fps, 4:12fps, 5:10fps */

/** Camera setting **/
#define OV7725_SETTING_TEST    (1)                 /* Exposure and Gain Setting Test 0:disable 1:enable */
/*****************************/

/* Video input and LCD layer 0 output */
#define VIDEO_FORMAT           (DisplayBase::VIDEO_FORMAT_YCBCR422)
#define GRAPHICS_FORMAT        (DisplayBase::GRAPHICS_FORMAT_YCBCR422)
#define WR_RD_WRSWA            (DisplayBase::WR_RD_WRSWA_32_16BIT)
#define DATA_SIZE_PER_PIC      (2u)

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#if MBED_CONF_APP_LCD
  #define VIDEO_PIXEL_HW       LCD_PIXEL_WIDTH   /* QVGA */
  #define VIDEO_PIXEL_VW       LCD_PIXEL_HEIGHT  /* QVGA */
#else
  #define VIDEO_PIXEL_HW       (640u)  /* VGA */
  #define VIDEO_PIXEL_VW       (480u)  /* VGA */
#endif

#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

DisplayBase Display;

#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]@ ".mirrorram";
#else
static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
#endif

#if JPEG_SEND
#include "JPEG_Converter.h"
#include "DisplayApp.h"
#include "dcache-control.h"

#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t JpegBuffer[2][1024 * 64];
#else
static uint8_t JpegBuffer[2][1024 * 64]__attribute((aligned(32)));
#endif
static size_t jcu_encode_size[2];
static JPEG_Converter Jcu;
static int jcu_buf_index_write = 0;
static int jcu_buf_index_write_done = 0;
static int jcu_buf_index_read = 0;
static volatile int jcu_encoding = 0;
static volatile int image_change = 0;
static DisplayApp  display_app;
static int Vfield_Int_Cnt = 0;

static void JcuEncodeCallBackFunc(JPEG_Converter::jpeg_conv_error_t err_code) {
    if (err_code == JPEG_Converter::JPEG_CONV_OK) {
        jcu_buf_index_write_done = jcu_buf_index_write;
        image_change = 1;
    }
    jcu_encoding = 0;
}

static void snapshot(void) {
    while ((jcu_encoding == 1) || (image_change == 0)) {
        Thread::wait(1);
    }
    jcu_buf_index_read = jcu_buf_index_write_done;
    image_change = 0;
    display_app.SendJpeg(JpegBuffer[jcu_buf_index_read], (int)jcu_encode_size[jcu_buf_index_read]);
}

static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type) {
    if (Vfield_Int_Cnt < VFIELD_INT_SKIP_CNT) {
        Vfield_Int_Cnt++;
        return;
    }
    Vfield_Int_Cnt = 0;

    //Interrupt callback function
    if (jcu_encoding == 0) {
        JPEG_Converter::bitmap_buff_info_t bitmap_buff_info;
        JPEG_Converter::encode_options_t   encode_options;

        bitmap_buff_info.width              = VIDEO_PIXEL_HW;
        bitmap_buff_info.height             = VIDEO_PIXEL_VW;
        bitmap_buff_info.format             = JPEG_Converter::WR_RD_YCbCr422;
        bitmap_buff_info.buffer_address     = (void *)user_frame_buffer0;

        encode_options.encode_buff_size     = sizeof(JpegBuffer[0]);
        encode_options.p_EncodeCallBackFunc = &JcuEncodeCallBackFunc;
        encode_options.input_swapsetting    = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;

        jcu_encoding = 1;
        if (jcu_buf_index_read == jcu_buf_index_write) {
            jcu_buf_index_write ^= 1;  // toggle
        }
        jcu_encode_size[jcu_buf_index_write] = 0;
        dcache_invalid(JpegBuffer[jcu_buf_index_write], sizeof(JpegBuffer[0]));
        if (Jcu.encode(&bitmap_buff_info, JpegBuffer[jcu_buf_index_write],
            &jcu_encode_size[jcu_buf_index_write], &encode_options) != JPEG_Converter::JPEG_CONV_OK) {
            jcu_encode_size[jcu_buf_index_write] = 0;
            jcu_encoding = 0;
        }
    }
}

#if OV7725_SETTING_TEST
static bool touch_notification = false;

static void touch_callback(void) {
    touch_notification = true;
}
#endif
#endif

static void Start_Video_Camera(void) {
    // Video capture setting (progressive form fixed)
    Display.Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)user_frame_buffer0,
        FRAME_BUFFER_STRIDE,
        VIDEO_FORMAT,
        WR_RD_WRSWA,
        VIDEO_PIXEL_VW,
        VIDEO_PIXEL_HW
    );
    EasyAttach_CameraStart(Display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

#if MBED_CONF_APP_LCD
static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;

    rect.vs = 0;
    rect.vw = LCD_PIXEL_HEIGHT;
    rect.hs = 0;
    rect.hw = LCD_PIXEL_WIDTH;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)user_frame_buffer0,
        FRAME_BUFFER_STRIDE,
        GRAPHICS_FORMAT,
        WR_RD_WRSWA,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

    Thread::wait(50);
    EasyAttach_LcdBacklight(true);
}
#endif

int main(void) {
    // Initialize the background to black
    for (uint32_t i = 0; i < sizeof(user_frame_buffer0); i += 2) {
        user_frame_buffer0[i + 0] = 0x10;
        user_frame_buffer0[i + 1] = 0x80;
    }

    EasyAttach_Init(Display);
#if JPEG_SEND
    Jcu.SetQuality(JPEG_ENCODE_QUALITY);
    // Interrupt callback function setting (Field end signal for recording function in scaler 0)
    Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);
#if OV7725_SETTING_TEST
    DisplayApp::touch_pos_t s_touch;
    display_app.SetCallback(&touch_callback);
#endif
#endif
    Start_Video_Camera();
#if MBED_CONF_APP_LCD
    Start_LCD_Display();
#endif

    while (1) {
#if JPEG_SEND
        snapshot();
#if OV7725_SETTING_TEST
        // OV7725 Setting test
        if (touch_notification) {
            display_app.GetCoordinates(1, &s_touch);
            if (s_touch.valid) {
                uint16_t usManualExposure = (uint16_t)(0xFFFF * ((float)s_touch.x / 800.f));
                uint8_t  usManualGain = (uint8_t)(0xFF * ((float)s_touch.y / 480.0f));
                OV7725_config::SetExposure(false, usManualExposure, usManualGain);
                printf("usManualExposure 0x%04x, usManualGain 0x%02x\r\n", usManualExposure, usManualGain);
            } else {
                OV7725_config::SetExposure(true, 0x0000, 0x00);
                printf("SetExposure Auto\r\n");
            }
            touch_notification = false;
        }
#endif
#else
        Thread::wait(1000);
#endif
    }
}

