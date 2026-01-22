#ifndef pointer_include_file
#define pointer_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define pointer_width 7
#define pointer_height 12
#define pointer_size 86
#define pointer ((gfx_sprite_t*)pointer_data)
extern unsigned char pointer_data[86];

#ifdef __cplusplus
}
#endif

#endif
