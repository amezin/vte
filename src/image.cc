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

/* Paint the image with provided cairo context */
void
Image::paint(cairo_t *cr, int offset_x, int offset_y, int cell_width, int cell_height) const noexcept
{
        /* FIXME-hpj: Scale the image if cell width/height is different */

        cairo_save(cr);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_rectangle(cr, offset_x, offset_y, m_width_pixels, m_height_pixels);
        cairo_clip(cr);
        cairo_set_source_surface(cr, m_surface.get(), offset_x, offset_y);
        cairo_paint(cr);
        cairo_restore(cr);
}

} // namespace image

} // namespace vte
