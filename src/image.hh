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

        // Draw/prune priority, must be unique
        int m_priority;

        // Image dimensions in pixels
        int m_width_pixels;
        int m_height_pixels;

        // Image geometry in cell units
        int m_left_cells;
        int m_top_cells;
        int m_width_cells;
        int m_height_cells;

public:
        Image(vte::cairo::Surface&& surface,
              int priority,
              int width_pixels, int height_pixels,
              int col, int row,
              int width_cells, int height_cells) noexcept
                : m_surface{std::move(surface)},
                  m_priority{priority},
                  m_width_pixels{width_pixels},
                  m_height_pixels{height_pixels},
                  m_left_cells{col},
                  m_top_cells{row},
                  m_width_cells{width_cells},
                  m_height_cells{height_cells}
        {
        }

        inline constexpr auto get_priority() const noexcept { return m_priority; }
        inline constexpr auto get_left() const noexcept { return m_left_cells; }
        inline constexpr auto get_top() const noexcept { return m_top_cells; }
        inline constexpr auto get_bottom() const noexcept { return m_top_cells + m_height_cells - 1; }
        inline constexpr auto get_width() const noexcept { return m_width_cells; }
        inline constexpr auto get_height() const noexcept { return m_height_cells; }
        inline auto resource_size() const noexcept {
                if (cairo_image_surface_get_stride(m_surface.get()) != 0)
                        return cairo_image_surface_get_stride(m_surface.get()) * m_height_pixels;

                /* Not an image surface: Only the device knows for sure, so we guess */
                return m_width_pixels * m_height_pixels * 4;
        }
        bool contains(const Image &other) const noexcept;
        void paint(cairo_t *cr, gint offset_x, gint offset_y) const noexcept;
};

} // namespace image

} // namespace vte
