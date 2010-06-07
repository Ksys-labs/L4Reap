#ifndef __L4PNG_WRAP_H__
#define __L4PNG_WRAP_H__

#define ARGB_BUF_TO_SMALL -2
#define ENOPNG -3;
#define EDAMAGEDPNG -4;

/**
 * \brief Get the dimension of an PNG picture.
 * \return 0 on success, negative on error
 */
int png_get_size_mem(void *png_data, int png_data_size, int *width, int *height);
int png_get_size_file(const char *fp, int *width, int *height);

/** CONVERT PNG TO A ARGB-BUFFER (ALPHA, RED, GREEN, BLUE) **/
int png_convert_ARGB_mem(void *png_data, void *argb_buf,
                         int png_data_size, unsigned argb_max_size);
int png_convert_ARGB_file(const char *filename, void *argb_buf,
                          unsigned argb_max_size);
int png_convert_RGB16bit_mem(void *png_data, void *argb_buf,
                             int png_data_size, unsigned argb_max_size,
                             int line_offset);
int png_convert_RGB16bit_file(const char *filename, void *argb_buf,
			      unsigned argb_max_size,
			      int line_offset);

#endif /* ! __L4PNG_WRAP_H__ */
