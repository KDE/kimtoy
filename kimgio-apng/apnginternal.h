// This file is generated as follows
// cat apng.cpp | grep png_ | sed -ne '/^.*png_[^(]*(.*$/p' | sed -e 's/^.*\(png_[^(]*\)(.*$/\1/g' | sort -u > tmp
// sed -i '/png_jmpbuf/d' tmp
// cat tmp | sed -e 's/^\(.*\)$/#define \1 __kimtoy__\1/g' > apnginternal.h
#define png_create_info_struct __kimtoy__png_create_info_struct
#define png_create_read_struct __kimtoy__png_create_read_struct
#define png_destroy_read_struct __kimtoy__png_destroy_read_struct
#define png_error __kimtoy__png_error
#define png_get_channels __kimtoy__png_get_channels
#define png_get_gAMA __kimtoy__png_get_gAMA
#define png_get_IHDR __kimtoy__png_get_IHDR
#define png_get_image_height __kimtoy__png_get_image_height
#define png_get_image_width __kimtoy__png_get_image_width
#define png_get_io_ptr __kimtoy__png_get_io_ptr
#define png_get_next_frame_delay_den __kimtoy__png_get_next_frame_delay_den
#define png_get_next_frame_delay_num __kimtoy__png_get_next_frame_delay_num
#define png_get_num_frames __kimtoy__png_get_num_frames
#define png_get_num_plays __kimtoy__png_get_num_plays
#define png_get_PLTE __kimtoy__png_get_PLTE
#define png_get_text __kimtoy__png_get_text
#define png_get_tRNS __kimtoy__png_get_tRNS
#define png_get_valid __kimtoy__png_get_valid
#define png_get_x_pixels_per_meter __kimtoy__png_get_x_pixels_per_meter
#define png_get_y_pixels_per_meter __kimtoy__png_get_y_pixels_per_meter
#define png_ptr __kimtoy__png_ptr
#define png_read_end __kimtoy__png_read_end
#define png_read_frame_head __kimtoy__png_read_frame_head
#define png_read_image __kimtoy__png_read_image
#define png_read_info __kimtoy__png_read_info
#define png_read_update_info __kimtoy__png_read_update_info
#define png_set_bgr __kimtoy__png_set_bgr
#define png_set_error_fn __kimtoy__png_set_error_fn
#define png_set_expand __kimtoy__png_set_expand
#define png_set_filler __kimtoy__png_set_filler
#define png_set_gamma __kimtoy__png_set_gamma
#define png_set_gray_to_rgb __kimtoy__png_set_gray_to_rgb
#define png_set_interlace_handling __kimtoy__png_set_interlace_handling
#define png_set_invert_mono __kimtoy__png_set_invert_mono
#define png_set_packing __kimtoy__png_set_packing
#define png_set_read_fn __kimtoy__png_set_read_fn
#define png_set_strip_16 __kimtoy__png_set_strip_16
#define png_set_swap_alpha __kimtoy__png_set_swap_alpha
