/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#include "mmsgui/fb/mmsfbconv.h"
#include <string.h>

void stretch_byte_buffer_no_antialiasing(unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
										 unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh) {
	// please note that the src and dst have to point to the first pixel which is to process
	int horifact = (dw<<16)/sw;
	int vertfact = (dh<<16)/sh;
	int vertcnt;
	unsigned char *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned char *dst_end = dst + dst_pitch_pix * dst_height;

	// no antialiasing
	if (horifact == 0x10000) {
		// no horizontal stretch needed, so use optimized loop
		vertcnt = 0x8000;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			vertcnt+=vertfact;
			if (vertcnt & 0xffff0000) {
				do {
					memcpy(dst, src, sw);
					vertcnt-=0x10000;
					dst = dst + dst_pitch;
				} while (vertcnt & 0xffff0000);
			}
			// next line
			src+=src_pitch;
		}
	}
	if (horifact == 0x20000) {
		// duplicate each horizontal pixel
		vertcnt = 0x8000;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			vertcnt+=vertfact;
			if (vertcnt & 0xffff0000) {
				unsigned char *line_end = src + sw;
				unsigned char *old_dst = dst;

				do {
					//int horicnt = 0x8000;
					while (src < line_end) {
						register unsigned short int SRC  = *src;
						*((unsigned short int *)dst) = SRC | (SRC << 8);
						dst+=2;
						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst +  dst_pitch;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=src_pitch;
		}
	}
	else {
		// normal stretch in both directions
		vertcnt = 0x8000;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			vertcnt+=vertfact;
			if (vertcnt & 0xffff0000) {
				unsigned char *line_end = src + sw;
				unsigned char *old_dst = dst;

				do {
					int horicnt = 0x8000;
					while (src < line_end) {
						horicnt+=horifact;
						if (horicnt & 0xffff0000) {
							register unsigned char SRC  = *src;

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst +  dst_pitch;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=src_pitch;
		}
	}
}

void stretch_byte_buffer_v_antialiasing(unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
									    unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh) {
	// please note that the src and dst have to point to the first pixel which is to process
	int horifact = (dw<<16)/sw;
	int vertfact = (dh<<16)/sh;
	int vertcnt;
	unsigned char *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned char *dst_end = dst + dst_pitch_pix * dst_height;

	// vertical antialiasing
	if (vertfact >= 0x10000) {
		// positive vertical stretch (scale up)
		if (horifact == 0x10000) {
			// no horizontal stretch needed, so use optimized loop
			vertcnt = 0x8000;
			register unsigned char vcnt = 0;
			while ((src < src_end)&&(dst < dst_end)) {
				// for all pixels in the line
				vertcnt+=vertfact;
				if (vertcnt & 0xffff0000) {
					unsigned char *line_end = src + sw;

					bool vaa = (vcnt > 1);
					vcnt = 0;
					if (!vaa) {
						// line without vertical antialiasing
						do {
							memcpy(dst, src, sw);
							vertcnt-=0x10000;
							dst = dst + dst_pitch;
							vcnt++;
						} while (vertcnt & 0xffff0000);
					}
					else {
						unsigned char *old_dst = dst;
						do {
							if (vaa) {
								// first line with vertical antialiasing
								register unsigned int SRC;
								while (src < line_end) {
									// load pixel
									SRC = *src;

									// put first pixel
									*dst = SRC;
									*(dst-dst_pitch) = (*(dst-dst_pitch) + SRC) >> 1;
									dst++;

									src++;
								}
								src-=sw;
							}
							else {
								// next line without vertical antialiasing
								memcpy(dst, src, sw);
							}
							vertcnt-=0x10000;
							dst = old_dst + dst_pitch;
							old_dst = dst;
							vcnt++;
							vaa = false;
						} while (vertcnt & 0xffff0000);
					}
				}

				// next line
				src+=src_pitch;
			}
		}
		else {
			// normal stretch in both directions
			vertcnt = 0x8000;
			register unsigned char vcnt = 0;
			while ((src < src_end)&&(dst < dst_end)) {
				// for all pixels in the line
				vertcnt+=vertfact;
				if (vertcnt & 0xffff0000) {
					unsigned char *line_end = src + sw;
					unsigned char *old_dst = dst;

					bool vaa = (vcnt > 1);
					vcnt = 0;
					if (!vaa) {
						do {
							int horicnt = 0x8000;
							register unsigned int SRC;
							while (src < line_end) {
								horicnt+=horifact;
								if (horicnt & 0xffff0000) {
									// load pixel
									SRC = *src;

									// put first pixel
									*dst = SRC;
									dst++;
									horicnt-=0x10000;

									// have to put further?
									if (horicnt & 0xffff0000) {
										do {
											*dst = SRC;
											dst++;
											horicnt-=0x10000;
										} while (horicnt & 0xffff0000);
									}
								}

								src++;
							}
							src-=sw;
							vertcnt-=0x10000;
							dst = old_dst + dst_pitch;
							old_dst = dst;
							vcnt++;
						} while (vertcnt & 0xffff0000);
					}
					else {
						do {
							int horicnt = 0x8000;
							register unsigned int SRC;
							if (vaa) {
								// first line with vertical antialiasing
								while (src < line_end) {
									horicnt+=horifact;
									if (horicnt & 0xffff0000) {
										// load pixel
										SRC = *src;

										// put first pixel
										*dst = SRC;
										*(dst-dst_pitch) = (*(dst-dst_pitch) + SRC) >> 1;
										dst++;
										horicnt-=0x10000;

										// have to put further?
										if (horicnt & 0xffff0000) {
											do {
												*dst = SRC;
												*(dst-dst_pitch) = (*(dst-dst_pitch) + SRC) >> 1;
												dst++;
												horicnt-=0x10000;
											} while (horicnt & 0xffff0000);
										}
									}

									src++;
								}
							}
							else {
								// next line without vertical antialiasing
								while (src < line_end) {
									horicnt+=horifact;
									if (horicnt & 0xffff0000) {
										// load pixel
										SRC = *src;

										// put first pixel
										*dst = SRC;
										dst++;
										horicnt-=0x10000;

										// have to put further?
										if (horicnt & 0xffff0000) {
											do {
												*dst = SRC;
												dst++;
												horicnt-=0x10000;
											} while (horicnt & 0xffff0000);
										}
									}

									src++;
								}
							}
							src-=sw;
							vertcnt-=0x10000;
							dst = old_dst + dst_pitch;
							old_dst = dst;
							vcnt++;
							vaa = false;
						} while (vertcnt & 0xffff0000);
					}
				}

				// next line
				src+=src_pitch;
			}
		}
	}
	else {
		// negative vertical stretch (scale down)
		if (horifact == 0x10000) {
			// no horizontal stretch needed, so use optimized loop
			vertcnt = 0x8000;
			bool vaa = false;
			while ((src < src_end)&&(dst < dst_end)) {
				// for all pixels in the line
				vertcnt+=vertfact;
				if (vertcnt & 0xffff0000) {
					unsigned char *line_end = src + sw;

					if (!vaa) {
						// line without vertical antialiasing
						memcpy(dst, src, sw);
						vertcnt-=0x10000;
						dst = dst + dst_pitch;
					}
					else {
						// line with vertical antialiasing
						unsigned char *old_dst = dst;
						while (src < line_end) {
							// put pixel with arithmetic mean
							*dst = (*(src-src_pitch) + *src) >> 1;
							dst++;
							src++;
						}
						src-=sw;
						vertcnt-=0x10000;
						dst = old_dst + dst_pitch;
						vaa = false;
					}
				}
				else
					vaa = true;

				// next line
				src+=src_pitch;
			}
		}
		else {
			// normal stretch in both directions
			vertcnt = 0x8000;
			bool vaa = false;
			while ((src < src_end)&&(dst < dst_end)) {
				// for all pixels in the line
				vertcnt+=vertfact;
				if (vertcnt & 0xffff0000) {
					unsigned char *line_end = src + sw;
					unsigned char *old_dst = dst;

					if (!vaa) {
						// line without vertical antialiasing
						int horicnt = 0x8000;
						register unsigned int SRC;
						while (src < line_end) {
							horicnt+=horifact;
							if (horicnt & 0xffff0000) {
								// load pixel
								SRC = *src;

								// put first pixel
								*dst = SRC;
								dst++;
								horicnt-=0x10000;

								// have to put further?
								if (horicnt & 0xffff0000) {
									do {
										*dst = SRC;
										dst++;
										horicnt-=0x10000;
									} while (horicnt & 0xffff0000);
								}
							}

							src++;
						}
						src-=sw;
						vertcnt-=0x10000;
						dst = old_dst + dst_pitch;
						old_dst = dst;
					}
					else {
						// line with vertical antialiasing
						int horicnt = 0x8000;
						register unsigned int SRC;
						while (src < line_end) {
							horicnt+=horifact;
							if (horicnt & 0xffff0000) {
								// load pixel (arithmetic mean)
								SRC = (*(src-src_pitch) + *src) >> 1;

								// put first pixel
								*dst = SRC;
								dst++;
								horicnt-=0x10000;

								// have to put further?
								if (horicnt & 0xffff0000) {
									do {
										*dst = SRC;
										dst++;
										horicnt-=0x10000;
									} while (horicnt & 0xffff0000);
								}
							}

							src++;
						}
						src-=sw;
						vertcnt-=0x10000;
						dst = old_dst + dst_pitch;
						old_dst = dst;
						vaa = false;
					}
				}
				else
					vaa = true;

				// next line
				src+=src_pitch;
			}
		}
	}
}

void stretch_byte_buffer_h_antialiasing(unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
									    unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh) {
	// please note that the src and dst have to point to the first pixel which is to process
	int horifact = (dw<<16)/sw;
	int vertfact = (dh<<16)/sh;
	int vertcnt;
	unsigned char *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned char *dst_end = dst + dst_pitch_pix * dst_height;

	// horizontal antialiasing
	// normal stretch in both directions
	vertcnt = 0x8000;
	while ((src < src_end)&&(dst < dst_end)) {
		// for all pixels in the line
		vertcnt+=vertfact;
		if (vertcnt & 0xffff0000) {
			unsigned char *line_end = src + sw;
			unsigned char *old_dst = dst;

			do {
				int horicnt = 0x8000;
				register unsigned int SRC;
				register bool haa = false;
				while (src < line_end) {
					horicnt+=horifact;
					if (horicnt & 0xffff0000) {
						// check for antialiasing
						if (haa) {
							*(dst-1) = (SRC + *src) >> 1;
							haa = false;
						}

						// load pixel
						SRC = *src;

						// put first pixel
						*dst = SRC;
						dst++;
						horicnt-=0x10000;

						// have to put further?
						if ((haa=(horicnt & 0xffff0000))) {
							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}
					}

					src++;
				}
				src-=sw;
				vertcnt-=0x10000;
				dst = old_dst + dst_pitch;
				old_dst = dst;
			} while (vertcnt & 0xffff0000);
		}

		// next line
		src+=src_pitch;
	}
}

void stretch_byte_buffer_hv_antialiasing(unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
										 unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh) {
	// please note that the src and dst have to point to the first pixel which is to process
	int horifact = (dw<<16)/sw;
	int vertfact = (dh<<16)/sh;
	int vertcnt;
	unsigned char *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned char *dst_end = dst + dst_pitch_pix * dst_height;

	// horizontal and vertical antialiasing
	// normal stretch in both directions
	vertcnt = 0x8000;
	register unsigned char vcnt = 0;
	while ((src < src_end)&&(dst < dst_end)) {
		// for all pixels in the line
		vertcnt+=vertfact;
		if (vertcnt & 0xffff0000) {
			unsigned char *line_end = src + sw;
			unsigned char *old_dst = dst;

			bool vaa = (vcnt > 1);
			vcnt = 0;
			if (!vaa) {
				do {
					int horicnt = 0x8000;
					register unsigned int SRC;
					register bool haa = false;
					while (src < line_end) {
						horicnt+=horifact;
						if (horicnt & 0xffff0000) {
							// check for antialiasing
							if (haa) {
								*(dst-1) = (SRC + *src) >> 1;
								haa = false;
							}

							// load pixel
							SRC = *src;

							// put first pixel
							*dst = SRC;
							dst++;
							horicnt-=0x10000;

							// have to put further?
							if ((haa=(horicnt & 0xffff0000))) {
								do {
									*dst = SRC;
									dst++;
									horicnt-=0x10000;
								} while (horicnt & 0xffff0000);
							}
						}

						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch;
					old_dst = dst;
					vcnt++;
				} while (vertcnt & 0xffff0000);
			}
			else {
				do {
					int horicnt = 0x8000;
					register unsigned int SRC;
					if (vaa) {
						// first line with vertical antialiasing
						register bool haa = false;
						while (src < line_end) {
							horicnt+=horifact;
							if (horicnt & 0xffff0000) {
								// check for antialiasing
								if (haa) {
									*(dst-1) = (SRC + *src) >> 1;
									haa = false;
								}

								// load pixel
								SRC = *src;

								// put first pixel
								*dst = SRC;
								*(dst-dst_pitch) = (*(dst-dst_pitch) + SRC) >> 1;
								dst++;
								horicnt-=0x10000;

								// have to put further?
								if ((haa=(horicnt & 0xffff0000))) {
									do {
										*dst = SRC;
										*(dst-dst_pitch) = (*(dst-dst_pitch) + SRC) >> 1;
										dst++;
										horicnt-=0x10000;
									} while (horicnt & 0xffff0000);
								}
							}

							src++;
						}

						vaa = false;
					}
					else {
						// line without vertical antialiasing
						register bool haa = false;
						while (src < line_end) {
							horicnt+=horifact;
							if (horicnt & 0xffff0000) {
								// check for antialiasing
								if (haa) {
									*(dst-1) = (SRC + *src) >> 1;
									haa = false;
								}

								// load pixel
								SRC = *src;

								// put first pixel
								*dst = SRC;
								dst++;
								horicnt-=0x10000;

								// have to put further?
								if ((haa=(horicnt & 0xffff0000))) {
									do {
										*dst = SRC;
										dst++;
										horicnt-=0x10000;
									} while (horicnt & 0xffff0000);
								}
							}

							src++;
						}
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch;
					old_dst = dst;
					vcnt++;
				} while (vertcnt & 0xffff0000);
			}
		}

		// next line
		src+=src_pitch;
	}
}

void stretch_byte_buffer(bool h_antialiasing, bool v_antialiasing,
						 unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
					     unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh) {
	// please note that the src and dst have to point to the first pixel which is to process

	if (!h_antialiasing) {
		if (!v_antialiasing) {
			// no antialiasing
			stretch_byte_buffer_no_antialiasing(src, src_pitch, src_pitch_pix, src_height, sw, sh,
												dst, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
		}
		else {
			// vertical antialiasing
			stretch_byte_buffer_v_antialiasing(src, src_pitch, src_pitch_pix, src_height, sw, sh,
											   dst, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
		}
	}
	else {
		if (!v_antialiasing) {
			// horizontal antialiasing
			stretch_byte_buffer_h_antialiasing(src, src_pitch, src_pitch_pix, src_height, sw, sh,
											   dst, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
		}
		else {
			// horizontal and vertical antialiasing
			stretch_byte_buffer_hv_antialiasing(src, src_pitch, src_pitch_pix, src_height, sw, sh,
												dst, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
		}
	}
}


void compress_2x2_matrix(unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
						 unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh) {
	// please note that the src and dst have to point to the first pixel which is to process
	unsigned char *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned char *dst_end = dst + dst_pitch_pix * dst_height;

	// prepare (have to work with even width/height)
	sw &= 0xfffffffe;
	sh &= 0xfffffffe;
	int src_pitch_diff = (src_pitch_pix << 1) - sw;
	int dst_pitch_diff = dst_pitch_pix - (sw >> 1);

	// calculate the arithmetic mean
	while ((src < src_end)&&(dst < dst_end)) {
		// go through two lines in parallel (square 2x2 pixel)
		unsigned char *line_end = src + sw;
		while (src < line_end) {
			int d = (int)*src + (int)src[1] + (int)src[src_pitch_pix] + (int)src[src_pitch_pix+1];
			*dst = d >> 2;
			src+=2;
			dst++;
		}

		// go to the next two lines
		src += src_pitch_diff;
		dst += dst_pitch_diff;
	}
}

void stretch_uint_buffer(bool h_antialiasing, bool v_antialiasing,
						 unsigned int *src, int src_pitch, int src_pitch_pix,
						 int src_height, int sx, int sy, int sw, int sh,
					     unsigned int *dst, int dst_pitch, int dst_pitch_pix,
					     int dst_height, int dx, int dy, int dw, int dh) {

	// point to the first pixel which is to process
	src = src + sx + sy * src_pitch_pix;
	dst = dst + dx + dy * dst_pitch_pix;

	// calc the end pointers
	unsigned int *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned int *dst_end = dst + dst_pitch_pix * dst_height;

	// setup defaults
	int start_vertcnt = 0x8000;
	int start_horicnt = 0x8000;
	bool vbreak = false;
	bool hbreak = false;
	int vertfact = (dh<<16)/sh;
	int horifact = (dw<<16)/sw;

	if (vertfact <= 0) {
		// have to calculate accurate factor based on surface dimensions
		vertfact = (dst_height<<16)/src_height;

		// have to calculate accurate start vertcnt
		for (int i=0, j=0; i<sy; i++) {
			start_vertcnt+=vertfact;
			if (start_vertcnt & 0xffff0000) {
				do {
					j++;
					if (j > dy) {
						vbreak = true;
						break;
					}
					start_vertcnt-=0x10000;
				} while (start_vertcnt & 0xffff0000);
			}
			if (vbreak) break;
		}
	}

	if (horifact <= 0) {
		// have to calculate accurate factor based on surface dimensions
		horifact = (dst_pitch_pix<<16)/src_pitch_pix;

		// have to calculate accurate start horicnt
		for (int i=0, j=0; i<sx; i++) {
			start_horicnt+=horifact;
			if (start_horicnt & 0xffff0000) {
				do {
					j++;
					if (j > dx) {
						hbreak = true;
						break;
					}
					start_horicnt-=0x10000;
				} while (start_horicnt & 0xffff0000);
			}
			if (hbreak) break;
		}
	}

	if ((!vbreak)&&(!hbreak)) {
		// optimized loop
		int vertcnt = start_vertcnt;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			vertcnt+=vertfact;
			if (vertcnt & 0xffff0000) {
				unsigned int *line_end = src + sw;
				unsigned int *old_dst = dst;

				do {
					int horicnt = start_horicnt;
					while (src < line_end) {
						horicnt+=horifact;
						if (horicnt & 0xffff0000) {
							register unsigned int SRC  = *src;

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch_pix;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=src_pitch_pix;
		}
	}
	else {
		// consider vbreak and hbreak values
		int vertcnt = start_vertcnt;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			if (vbreak) {
				vbreak = false;
				src-= src_pitch_pix;
			}
			else
				vertcnt+=vertfact;

			if (vertcnt & 0xffff0000) {
				unsigned int *line_end = src + sw;
				unsigned int *old_dst = dst;

				do {
					int horicnt = start_horicnt;
					bool hb = hbreak;
					while (src < line_end) {
						if (hb) {
							hb = false;
							src--;
						}
						else
							horicnt+=horifact;

						if (horicnt & 0xffff0000) {
							register unsigned int SRC  = *src;

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch_pix;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=src_pitch_pix;
		}
	}
}

void stretch_usint_buffer(bool h_antialiasing, bool v_antialiasing,
						  unsigned short int *src, int src_pitch, int src_pitch_pix,
						  int src_height, int sx, int sy, int sw, int sh,
					      unsigned short int *dst, int dst_pitch, int dst_pitch_pix,
					      int dst_height, int dx, int dy, int dw, int dh) {

	// point to the first pixel which is to process
	src = src + sx + sy * src_pitch_pix;
	dst = dst + dx + dy * dst_pitch_pix;

	// calc the end pointers
	unsigned short int *src_end = src + src_pitch_pix * sh;
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned short int *dst_end = dst + dst_pitch_pix * dst_height;

	// setup defaults
	int start_vertcnt = 0x8000;
	int start_horicnt = 0x8000;
	bool vbreak = false;
	bool hbreak = false;
	int vertfact = (dh<<16)/sh;
	int horifact = (dw<<16)/sw;

	if (vertfact <= 0) {
		// have to calculate accurate factor based on surface dimensions
		vertfact = (dst_height<<16)/src_height;

		// have to calculate accurate start vertcnt
		for (int i=0, j=0; i<sy; i++) {
			start_vertcnt+=vertfact;
			if (start_vertcnt & 0xffff0000) {
				do {
					j++;
					if (j > dy) {
						vbreak = true;
						break;
					}
					start_vertcnt-=0x10000;
				} while (start_vertcnt & 0xffff0000);
			}
			if (vbreak) break;
		}
	}

	if (horifact <= 0) {
		// have to calculate accurate factor based on surface dimensions
		horifact = (dst_pitch_pix<<16)/src_pitch_pix;

		// have to calculate accurate start horicnt
		for (int i=0, j=0; i<sx; i++) {
			start_horicnt+=horifact;
			if (start_horicnt & 0xffff0000) {
				do {
					j++;
					if (j > dx) {
						hbreak = true;
						break;
					}
					start_horicnt-=0x10000;
				} while (start_horicnt & 0xffff0000);
			}
			if (hbreak) break;
		}
	}

	if ((!vbreak)&&(!hbreak)) {
		// optimized loop
		int vertcnt = start_vertcnt;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			vertcnt+=vertfact;
			if (vertcnt & 0xffff0000) {
				unsigned short int *line_end = src + sw;
				unsigned short int *old_dst = dst;

				do {
					int horicnt = start_horicnt;
					while (src < line_end) {
						horicnt+=horifact;
						if (horicnt & 0xffff0000) {
							register unsigned short int SRC  = *src;

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch_pix;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=src_pitch_pix;
		}
	}
	else {
		// consider vbreak and hbreak values
		int vertcnt = start_vertcnt;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			if (vbreak) {
				vbreak = false;
				src-= src_pitch_pix;
			}
			else
				vertcnt+=vertfact;

			if (vertcnt & 0xffff0000) {
				unsigned short int *line_end = src + sw;
				unsigned short int *old_dst = dst;

				do {
					int horicnt = start_horicnt;
					bool hb = hbreak;
					while (src < line_end) {
						if (hb) {
							hb = false;
							src--;
						}
						else
							horicnt+=horifact;

						if (horicnt & 0xffff0000) {
							register unsigned short int SRC  = *src;

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src++;
					}
					src-=sw;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch_pix;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=src_pitch_pix;
		}
	}
}



void stretch_324byte_buffer(bool h_antialiasing, bool v_antialiasing,
							unsigned char *src, int src_pitch, int src_pitch_pix,
							int src_height, int sx, int sy, int sw, int sh,
							unsigned int *dst, int dst_pitch, int dst_pitch_pix,
							int dst_height, int dx, int dy, int dw, int dh) {

	// point to the first pixel which is to process
	src = src + (sx + sy * src_pitch_pix) * 3;
	dst = dst + dx + dy * dst_pitch_pix;

	// calc the end pointers
	unsigned char *src_end = src + (src_pitch_pix * sh) * 3;
	if (src_end > src + (src_pitch_pix * src_height) * 3)
		src_end = src + (src_pitch_pix * src_height) * 3;
	unsigned int *dst_end = dst + dst_pitch_pix * dst_height;

	// setup defaults
	int start_vertcnt = 0x8000;
	int start_horicnt = 0x8000;
	bool vbreak = false;
	bool hbreak = false;
	int vertfact = (dh<<16)/sh;
	int horifact = (dw<<16)/sw;
	int sww = sw * 3;
	int spp = src_pitch_pix * 3;

	if (vertfact <= 0) {
		// have to calculate accurate factor based on surface dimensions
		vertfact = (dst_height<<16)/src_height;

		// have to calculate accurate start vertcnt
		for (int i=0, j=0; i<sy; i++) {
			start_vertcnt+=vertfact;
			if (start_vertcnt & 0xffff0000) {
				do {
					j++;
					if (j > dy) {
						vbreak = true;
						break;
					}
					start_vertcnt-=0x10000;
				} while (start_vertcnt & 0xffff0000);
			}
			if (vbreak) break;
		}
	}

	if (horifact <= 0) {
		// have to calculate accurate factor based on surface dimensions
		horifact = (dst_pitch_pix<<16)/src_pitch_pix;

		// have to calculate accurate start horicnt
		for (int i=0, j=0; i<sx; i++) {
			start_horicnt+=horifact;
			if (start_horicnt & 0xffff0000) {
				do {
					j++;
					if (j > dx) {
						hbreak = true;
						break;
					}
					start_horicnt-=0x10000;
				} while (start_horicnt & 0xffff0000);
			}
			if (hbreak) break;
		}
	}

	if ((!vbreak)&&(!hbreak)) {
		// optimized loop
		int vertcnt = start_vertcnt;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			vertcnt+=vertfact;
			if (vertcnt & 0xffff0000) {
				unsigned char *line_end = src + sww;
				unsigned int *old_dst = dst;

				do {
					int horicnt = start_horicnt;
					while (src < line_end) {
						horicnt+=horifact;
						if (horicnt & 0xffff0000) {
							register unsigned int SRC  = 0xff000000 | ((*src)<<16) | ((*(src+1))<<8) | *(src+2);

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src+=3;
					}
					src-=sww;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch_pix;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=spp;
		}
	}
	else {
		// consider vbreak and hbreak values
		int vertcnt = start_vertcnt;
		while ((src < src_end)&&(dst < dst_end)) {
			// for all pixels in the line
			if (vbreak) {
				vbreak = false;
				src-= spp;
			}
			else
				vertcnt+=vertfact;

			if (vertcnt & 0xffff0000) {
				unsigned char *line_end = src + sww;
				unsigned int *old_dst = dst;

				do {
					int horicnt = start_horicnt;
					bool hb = hbreak;
					while (src < line_end) {
						if (hb) {
							hb = false;
							src-=3;
						}
						else
							horicnt+=horifact;

						if (horicnt & 0xffff0000) {
							register unsigned int SRC  = 0xff000000 | ((*src)<<16) | ((*(src+1))<<8) | *(src+2);

							do {
								*dst = SRC;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}

						src+=3;
					}
					src-=sww;
					vertcnt-=0x10000;
					dst = old_dst + dst_pitch_pix;
					old_dst = dst;
				} while (vertcnt & 0xffff0000);
			}

			// next line
			src+=spp;
		}
	}
}


void mmsfb_blit_uint(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
					 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy) {

	// get the first source ptr/pitch
	unsigned int *src = (unsigned int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned int *dst = (unsigned int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int src_pitch_pix = src_pitch >> 2;
	int dst_pitch_pix = dst_pitch >> 2;
	src+= sx + sy * src_pitch_pix;
	dst+= dx + dy * dst_pitch_pix;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	unsigned int *src_end = src + src_pitch_pix * sh;

	// for all lines
	while (src < src_end) {
		// copy the line
		memcpy(dst, src, sw << 2);

		// go to the next line
		src+= src_pitch_pix;
		dst+= dst_pitch_pix;
	}
}

void mmsfb_blit_usint(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
					  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy) {

	// get the first source ptr/pitch
	unsigned short int *src = (unsigned short int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned short int *dst = (unsigned short int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int src_pitch_pix = src_pitch >> 1;
	int dst_pitch_pix = dst_pitch >> 1;
	src+= sx + sy * src_pitch_pix;
	dst+= dx + dy * dst_pitch_pix;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	unsigned short int *src_end = src + src_pitch_pix * sh;

	// for all lines
	while (src < src_end) {
		// copy the line
		memcpy(dst, src, sw << 1);

		// go to the next line
		src+= src_pitch_pix;
		dst+= dst_pitch_pix;
	}
}

