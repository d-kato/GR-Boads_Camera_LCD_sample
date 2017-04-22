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

カメラとLCDの指定を行う場合は``mbed_app.json``に以下を追加してください。
```json
{
    "config": {
        "camera":{
            "help": "0:disable 1:enable",
            "value": "1"
        },
        "camera-type":{
            "help": "Options are CAMERA_CVBS, CAMERA_MT9V111, CAMERA_OV7725",
            "value": "CAMERA_CVBS"
        },
        "lcd":{
            "help": "0:disable 1:enable",
            "value": "1"
        },
        "lcd-type":{
            "help": "Options are GR_PEACH_4_3INCH_SHIELD, GR_PEACH_7_1INCH_SHIELD, GR_PEACH_RSK_TFT, GR_PEACH_DISPLAY_SHIELD, GR_LYCHEE_LCD",
            "value": "GR_PEACH_4_3INCH_SHIELD"
        }
    }
}
```
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
