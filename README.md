# GR-Boads_Camera_LCD_sample
GR-PEACH、および、GR-LYCHEEで動作するサンプルプログラムです。  
GR-LYCHEEの開発環境については、[GR-LYCHEE用オフライン開発環境の手順](https://developer.mbed.org/users/dkato/notebook/offline-development-lychee-langja/)を参照ください。


## 概要
カメラ画像をLCD、または、Windows用PCアプリ**DisplayApp**に表示させるサンプルです。  

### カメラとLCDの設定
``mbed_app.json``ファイルを変更することでLCD表示をONにできます。
```json
{
    "config": {
        "camera":{
            "help": "0:disable 1:enable",
            "value": "1"
        },
        "lcd":{
            "help": "0:disable 1:enable",
            "value": "1"
        }
    }
}
```

カメラとLCDの指定を行う場合は``mbed_app.json``に``camera-type``と``lcd-type``を追加してください。
```json
{
    "config": {
        "camera":{
            "help": "0:disable 1:enable",
            "value": "1"
        },
        "camera-type":{
            "help": "Please see mbed-gr-libs/README.md",
            "value": "CAMERA_CVBS"
        },
        "lcd":{
            "help": "0:disable 1:enable",
            "value": "1"
        },
        "lcd-type":{
            "help": "Please see mbed-gr-libs/README.md",
            "value": "GR_PEACH_4_3INCH_SHIELD"
        }
    }
}
```

| camera-type "value"     | 説明                               |
|:------------------------|:-----------------------------------|
| CAMERA_CVBS             | GR-PEACH NTSC信号                  |
| CAMERA_MT9V111          | GR-PEACH MT9V111                   |
| CAMERA_OV7725           | GR-LYHCEE 付属カメラ               |
| CAMERA_OV5642           | GR-PEACH OV5642                    |

| lcd-type "value"        | 説明                               |
|:------------------------|:-----------------------------------|
| GR_PEACH_4_3INCH_SHIELD | GR-PEACH 4.3インチLCDシールド      |
| GR_PEACH_7_1INCH_SHIELD | GR-PEACH 7.1インチLCDシールド      |
| GR_PEACH_RSK_TFT        | GR-PEACH RSKボード用LCD            |
| GR_PEACH_DISPLAY_SHIELD | GR-PEACH Display Shield            |
| GR_LYCHEE_LCD           | GR-LYHCEE TF043HV001A0など(40pin)  |


camera-typeとlcd-typeを指定しない場合は以下の設定となります。  
* GR-PEACH、カメラ：CAMERA_MT9V111、LCD：GR_PEACH_4_3INCH_SHIELD  
* GR-LYCHEE、カメラ：CAMERA_OV7725、LCD：GR_LYCHEE_LCD  

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

#### カメラの露光・ゲイン設定を変更する (GR-LYCHEEのみ)
カメラの露光とゲインの設定を変更することができます。  
DisplayAppで画像が映っている状態で、画面上をマウスでクリックすると、露光・ゲインのマニュアル設定ができます。X軸が露光、Y軸がゲインに対応しています。マウスボタンを離すと自動調整ONに戻ります。  
ゲイン設定については特殊な計算方法で算出されます。詳しくはデータシートを参照してください。設定された露光・ゲインの値はprintf()にてターミナル上に出力されます。  

機能を無効にするには`main.cpp`の``OV7725_SETTING_TEST``に``0``を設定します。  

```cpp
#define OV7725_SETTING_TEST    (1)                 /* Exposure and Gain Setting Test 0:disable 1:enable */
```
