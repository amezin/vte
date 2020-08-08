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
#include "cairo-glue.hh"

namespace vte {

namespace image {

struct Image {
private:
        // Device-friendly Cairo surface
        vte::cairo::Surface m_surface{};

        // Image dimensions in pixels
        int m_pixelwidth;
        int m_pixelheight;

        // Image geometry in cell units
        int m_left;
        int m_top;
        int m_width;
        int m_height;

public:
        Image(vte::cairo::Surface&& surface, int width_pixels, int height_pixels,
              int col, int row, int width_cells, int height_cells) noexcept
                : m_surface{std::move(surface)},
                  m_pixelwidth{width_pixels},
                  m_pixelheight{height_pixels},
                  m_left{col},
                  m_top{row},
                  m_width{width_cells},
                  m_height{height_cells}
        {
        }

        constexpr auto const get_left() const noexcept { return m_left; }
        constexpr auto const get_top() const noexcept { return m_top; }
        constexpr auto const get_bottom() const noexcept { return m_top + m_height - 1; }
        auto const resource_size() const noexcept {
                return cairo_image_surface_get_stride(m_surface.get()) * m_pixelheight;
        }
        bool contains(const Image &other) const;
        bool paint(cairo_t *cr, gint offsetx, gint offsety);
};

} // namespace image

} // namespace vte
