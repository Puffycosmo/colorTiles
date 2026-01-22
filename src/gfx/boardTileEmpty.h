#ifndef boardTileEmpty_include_file
#define boardTileEmpty_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define boardTileEmpty_width 4
#define boardTileEmpty_height 4
#define boardTileEmpty_size 18
#define boardTileEmpty ((gfx_sprite_t*)boardTileEmpty_data)
extern unsigned char boardTileEmpty_data[18];

#ifdef __cplusplus
}
#endif

#endif
