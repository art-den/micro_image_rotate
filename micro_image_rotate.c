#include <math.h>
#include "micro_image_rotate.h"

#define FP_BITS 16
#define FP_BITS_ONE (1 << FP_BITS)
#define FP_BITS_MASK ((1 << FP_BITS)-1)

inline uint32_t interpolate_color_values(uint32_t color1, uint32_t color2, int32_t fp_frac, int colors_count)
{
	uint32_t result = 0;

	if (colors_count != IMG_ROTATE_COLOR_RGB565)
	{
		while (colors_count)
		{
			int32_t v1 = color1 & 0xFF;
			int32_t v2 = color2 & 0xFF;
			int32_t v = (((v2 - v1) * fp_frac) >> FP_BITS) + v1;
			colors_count--;
			color1 >>= 8;
			color2 >>= 8;
			result |= (uint32_t)v << (colors_count << 3);
		}
	}
	else
	{
		int32_t v1 = color1 & 0x1F;
		int32_t v2 = color2 & 0x1F;
		int32_t r = (((v2 - v1) * fp_frac) >> FP_BITS) + v1;
		color1 >>= 5;
		color2 >>= 5;
		v1 = color1 & 0x3F;
		v2 = color2 & 0x3F;
		int32_t g = (((v2 - v1) * fp_frac) >> FP_BITS) + v1;
		color1 >>= 6;
		color2 >>= 6;
		v1 = color1 & 0x1F;
		v2 = color2 & 0x1F;
		int32_t b = (((v2 - v1) * fp_frac) >> FP_BITS) + v1;
		result = b | (g << 5) | (r << 11);
	}

	return result;
}

void show_rotated_image_line(
	get_img_pixel_t get_pixel_fun,
	void*           get_pixel_cookie,
	uint32_t        image_width,
	uint32_t        image_height,
	float           image_center_x,
	float           image_center_y,
	float           image_angle,
	uint32_t        outside_color,
	int             colors_count,
	int             interpolate,
	set_img_pixel_t set_pixel_fun,
	void*           set_pixel_cookie,
	int32_t         dst_width,
	int32_t         dst_height,
	int32_t         dst_y)
{
	float sin_a = sin(-image_angle);
	float cos_a = cos(-image_angle);
	float src_x_f = - dst_width/2 * cos_a - (dst_y - dst_height/2) * sin_a + image_center_x;
	float src_y_f = (dst_y - dst_height/2) * cos_a - dst_width/2 * sin_a + image_center_y;

	int32_t src_x = src_x_f * FP_BITS_ONE;
	int32_t src_y = src_y_f * FP_BITS_ONE;
	const int32_t src_dx = cos_a * FP_BITS_ONE;
	const int32_t src_dy = sin_a * FP_BITS_ONE;
	const int32_t max_image_x = image_width - 1;
	const int32_t max_image_y = image_height - 1;
	const int32_t max_src_x = max_image_x << FP_BITS;
	const int32_t max_src_y = max_image_y << FP_BITS;

	for (int32_t x = 0; x <= dst_width; x++)
	{
		uint32_t dst_color = outside_color;

		if (interpolate == IMG_ROTATE_NO_INTERPOLATE)
		{
			if ((src_x >= 0) && (src_x < max_src_x) && (src_y >= 0) && (src_y < max_src_y))
				dst_color = get_pixel_fun(get_pixel_cookie, src_x >> FP_BITS, src_y >> FP_BITS);
		}
		else if (interpolate == IMG_ROTATE_BILINEAR)
		{
			const int32_t x = src_x >> FP_BITS;
			const int32_t y = src_y >> FP_BITS;

			if ((x >= -1) && (y >= -1) && (x < (int32_t)image_width) && (y < (int32_t)image_height))
			{
				const int hit_h0 = x >= 0;
				const int hit_h1 = x != max_image_x;
				const int hit_v0 = y >= 0;
				const int hit_v1 = y != max_image_y;

				const uint32_t color00 = (hit_h0 && hit_v0) ? get_pixel_fun(get_pixel_cookie, x, y) : outside_color;
				const uint32_t color10 = (hit_h1 && hit_v0) ? get_pixel_fun(get_pixel_cookie, x + 1, y) : outside_color;
				const uint32_t color01 = (hit_h0 && hit_v1) ? get_pixel_fun(get_pixel_cookie, x, y + 1) : outside_color;
				const uint32_t color11 = (hit_h1 && hit_v1) ? get_pixel_fun(get_pixel_cookie, x + 1, y + 1) : outside_color;

				const uint32_t color0 = interpolate_color_values(color00, color10, src_x & FP_BITS_MASK, colors_count);
				const uint32_t color1 = interpolate_color_values(color01, color11, src_x & FP_BITS_MASK, colors_count);

				dst_color = interpolate_color_values(color0, color1, src_y & FP_BITS_MASK, colors_count);
			}
		}
		set_pixel_fun(set_pixel_cookie, x, dst_color);
		src_x += src_dx;
		src_y += src_dy;
	}
}
