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

#include "config.h"

#include <glib.h>
#include <stdio.h>
#include "vteinternal.hh"
#include "image.hh"

namespace vte {

namespace image {

/* Test whether this image contains given image */
bool
Image::contains(const Image &other) const
{
        return other.m_left >= m_left &&
               other.m_top >= m_top &&
               other.m_left + other.m_width <= m_left + m_width &&
               other.m_top + other.m_height <= m_top + m_height;
}

/* Paint the image with provided cairo context */
bool
Image::paint(cairo_t *cr, int offsetx, int offsety)
{
        cairo_save(cr);
        cairo_rectangle(cr, offsetx, offsety, m_pixelwidth, m_pixelheight);
        cairo_clip(cr);
        cairo_set_source_surface(cr, m_surface.get(), offsetx, offsety);
        cairo_paint(cr);
        cairo_restore(cr);

        return true;
}

} // namespace image

} // namespace vte
