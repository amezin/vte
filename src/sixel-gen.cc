/*
 * Copyright © 2020 Hans Petter Jansson <hpj@cl.no>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if 0
#include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <glib.h>

/* The image data is stored as a series of palette indexes, with 16 bits
 * per pixel and TRANSPARENT_SLOT indicating transparency. This allows for
 * palette sizes up to 65535 colors.
 *
 * TRANSPARENT_SLOT can be any u16 value. Typically, the first or last
 * slot (0, n_colors) is used. The transparency index is never emitted;
 * instead pixels with this value are left blank in the output. */

#define N_COLORS_MAX 65536
#define TRANSPARENT_SLOT ((N_COLORS_MAX) - 1)

#define WIDTH_MAX 65536
#define HEIGHT_MAX 65536

#define N_PIXELS_IN_SIXEL 6

#define PRE_SEQ "\x1bP"
#define POST_SEQ "\x1b\\"

typedef struct
{
        int width, height;
        int n_colors;
        uint32_t palette [N_COLORS_MAX];
        uint16_t *pixels;
}
Image;

static int
round_up_to_multiple (int n, int m)
{
        n += m - 1;
        return n - (n % m);
}

static void
memset_u16 (uint16_t *buf, uint16_t val, int n)
{
        int i;

        for (i = 0; i < n; i++) {
                buf [i] = val;
        }
}

static uint16_t
pen_to_slot (int i)
{
        if (i >= TRANSPARENT_SLOT)
                return i + 1;

        return i;
}

static uint8_t
interp_u8 (uint8_t a, uint8_t b, int fraction, int total)
{
        uint32_t ta, tb;

        assert (fraction >= 0 && fraction <= total);

        /* Only one color in palette */
        if (total == 0)
                return a;

        ta = (uint32_t) a * (total - fraction) / total;
        tb = (uint32_t) b * fraction / total;

        return ta + tb;
}

static uint32_t
interp_colors (uint32_t a, uint32_t b, int fraction, int total)
{
        return interp_u8 (a, b, fraction, total)
                | (interp_u8 (a >> 8, b >> 8, fraction, total) << 8)
                | (interp_u8 (a >> 16, b >> 16, fraction, total) << 16)
                | (interp_u8 (a >> 24, b >> 24, fraction, total) << 24);
}

static void
image_init (Image *image, int width, int height, int n_colors)
{
        int alloc_height;
        int n_pixels;

        assert (width > 0 && width <= WIDTH_MAX);
        assert (height > 0 && height <= HEIGHT_MAX);
        assert (n_colors > 0 && n_colors < N_COLORS_MAX);

        image->width = width;
        image->height = height;
        image->n_colors = n_colors;

        alloc_height = round_up_to_multiple (height, N_PIXELS_IN_SIXEL);

        n_pixels = width * alloc_height;
        image->pixels = (uint16_t *) malloc (n_pixels * sizeof (uint16_t));
        memset_u16 (image->pixels, TRANSPARENT_SLOT, n_pixels);
}

static void
image_deinit (Image *image)
{
        free (image->pixels);
        image->pixels = NULL;
}

static void
image_generate_palette (Image *image, uint32_t first_color, uint32_t last_color)
{
        int pen;

        for (pen = 0; pen < image->n_colors; pen++) {
                image->palette [pen_to_slot (pen)]
                        = interp_colors (first_color, last_color, pen, image->n_colors - 1);
        }
}

static void
image_set_pixel (Image *image, int x, int y, uint16_t value)
{
        image->pixels [y * image->width + x] = value;
}

static uint16_t
image_get_pixel (const Image *image, int x, int y)
{
        return image->pixels [y * image->width + x];
}

static uint8_t
image_get_sixel (const Image *image, int x, int y, uint16_t value)
{
        uint8_t sixel = 0;
        int i;

        for (i = 0; i < N_PIXELS_IN_SIXEL; i++) {
                uint16_t p = image_get_pixel (image, x, y + N_PIXELS_IN_SIXEL - 1 - i);

                sixel <<= 1;
                if (p == value)
                        sixel |= 1;
        }

        return sixel;
}

static void
image_draw_shape (Image *image)
{
        int y, x;

        for (y = 0; y < image->height; y++) {
                int pen = ((image->n_colors - 1) * y + image->height / 2) / image->height;

                for (x = 0; x < image->width; x++) {
                        if (x == 0 || x == image->width - 1          /* Box left/right */
                            || y == 0 || y == image->height - 1      /* Box top/bottom */
                            || y == x || y == image->width - 1 - x)  /* X diagonals */
                                image_set_pixel (image, x, y, pen_to_slot (pen));
                }
        }
}

