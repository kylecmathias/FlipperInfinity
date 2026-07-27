#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_bluetooth;
extern const lv_img_dsc_t img_ir;
extern const lv_img_dsc_t img_logo;
extern const lv_img_dsc_t img_nfc;
extern const lv_img_dsc_t img_no_bluetooth;
extern const lv_img_dsc_t img_no_wifi;
extern const lv_img_dsc_t img_rf;
extern const lv_img_dsc_t img_rfid;
extern const lv_img_dsc_t img_settings;
extern const lv_img_dsc_t img_wifi;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[10];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/