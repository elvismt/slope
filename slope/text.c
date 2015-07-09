/*
 * Copyright (C) 2015  Elvis Teixeira
 *
 * This source code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any
 * later version.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "slope/text.h"
#include "slope-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if SLOPE_HAVE_PANGO
# include <pango/pangocairo.h>
#endif


struct _slope_font
{
    #if SLOPE_HAVE_PANGO
    PangoFontDescription *desc;
    #else /* cairo toy api */
    char *desc;
    int size;
    #endif /* SLOPE_HAVE_PANGO */
};


void slope_draw_text(cairo_t *cr, slope_font_t *font,
                     double x, double y, const char *text)
{
    #if SLOPE_HAVE_PANGO
    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description (layout, font->desc);
    pango_layout_set_text(layout, text, -1);
    pango_cairo_update_layout(cr, layout);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout_line(cr, pango_layout_get_line (layout, 0));
    g_object_unref(layout);
    #else  /* cairo toy api */
    cairo_select_font_face(
        cr, font->desc, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font->size);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
    #endif /* SLOPE_HAVE_PANGO */
}


void slope_get_text_rect(cairo_t *cr, slope_font_t *font,
                         slope_rect_t *rect, const char *text)
{
    #if SLOPE_HAVE_PANGO
    int width, height;
    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description (layout, font->desc);
    pango_layout_set_text(layout, text, -1);
    pango_cairo_update_layout(cr, layout);
    pango_layout_get_pixel_size(layout, &width, &height);
    rect->width = (double) width;
    rect->height = (double) height;
    g_object_unref(layout);
    #else  /* cairo toy api */
    cairo_text_extents_t txt_ext;
    cairo_select_font_face(
        cr, font->desc, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font->size);
    cairo_text_extents(cr, text, &txt_ext);
    rect->width = txt_ext.width;
    rect->height = txt_ext.height;
    #endif /* SLOPE_HAVE_PANGO */
}


slope_font_t* slope_font_create (const char *family, int size)
{
    slope_font_t *font = malloc(sizeof(slope_font_t));
    font->desc = NULL;
    slope_font_select(font, family, size);
    return font;
}


void slope_font_destroy (slope_font_t *font)
{
    if (font == NULL) return;
    #if SLOPE_HAVE_PANGO
    if (font->desc != NULL) {
        pango_font_description_free(font->desc);
    }
    #else /* cairo toy api */
    if (font->desc != NULL) {
        free(font->desc);
    }
    #endif /* SLOPE_HAVE_PANGO */
    free(font);
}


void slope_font_select (slope_font_t *font,
                        const char *family, int size)
{
    #if SLOPE_HAVE_PANGO
    char fontdesc[40];
    sprintf(fontdesc, "%s %d", family, size);
    if (font->desc != NULL) {
        pango_font_description_free(font->desc);
    }
    font->desc = pango_font_description_from_string (fontdesc);
    #else /* cairo toy api */
    if (font->desc != NULL) {
        free(font->desc);
    }
    font->desc = strdup(family);
    font->size = size + 2; /* Why pango fonts appear bigger? */
    #endif /* SLOPE_HAVE_PANGO */
}