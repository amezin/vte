/*
 * Copyright © 2016-2020 Hayaki Saito <saitoha@me.com>
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

#pragma once

#include <pango/pangocairo.h>
#include "vtestream.h"

namespace vte {

namespace image {

struct Image {
private:
        int m_pixelwidth;           /* Image width in pixels */
        int m_pixelheight;          /* Image height in pixels */
        int m_left;                 /* Left position in cell units */
        int m_top;                  /* Top position in cell units */
        int m_width;                /* Width in cell units */
        int m_height;               /* Height in cell units */
        VteStream *m_stream;        /* For serialization */
        unsigned long m_position;   /* Indicates the position at the stream if it's serialized */
        size_t m_nread;             /* Private use: for read callback */
        size_t m_nwrite;            /* Private use: for write callback */
        cairo_surface_t *m_surface; /* Internal cairo image */
public:
        explicit Image(cairo_surface_t *surface, int pixelwidth, int pixelheight,
                       int col, int row, int w, int h, _VteStream *stream);
        ~Image();
        long get_left() const;
        long get_top() const;
        long get_bottom() const;
        unsigned long get_stream_position() const;
        bool is_frozen() const;
        bool contains(const Image *rhs) const;
        size_t resource_size() const;
        void freeze();
        bool thaw();
        bool subsume(Image *rhs, gulong cell_width, gulong cell_height);
        bool unite(Image *rhs, gulong cell_width, gulong cell_height);
        bool paint(cairo_t *cr, gint offsetx, gint offsety);

        /* Callbacks */

        static cairo_status_t read_callback(void *closure, char *data, unsigned int length);
        static cairo_status_t write_callback(void *closure, const char *data, unsigned int length);
};

} // namespace image

} // namespace vte