static void
image_generate (Image *image, uint32_t first_color, uint32_t last_color)
{
        image_generate_palette (image, first_color, last_color);
        image_draw_shape (image);
}

static void
image_print_sixels_palette (const Image *image, GString *gstr)
{
        int pen;

        for (pen = 0; pen < image->n_colors; pen++) {
                uint32_t col = image->palette [pen_to_slot (pen)];

                g_string_append_printf (gstr, "#%d;2;%d;%d;%d",
                                        pen_to_slot (pen),
                                        (col >> 16) & 0xff,
                                        (col >> 8) & 0xff,
                                        col & 0xff);
        }
}

static void
emit_sixels (GString *gstr, uint8_t sixel, int n, uint16_t slot,
             bool pass_ended, uint16_t *emitted_slot_inout,
             bool *need_emit_cr_inout, bool *need_emit_cr_inout_next)
{
        if (n == 0) {
                return;
        }

        if (!pass_ended || sixel != 0) {
                char c = '?' + (char) sixel;

                if (*need_emit_cr_inout) {
                        g_string_append_c (gstr, '$');
                        *need_emit_cr_inout = FALSE;
                }

                if (slot != *emitted_slot_inout) {
                        g_string_append_printf (gstr, "#%d", slot);
                        *emitted_slot_inout = slot;
                }

                while (n > 255) {
                        g_string_append_printf (gstr, "!255%c", c);
                        n -= 255;
                }

                if (n >= 4) {
                        g_string_append_printf (gstr, "!%d%c", n, c);
                        n = 0;
                } else {
                        for ( ; n > 0; n--)
                                g_string_append_c (gstr, c);
                }
        }

        if (sixel != 0)
                *need_emit_cr_inout_next = TRUE;
}

static void
image_print_sixels_row (const Image *image, GString *gstr, int y, uint16_t *emitted_slot_inout)
{
        bool need_emit_cr = FALSE;
        bool need_emit_cr_next = FALSE;
        int pen;

        for (pen = 0; pen < image->n_colors; pen++) {
                uint16_t slot = pen_to_slot (pen);
                uint8_t cur_sixel = 0;
                int n_cur_sixel = 0;
                int x;

                for (x = 0; x < image->width; x++) {
                        uint8_t next_sixel = image_get_sixel (image, x, y, slot);

                        if (next_sixel == cur_sixel) {
                                n_cur_sixel++;
                                continue;
                        }

                        emit_sixels (gstr, cur_sixel, n_cur_sixel, slot, FALSE,
                                     emitted_slot_inout, &need_emit_cr, &need_emit_cr_next);
                        cur_sixel = next_sixel;
                        n_cur_sixel = 1;
                }

                emit_sixels (gstr, cur_sixel, n_cur_sixel, slot, TRUE,
                             emitted_slot_inout, &need_emit_cr, &need_emit_cr_next);
                need_emit_cr = need_emit_cr_next;
        }

        /* CR + Linefeed */
        g_string_append_c (gstr, '-');
}

static void
image_print_sixels_data (const Image *image, GString *gstr)
{
        uint16_t emitted_slot = TRANSPARENT_SLOT;
        int y;

        for (y = 0; y < image->height; y += N_PIXELS_IN_SIXEL) {
                image_print_sixels_row (image, gstr, y, &emitted_slot);
        }
}

static void
image_print_sixels (const Image *image, GString *gstr)
{
        g_string_append_printf (gstr, PRE_SEQ "0;0;0q\"1;1;%d;%d",
                                image->width, image->height);
        image_print_sixels_palette (image, gstr);
        image_print_sixels_data (image, gstr);
        g_string_append (gstr, POST_SEQ);
}

static void
print_loop (int n_iterations)
{
        int i;

        for (i = 0; i < n_iterations; i++) {
                Image image;
                GString *gstr;

                image_init (&image, 64, 64, 1024);
                image_generate (&image, 0x00ff0000, 0x000000ff);

                gstr = g_string_new ("");
                image_print_sixels (&image, gstr);
                fwrite (gstr->str, sizeof (char), gstr->len, stdout);
                g_string_free (gstr, TRUE);

                image_deinit (&image);
        }
}

int
main (int argc, char *argv [])
{
        print_loop (1);
        return 0;
}
