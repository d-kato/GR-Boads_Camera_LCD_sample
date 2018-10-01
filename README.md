# GR-Boads_Camera_LCD_sample
GR-PEACH、および、GR-LYCHEEで動作するサンプルプログラムです。  
GR-LYCHEEの開発環境については、[GR-LYCHEE用オフライン開発環境の手順](https://developer.mbed.org/users/dkato/notebook/offline-development-lychee-langja/)を参照ください。


## 概要
カメラ画像をLCD、または、Windows用PCアプリ**DisplayApp**に表示させるサンプルです。  

### カメラとLCDの設定
``mbed_app.json``ファイルを変更することでLCD表示をONにできます。  
詳細は ``mbed-gr-libs/EasyAttach_CameraAndLCD/README.md`` を参照ください。  


***mbed CLI以外の環境で使用する場合***  
mbed CLI以外の環境をお使いの場合、``mbed_app.json``の変更は反映されません。  
``mbed_config.h``に以下のようにマクロを追加してください。  
```cpp
#define MBED_CONF_APP_CAMERA                        1    // set by application
#define MBED_CONF_APP_CAMERA_TYPE                   CAMERA_CVBS             // set by application
#define MBED_CONF_APP_LCD                           0    // set by application
#define MBED_CONF_APP_LCD_TYPE                      GR_PEACH_4_3INCH_SHIELD // set by application
```


### Windows用PCアプリで表示する
``main.cpp``の``JPEG_SEND``に``1``を設定すると、カメラ画像をPCアプリに表示する機能が有効になります。  
カメラ画像はJPEGに変換され、USBファンクションのCDCクラス通信でPCに送信します。  
```cpp
/**** User Selection *********/
/** JPEG out setting **/
#define JPEG_SEND              (1)                 /* Select  0(JPEG images are not output to PC) or 1(JPEG images are output to PC on USB(CDC) for focusing the camera) */
/*****************************/
```
PC用アプリは以下よりダウンロードできます。  
[DisplayApp](https://developer.mbed.org/users/dkato/code/DisplayApp/)  


#### PCへ送信するデータサイズやフレームレートを変更する
``main.cpp``の下記マクロを変更することで
``JPEG_ENCODE_QUALITY``はJPEGエンコード時の品質(画質)を設定します。
API``SetQuality()``の上限は**100**ですが、JPEG変換結果を格納するメモリのサイズなどを考慮すると,上限は**75**程度としてください。  
``VFIELD_INT_SKIP_CNT``はカメラからの入力画像を何回読み捨てるかを設定します。
読み捨てる回数が多いほどPCへ転送するデータのフレームレートが下がります。
GR-LYCHEEの場合、「0:60fps, 1:30fps, 2:20fps, 3:15fps, 4:12fps, 5:10fps」となります。

```cpp
/**** User Selection *********/
/** JPEG out setting **/
#define JPEG_SEND              (1)                 /* Select  0(JPEG images are not output to PC) or 1(JPEG images are output to PC on USB(CDC) for focusing the camera) */
#define JPEG_ENCODE_QUALITY    (75)                /* JPEG encode quality (min:1, max:75 (Considering the size of JpegBuffer, about 75 is the upper limit.)) */
#define VFIELD_INT_SKIP_CNT    (0)                 /* A guide for GR-LYCHEE.  0:60fps, 1:30fps, 2:20fps, 3:15fps, 4:12fps, 5:10fps */
/*****************************/
```

また、以下を変更することで画像の画素数を変更できます。画素数が小さくなると転送データは少なくなります。

```cpp
#define VIDEO_PIXEL_HW       (640u)  /* VGA */
#define VIDEO_PIXEL_VW       (480u)  /* VGA */
```
