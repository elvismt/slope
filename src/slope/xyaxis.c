/*
 * Copyright (C) 2016  Elvis Teixeira
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
 * PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <slope/xyaxis.h>
#include <slope/scale.h>

typedef struct
_SlopeXyAxisPrivate
{
    int orientation;

    double min;
    double max;
    double anchor;

    SlopeColor color;
    SlopeColor selected_color;
    double line_width;

    SlopeXyAxisSampler *sampler;
}
SlopeXyAxisPrivate;


#define SLOPE_XYAXIS_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
     SLOPE_XYAXIS_TYPE, SlopeXyAxisPrivate))

G_DEFINE_TYPE_WITH_PRIVATE(
    SlopeXyAxis, slope_xyaxis, SLOPE_ITEM_TYPE)


static void _xyaxis_finalize (GObject *self);
static void _xyaxis_get_figure_rect (SlopeItem *self, SlopeRect *rect);
static void _xyaxis_get_data_rect (SlopeItem *self, SlopeRect *rect);
static void _xyaxis_draw (SlopeItem *self, cairo_t *cr);
static void _xyaxis_draw_horizontal (SlopeXyAxis *self, cairo_t *cr);
static void _xyaxis_draw_vertical (SlopeXyAxis *self, cairo_t *cr);


static void
slope_xyaxis_class_init (SlopeXyAxisClass *klass)
{
    GObjectClass *object_klass = G_OBJECT_CLASS(klass);
    SlopeItemClass *item_klass = SLOPE_ITEM_CLASS(klass);

    object_klass->finalize = _xyaxis_finalize;

    item_klass->draw = _xyaxis_draw;
    item_klass->get_data_rect = _xyaxis_get_data_rect;
    item_klass->get_figure_rect = _xyaxis_get_figure_rect;
}


static void
slope_xyaxis_init (SlopeXyAxis *self)
{
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);

    priv->orientation = SLOPE_XYAXIS_HORIZONTAL;
    priv->color = SLOPE_GREY1;
    priv->selected_color = SLOPE_BLUE;
    priv->line_width = 1.0;

    priv->sampler = slope_xyaxis_sampler_new();
}


static
void _xyaxis_finalize (GObject *self)
{
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);

    slope_xyaxis_sampler_destroy(priv->sampler);
}


SlopeItem* slope_xyaxis_new (int orientation)
{
    SlopeXyAxis *self = SLOPE_XYAXIS(g_object_new(SLOPE_XYAXIS_TYPE, NULL));
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);

    priv->orientation = orientation;
    return SLOPE_ITEM(self);
}


static
void _xyaxis_draw (SlopeItem *self, cairo_t *cr)
{
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);

    cairo_set_line_width(cr, priv->line_width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    if (priv->orientation == SLOPE_XYAXIS_HORIZONTAL) {
        _xyaxis_draw_horizontal(SLOPE_XYAXIS(self), cr);
    }
    else if (priv->orientation == SLOPE_XYAXIS_VERTICAL) {
        _xyaxis_draw_vertical(SLOPE_XYAXIS(self), cr);
    }
}


static
void _xyaxis_draw_horizontal (SlopeXyAxis *self, cairo_t *cr)
{
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);
    SlopeScale *scale = slope_item_get_scale(SLOPE_ITEM(self));
    cairo_text_extents_t txt_ext;
    SlopePoint p, p1, p2;
    GList *sample_list, *iter;
    double txt_height;

    cairo_text_extents(cr, "dummy", &txt_ext);
    txt_height = txt_ext.height;

    p.x = priv->min;
    p.y = priv->anchor;
    slope_scale_map(scale, &p1, &p);

    p.x = priv->max;
    p.y = priv->anchor;
    slope_scale_map(scale, &p2, &p);

    cairo_new_path(cr);
    slope_cairo_set_color(cr, priv->color);
    slope_cairo_line_cosmetic(cr, &p1, &p2, priv->line_width);
    cairo_stroke(cr);

    sample_list = slope_xyaxis_sampler_get_sample_list(priv->sampler);
    iter = sample_list;

    while (iter != NULL) {
        SlopeXyAxisSample *sample = SLOPE_XYAXIS_SAMPLE(iter->data);
        SlopePoint sample_p1, sample_p2;

        p.x = sample->coord;
        p.y = priv->anchor;
        slope_scale_map(scale, &sample_p1, &p);
        sample_p2 = sample_p1;
        sample_p2.y -= 5.0;

        slope_cairo_line_cosmetic(cr, &sample_p1, &sample_p2, priv->line_width);
        cairo_stroke(cr);

        if (sample->label != NULL) {
            cairo_text_extents(cr, sample->label, &txt_ext);
            slope_cairo_text(cr,
                sample_p1.x - txt_ext.width * 0.5,
                sample_p1.y + txt_height * 1.25,
                sample->label);
        }

        iter = iter->next;
    }
}


static
void _xyaxis_draw_vertical (SlopeXyAxis *self, cairo_t *cr)
{
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);
    SlopeScale *scale = slope_item_get_scale(SLOPE_ITEM(self));
    SlopePoint p, p1, p2;

    p.x = priv->anchor;
    p.y = priv->min;
    slope_scale_map(scale, &p1, &p);

    p.x = priv->anchor;
    p.y = priv->max;
    slope_scale_map(scale, &p2, &p);

    cairo_new_path(cr);
    slope_cairo_set_color(cr, priv->color);
    slope_cairo_line_cosmetic(cr, &p1, &p2, priv->line_width);
    cairo_stroke(cr);
}


void slope_xyaxis_set_position (SlopeXyAxis *self, double min,
                                double max, double anchor)
{
    SlopeXyAxisPrivate *priv = SLOPE_XYAXIS_GET_PRIVATE(self);

    priv->min = min;
    priv->max = max;
    priv->anchor = anchor;
}


static void _xyaxis_get_figure_rect (SlopeItem *self, SlopeRect *rect)
{

}


static void _xyaxis_get_data_rect (SlopeItem *self, SlopeRect *rect)
{

}


SlopeXyAxisSampler* slope_xyaxis_get_sampler (SlopeXyAxis *self)
{
    return SLOPE_XYAXIS_GET_PRIVATE(self)->sampler;
}

/* slope/xyaxis.c */