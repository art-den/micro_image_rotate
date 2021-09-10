#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IMG_ROTATE_COLOR_RGB888    3
#define IMG_ROTATE_COLOR_ARGB8888  4
#define IMG_ROTATE_COLOR_GRAYSCALE 1
#define IMG_ROTATE_COLOR_RGB565   -1

#define IMG_ROTATE_NO_INTERPOLATE 0
#define IMG_ROTATE_BILINEAR       1

typedef uint32_t (*get_img_pixel_t)(void *cookie, uint32_t x, uint32_t y);
typedef void (*set_img_pixel_t)(void *cookie, uint32_t x, uint32_t rgb);

void show_rotated_image_line(
	get_img_pixel_t get_pixel_fun,
	void*           get_pixel_cookie,
	uint32_t        image_width,
	uint32_t        image_height,
	float           image_center_x,
	float           image_center_y,
	float           image_angle,
	uint32_t        outside_color,
	int             colors_count, // see IMG_ROTATE_COLOR_*
	int             interpolate, // see IMG_ROTATE_*
	set_img_pixel_t set_pixel_fun,
	void*           set_pixel_cookie,
	int32_t         dst_width,
	int32_t         dst_height,
	int32_t         dst_y
);

#ifdef __cplusplus
}
#endif
