/*
 *  Copyright 2024,2026 Patrick T. Head
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file svg.h
 *  @brief A simple SVG generation and editing library.
 */

#ifndef SVG_H
#define SVG_H

#include <xml.h>

  /**
   *  @enum svg_element_type
   *  SVG element types
   */

typedef enum
{
  svg_element_type_none = 0,  /**< Unknown element type      */
  svg_element_type_rect,      /**< SVG rect element type     */
  svg_element_type_circle,    /**< SVG circle element type   */
  svg_element_type_ellipse,   /**< SVG ellipse element type  */
  svg_element_type_line,      /**< SVG line element type     */
  svg_element_type_polygon,   /**< SVG polygon element type  */
  svg_element_type_polyline,  /**< SVG polyline element type */
  svg_element_type_path,      /**< SVG path element type     */
  svg_element_type_text,      /**< SVG text element type     */
  svg_element_type_textpath,  /**< SVG textpath element type */
  svg_element_type_link,      /**< SVG link element type     */
  svg_element_type_image,     /**< SVG image element type    */
  svg_element_type_marker,    /**< SVG marker element type   */
  svg_element_type_group,    /**< SVG group element type   */
} svg_element_type;

  /**
   *  @enum svg_length_adjust_type
   *  SVG length adjust types
   */

typedef enum
{
  svg_length_adjust_type_none = 0,               /**< see SVG reference for details */
  svg_length_adjust_type_spacing,            /**< see SVG reference for details */
  svg_length_adjust_type_spacing_and_glyphs  /**< see SVG reference for details */
} svg_length_adjust_type;

  /**
  *  @enum svg_spacing_type
  *  SVG spacing types
  */

typedef enum
{
  svg_spacing_type_none,  /**< see SVG reference for details */
  svg_spacing_type_auto,  /**< see SVG reference for details */
  svg_spacing_type_exact  /**< see SVG reference for details */
} svg_spacing_type;

  /**
  *  @enum svg_method_type
  *  SVG method types
  */

typedef enum
{
  svg_method_type_none,    /**< see SVG reference for details */
  svg_method_type_align,   /**< see SVG reference for details */
  svg_method_type_stretch  /**< see SVG reference for details */
} svg_method_type;

  /**
  *  @enum svg_text_length_type
  *  SVG text length types
  */

typedef enum
{
  svg_text_length_type_none,       /**< see SVG reference for details */
  svg_text_length_type_ems,        /**< see SVG reference for details */
  svg_text_length_type_exs,        /**< see SVG reference for details */
  svg_text_length_type_px,         /**< see SVG reference for details */
  svg_text_length_type_cm,         /**< see SVG reference for details */
  svg_text_length_type_mm,         /**< see SVG reference for details */
  svg_text_length_type_in,         /**< see SVG reference for details */
  svg_text_length_type_pc,         /**< see SVG reference for details */
  svg_text_length_type_pt,         /**< see SVG reference for details */
  svg_text_length_type_percentage  /**< see SVG reference for details */
} svg_text_length_type;

  /**
  *  @enum svg_orient_type
  *  SVG orient types
  */

typedef enum
{
  svg_orient_type_auto,                /**< see SVG reference for details */
  svg_orient_type_auto_start_reverse,  /**< see SVG reference for details */
  svg_orient_type_degrees,             /**< see SVG reference for details */
  svg_orient_type_radians,             /**< see SVG reference for details */
  svg_orient_type_gradians,            /**< see SVG reference for details */
  svg_orient_type_turns                /**< see SVG reference for details */
} svg_orient_type;

  /**
  *  @enum svg_fill_rule_type
  *  SVG fill rule types
  */

typedef enum
{
  svg_fill_rule_type_nonzero,  /**< see SVG reference for details */
  svg_fill_rule_type_evenodd   /**< see SVG reference for details */
} svg_fill_rule_type;

  /**
  *  @enum svg_stroke_linecap_type
  *  SVG line cap types
  */

typedef enum
{
  svg_stroke_linecap_type_butt,   /**< see SVG reference for details */
  svg_stroke_linecap_type_round,  /**< see SVG reference for details */
  svg_stroke_linecap_type_square  /**< see SVG reference for details */
} svg_stroke_linecap_type;

  /**
  *  @enum svg_stroke_linejoin_type
  *  SVG line join types
  */

typedef enum
{
  svg_stroke_linejoin_type_arcs,        /**< see SVG reference for details */
  svg_stroke_linejoin_type_bevel,       /**< see SVG reference for details */
  svg_stroke_linejoin_type_miter,       /**< see SVG reference for details */
  svg_stroke_linejoin_type_miter_clip,  /**< see SVG reference for details */
  svg_stroke_linejoin_type_round        /**< see SVG reference for details */
} svg_stroke_linejoin_type;

  /**
  *  @enum svg_font_weight_type
  *  SVG font weight types
  */

typedef enum
{
  svg_font_weight_type_normal,   /**< see SVG reference for details */
  svg_font_weight_type_bolder,   /**< see SVG reference for details */
  svg_font_weight_type_bold,     /**< see SVG reference for details */
  svg_font_weight_type_lighter,  /**< see SVG reference for details */
  svg_font_weight_type_100,      /**< see SVG reference for details */
  svg_font_weight_type_200,      /**< see SVG reference for details */
  svg_font_weight_type_300,      /**< see SVG reference for details */
  svg_font_weight_type_400,      /**< see SVG reference for details */
  svg_font_weight_type_500,      /**< see SVG reference for details */
  svg_font_weight_type_600,      /**< see SVG reference for details */
  svg_font_weight_type_700,      /**< see SVG reference for details */
  svg_font_weight_type_800,      /**< see SVG reference for details */
  svg_font_weight_type_900       /**< see SVG reference for details */
} svg_font_weight_type;

  /**
  *  @enum svg_font_stretch_type
  *  SVG font stretch types
  */

typedef enum
{
  svg_font_stretch_type_normal,           /**< see SVG reference for details */
  svg_font_stretch_type_ultra_condensed,  /**< see SVG reference for details */
  svg_font_stretch_type_extra_condensed,  /**< see SVG reference for details */
  svg_font_stretch_type_condensed,        /**< see SVG reference for details */
  svg_font_stretch_type_semi_condensed,   /**< see SVG reference for details */
  svg_font_stretch_type_semi_expanded,    /**< see SVG reference for details */
  svg_font_stretch_type_expanded,         /**< see SVG reference for details */
  svg_font_stretch_type_extra_expanded,   /**< see SVG reference for details */
  svg_font_stretch_type_ultra_expanded    /**< see SVG reference for details */
} svg_font_stretch_type;

  /**
  *  @enum svg_font_style_type
  *  SVG font style types
  */

typedef enum
{
  svg_font_style_type_normal,  /**< see SVG reference for details */
  svg_font_style_type_italic,  /**< see SVG reference for details */
  svg_font_style_type_oblique  /**< see SVG reference for details */
} svg_font_style_type;

  /**
  *  @enum svg_transform_type
  *  SVG transform types
  */

typedef enum
{
  svg_transform_type_none,       /**< see SVG reference for details */
  svg_transform_type_matrix,     /**< see SVG reference for details */
  svg_transform_type_translate,  /**< see SVG reference for details */
  svg_transform_type_scale,      /**< see SVG reference for details */
  svg_transform_type_rotate,     /**< see SVG reference for details */
  svg_transform_type_skew_x,     /**< see SVG reference for details */
  svg_transform_type_skew_y      /**< see SVG reference for details */
} svg_transform_type;

  /**
   *  @typedef svg_elements
   *  @brief creates type for struct svg_elements
   */

typedef struct svg_elements svg_elements;

  /** @typedef svg_point
   *  @brief creates type for struct svg_point
   */

typedef struct svg_point svg_point;

  /**
   *  @struct svg_point
   *  @brief generic point used in SVG elements
   */

struct svg_point
{
  double x;  /**< x value of coordinate */
  double y;  /**< y value of coordinate */
};

  /**
   *  @typedef svg_points
   *  @brief creates type for struct svg_points
   */

typedef struct svg_points svg_points;

  /**
   *  @struct svg_points
   *  @brief list of points
   */

typedef struct svg_points
{
  int size;         /**< count of points in list                                                   */
  int cursor;       /**< current point for @a svg_points_next and @a svg_points_previous functions */
  svg_point **arr;  /**< array of @a svg_point                                                     */
} svg_points;

  /**
   *  @typedef svg_text_length
   *  @brief creates type for struct svg_text_length
   */

typedef struct svg_text_length svg_text_length;

  /**
   *  @struct svg_text_length
   *  @brief data for text-length attribute
   */

struct svg_text_length
{
  svg_text_length_type type;  /**< type of length */
  double val;                 /**< value of length */
};

  /**
   *  @typedef svg_style
   *  @brief creates type for struct svg_style
   */

typedef struct svg_style svg_style;

  /**
   *  @struct svg_style
   *  @brief collection of style information that can be attached to any SVG element
   */

struct svg_style
{
  char *fill;                                /**< see SVG ref. for details */
  double fill_opacity;                       /**< see SVG ref. for details */
  svg_fill_rule_type fill_rule;              /**< see SVG ref. for details */
  char *stroke;                              /**< see SVG ref. for details */
  double stroke_width;                       /**< see SVG ref. for details */
  double stroke_opacity;                     /**< see SVG ref. for details */
  svg_stroke_linecap_type stroke_linecap;    /**< see SVG ref. for details */
  char *stroke_dash_array;                   /**< see SVG ref. for details */
  svg_stroke_linejoin_type stroke_linejoin;  /**< see SVG ref. for details */
  char *background_color;                    /**< see SVG ref. for details */
  char *font_family;                         /**< see SVG ref. for details */
  svg_font_weight_type font_weight;          /**< see SVG ref. for details */
  svg_font_stretch_type font_stretch;        /**< see SVG ref. for details */
  svg_font_style_type font_style;            /**< see SVG ref. for details */
  char *font_size;                           /**< see SVG ref. for details */
};

  /**
   *  @typedef svg_orient
   *  @brief creates type for struct svg_orient
   */

typedef struct svg_orient svg_orient;

  /**
   *  @struct svg_orient
   *  @brief data for orient attribute
   */

typedef struct svg_orient
{
  svg_orient_type type;  /**< type of orient  */
  double val;            /**< value of orient */
} svg_orient;

  /**
   *  @typedef svg_rect
   *  @brief creates type for struct svg_rect
   */

typedef struct svg_rect svg_rect;

  /**
   *  @struct svg_rect
   *  @brief data for SVG rect element
   */

struct svg_rect
{
  double width;   /**< see SVG reference for details */
  double height;  /**< see SVG reference for details */
  svg_point p;    /**< see SVG reference for details */
  double rx;      /**< see SVG reference for details */
  double ry;      /**< see SVG reference for details */
};

  /**
   *  @typedef svg_circle
   *  @brief creates type for struct svg_circle
   */

typedef struct svg_circle svg_circle;

  /**
   *  @struct svg_circle
   *  @brief data for SVG circle element
   */

struct svg_circle
{
  double r;     /**< see SVG reference for details */
  svg_point c;  /**< see SVG reference for details */
};

  /**
   *  @typedef svg_ellipse
   *  @brief creates type for struct @a svg_ellipse
   */

typedef struct svg_ellipse svg_ellipse;

  /**
   *  @struct svg_ellipse
   *  @brief data for SVG ellipse element
   */

struct svg_ellipse
{
  double rx;    /**< see SVG reference for details */
  double ry;    /**< see SVG reference for details */
  svg_point c;  /**< see SVG reference for details */
};

  /**
   *  @typedef svg_line
   *  @brief creates type for struct svg_line
   */

typedef struct svg_line svg_line;

  /**
   *  @struct svg_line
   *  @brief data for SVG line element
   */

struct svg_line
{
  svg_point p1;  /**< see SVG reference for details */
  svg_point p2;  /**< see SVG reference for details */
};

  /**
   *  @typedef svg_polygon
   *  creates type for struct svg_polygon
   */

typedef struct svg_polygon svg_polygon;

  /**
   *  @struct svg_polygon
   *  @brief data for SVG polygon element
   */

typedef struct svg_polygon
{
  svg_points *pts;  /**< see SVG reference for details */
} svg_polygon;      /**< see SVG reference for details */

  /**
   *  @typedef svg_polyline
   *  @brief creates type for struct svg_polyline
   */

typedef struct svg_polyline svg_polyline;

  /**
   *  @struct svg_polyline
   *  @brief data for SVG polyline element
   */

struct svg_polyline
{
  svg_points *pts;  /**< see SVG reference for details */
};

  /**
   *  @typedef svg_path
   *  @brief creates type for struct svg_path
   */

typedef struct svg_path svg_path;

  /**
   *  @struct svg_path
   *  @brief data for SVG path element
   */

struct svg_path
{
  char *d;  /**< see SVG reference for details */
};

  /**
   *  @typedef svg_text
   *  @brief creates type for struct svg_text
   */

typedef struct svg_text svg_text;

  /**
   *  @struct svg_text
   *  @brief data for SVG text element
   */

struct svg_text
{
  svg_point p;                           /**< see SVG reference for details */
  double dx;                             /**< see SVG reference for details */
  double dy;                             /**< see SVG reference for details */
  double rotate;                         /**< see SVG reference for details */
  svg_text_length text_length;           /**< see SVG reference for details */
  svg_length_adjust_type length_adjust;  /**< see SVG reference for details */
  char *contents;                        /**< see SVG reference for details */
};

  /**
   *  @typedef svg_textpath
   *  @brief creates type for struct svg_textpath
   */

typedef struct svg_textpath svg_textpath;

  /**
   *  @struct svg_textpath
   *  @brief data for SVG textpath element
   */

struct svg_textpath
{
  char *href;                            /**< see SVG reference for details */
  svg_length_adjust_type length_adjust;  /**< see SVG reference for details */
  svg_method_type method;                /**< see SVG reference for details */
  svg_spacing_type spacing;              /**< see SVG reference for details */
  svg_text_length start_offset;          /**< see SVG reference for details */
  svg_text_length text_length;           /**< see SVG reference for details */
  char *contents;                        /**< see SVG reference for details */
};

  /**
  *  @typedef svg_link
  *  @brief creates type for struct svg_link
  */

typedef struct svg_link svg_link;

  /**
  *  @struct svg_link
  *  @brief Contains definition data for SVG link element
  */

struct svg_link
{
  char *href;             /**< URL of link                                    */
  char *download;         /**< download attribute, NULL = false, empty = true */
  char *hreflang;         /**< language of page in URL                        */
  char *referrer_policy;  /**< the referrer to send when fetching URL         */
  char *rel;              /**< target-link relationship                       */
  char *target;           /**< _self, _parent, _top, _blank, or any name      */
  char *type;             /**< MIME type of href link                         */
  svg_elements *els;      /**< list of elements that define visual link       */
};

  /**
   *  @typedef svg_image
   *  @brief creates type for struct svg_image
   */

typedef struct svg_image svg_image;

  /**
   *  @struct svg_image
   *  @brief Contains definition data for SVG image element
   */

struct svg_image
{
  double width;    /**< see SVG reference for details */
  double height;   /**< see SVG reference for details */
  char *href;      /**< see SVG reference for details */
  svg_point p;     /**< see SVG reference for details */
};

  /**
   *  @typedef svg_marker
   *  @brief creates type for struct svg_marker
   */

typedef struct svg_marker svg_marker;

  /**
   *  @struct svg_marker
   *  @brief Contains definition data for SVG marker element
   */

typedef struct svg_marker
{
  double marker_height;  /**< see SVG reference for details                   */
  double marker_width;   /**< see SVG reference for details                   */
  svg_point ref;         /**< see SVG reference for details                   */
  svg_orient *orient;    /**< see SVG reference for details                   */
  svg_elements *els;     /**< list of elements included in marker definitions */
} svg_marker;

  /**
  *  @typedef svg_transform_matrix
  *  @brief creates type for struct svg_transform_matrix
  */

typedef struct svg_transform_matrix svg_transform_matrix;

  /**
  *  @struct svg_transform_matrix
  *  @brief Contains data for SVG matrix transform
  */

struct svg_transform_matrix
{
  double a;  /**< @b a matrix parameter, see SVG reference for details */
  double b;  /**< @b b matrix parameter, see SVG reference for details */
  double c;  /**< @b c matrix parameter, see SVG reference for details */
  double d;  /**< @b d matrix parameter, see SVG reference for details */
  double e;  /**< @b e matrix parameter, see SVG reference for details */
  double f;  /**< @b f matrix parameter, see SVG reference for details */
};

  /**
  *  @typedef svg_transform_translate
  *  @brief creates type for struct svg_transform_translate
  */

typedef struct svg_transform_translate svg_transform_translate;

  /**
  *  @struct svg_transform_translate
  *  @brief Contains data for SVG translate transform
  */

struct svg_transform_translate
{
  double x;  /**< @b x translate parameter, see SVG reference for details */
  double y;  /**< @b y translate parameter, see SVG reference for details */
};

  /**
  *  @typedef svg_transform_scale
  *  @brief creates type for struct svg_transform_scale
  */

typedef struct svg_transform_scale svg_transform_scale;

  /**
  *  @struct svg_transform_scale
  *  @brief Contains data for SVG scale transform
  */

struct svg_transform_scale
{
  double x;  /**< @b x scale paramter, see SVG reference for details */
  double y;  /**< @b y scale paramter, see SVG reference for details */
};

  /**
  *  @typedef svg_transform_rotate
  *  @brief creates type for struct svg_transform_rotate
  */

typedef struct svg_transform_rotate svg_transform_rotate;

  /**
  *  @struct svg_transform_rotate
  *  @brief Contains data for SVG rotate transform
  */

struct svg_transform_rotate
{
  double a;  /**< @b a rotate parameter, see SVG reference for details */
  double x;  /**< @b x rotate parameter, see SVG reference for details */
  double y;  /**< @b y rotate parameter, see SVG reference for details */
};

  /**
  *  @typedef svg_transform_skewX
  *  @brief creates type for struct svg_transform_skewX
  */

typedef struct svg_transform_skewX svg_transform_skewX;

  /**
  *  @struct svg_transform_skewX
  *  @brief Contains data for SVG skewX transform
  */

struct svg_transform_skewX
{
  double a;  /**< @b a skewX parameter, see SVG reference for details */
};

  /**
  *  @typedef svg_transform_skewY
  *  @brief creates type for struct svg_transform_skewY
  */

typedef struct svg_transform_skewY svg_transform_skewY;

  /**
  *  @struct svg_transform_skewY
  *  @brief Contains data for SVG skewY transform
  */

struct svg_transform_skewY
{
  double a;  /**< @b a skewY parameter, see SVG reference for details */
};

  /**
  *  @typedef svg_transform
  *  @brief creates type for struct svg_transform
  */

typedef struct svg_transform svg_transform;

  /**
  *  @struct svg_transform
  *  @brief Contains data for SVG transform
  */

struct svg_transform
{
  svg_transform_type type;              /**< indicates type of this transform */

  union
  {
    svg_transform_matrix matrix;        /**< matrix data                      */
    svg_transform_translate translate;  /**< translate data                   */
    svg_transform_scale scale;          /**< scale data                       */
    svg_transform_rotate rotate;        /**< rotate data                      */
    svg_transform_skewX skew_x;         /**< skewX data                       */
    svg_transform_skewY skew_y;         /**< skewY data                       */
  };
};

  /**
   *  @typedef svg_transforms
   *  @brief creates type for struct svg_transforms
   */

typedef struct svg_transforms svg_transforms;

  /**
   *  @struct svg_transforms
   *  @brief list of transforms included in transform attribute
   */

struct svg_transforms
{
  int size;             /**< count of transforms in list                      */
  int cursor;           /**< current transform for @a svg_transforms_next and */
                        /**< @a svg_transforms_previous functions             */
  svg_transform **arr;  /**< array of @a svg_transform                        */
};

  /**
   *  @typedef svg_group
   *  @brief creates type for struct svg_group
   */

typedef struct svg_group svg_group;

  /**
   *  @struct svg_group
   *  @brief contains data that defines an SVG group
   */

struct svg_group
{
  svg_elements *els;  /**<  list of elements contained in group */
};

  /**
   *  @typedef svg_element
   *  @brief creates type for struct svg_element
   */

typedef struct svg_element svg_element;

  /**
   *  @struct svg_element
   *  @brief contains data that defines an individual SVG element
   */

struct svg_element
{
  svg_element_type type;       /**< type of element  */
  union
  {
    void *any;                 /**< generic pointer for any type */
    svg_rect *r;               /**< SVG rect data  */
    svg_circle *c;             /**< SVG circle data  */
    svg_ellipse *e;            /**< SVG ellipse data  */
    svg_line *l;               /**< SVG line data  */
    svg_polygon *pg;           /**< SVG polygon data  */
    svg_polyline *pl;          /**< SVG polyline data  */
    svg_path *ph;              /**< SVG path data  */
    svg_text *t;               /**< SVG text data  */
    svg_textpath *tp;          /**< SVG textpath data  */
    svg_link *lnk;             /**< SVG link data  */
    svg_image *img;            /**< SVG image data  */
    svg_marker *m;             /**< SVG marker data  */
    svg_group *g;              /**< SVG group data  */
  };
  char *id;                    /**< id attribute for SVG element  */
  char *class;                 /**< class attribute for SVG element  */
  svg_style *style;            /**< SVG style applied to element  */
  svg_transforms *transforms;  /**< SVG transforms applied to element  */
};

  /**
   *  @struct svg_elements
   *  @brief list of elements
   */

struct svg_elements
{
  int size;           /**< count of elements in list  */
  int cursor;         /**< current element for next and previous functions */
  svg_element **arr;  /**< array of @a svg_element  */
};

  /**
   *  @typedef svg
   *  @brief creates type for struct svg
   */

typedef struct svg svg;

  /**
   *  @struct svg
   *  @brief complete definition of SVG document
   */

struct svg
{
  int width;          /**< see SVG reference for details */
  int height;         /**< see SVG reference for details */
  char *xmlns;        /**< see SVG reference for details */
  svg_elements *els;  /**< see SVG reference for details */
  svg_style *style;   /**< see SVG reference for details */
};

  /*
   *  Function prototypes
   */

    /*
     *  svg_point
     */

svg_point *svg_point_new(void);
svg_point *svg_point_new_with_all(double x, double y);
svg_point *svg_point_dup(svg_point *sp);
void svg_point_free(svg_point *sp);

double svg_point_get_x(svg_point *sp);
void svg_point_set_x(svg_point *sp, double x);

double svg_point_get_y(svg_point *sp);
void svg_point_set_y(svg_point *sp, double y);

    /*
     *  svg_points
     */

svg_points *svg_points_new(void);
svg_points *svg_points_dup(svg_points *sps);
void svg_points_free(svg_points *sps);

void svg_points_add(svg_points *sps, svg_point *sp);
void svg_points_remove(svg_points *sps, int index);

    /*
     *  svg_text_length
     */

svg_text_length *svg_text_length_new(void);
svg_text_length *svg_text_length_dup(svg_text_length *tl);
void svg_text_length_free(svg_text_length *tl);

svg_text_length_type svg_text_length_get_type(svg_text_length *tl);
void svg_text_length_set_type(svg_text_length *tl, svg_text_length_type type);

double svg_text_length_get_value(svg_text_length *tl);
void svg_text_length_set_value(svg_text_length *tl, double value);

    /*
     *  svg_orient
     */

svg_orient *svg_orient_new(void);
svg_orient *svg_orient_dup(svg_orient *o);
void svg_orient_free(svg_orient *o);

svg_orient_type svg_orient_get_type(svg_orient *o);
void svg_orient_set_type(svg_orient *o, svg_orient_type type);

double svg_orient_get_value(svg_orient *o);
void svg_orient_set_value(svg_orient *o, double val);

    /*
     * svg
     */

svg *svg_new(void);
svg *svg_dup(svg *s);
void svg_free(svg *s);

svg *svg_parse(char *svg_buf);
char *svg_to_string(svg *s);
xml *svg_to_xml(svg *s);

svg *svg_read(char *filename);
int svg_write(svg *s, char *filename);

int svg_get_width(svg *s);
void svg_set_width(svg *s, int width);

int svg_get_height(svg *s);
void svg_set_height(svg *s, int height);

char *svg_get_xmlns(svg *s);
void svg_set_xmlns(svg *s, char *xmlns);

svg_elements *svg_get_elements(svg *s);
void svg_set_elements(svg *s, svg_elements *els);

svg_style *svg_get_style(svg *s);
void svg_set_style(svg *s, svg_style *st);

    /*
     *  svg_style
     */

svg_style *svg_style_new(void);
svg_style *svg_style_dup(svg_style *st);
void svg_style_free(svg_style *st);

char *svg_style_get_fill(svg_style *st);
void svg_style_set_fill(svg_style *st, char *fill);

double svg_style_get_fill_opacity(svg_style *st);
void svg_style_set_fill_opacity(svg_style *st, double fill_opacity);

svg_fill_rule_type svg_style_get_fill_rule(svg_style *st);
void svg_style_set_fill_rule(svg_style *st, svg_fill_rule_type fill_rule);

char *svg_style_get_stroke(svg_style *st);
void svg_style_set_stroke(svg_style *st, char *stroke);

double svg_style_get_stroke_width(svg_style *st);
void svg_style_set_stroke_width(svg_style *st, double stroke_width);

double svg_style_get_stroke_opacity(svg_style *st);
void svg_style_set_stroke_opacity(svg_style *st, double stroke_opacity);

svg_stroke_linecap_type svg_style_get_stroke_linecap(svg_style *st);
void svg_style_set_stroke_linecap(svg_style *st, svg_stroke_linecap_type stroke_linecap);

char *svg_style_get_stroke_dash_array(svg_style *st);
void svg_style_set_stroke_dash_array(svg_style *st, char *stroke_dash_array);

svg_stroke_linejoin_type svg_style_get_stroke_linejoin(svg_style *st);
void svg_style_set_stroke_linejoin(svg_style *st, svg_stroke_linejoin_type stroke_linejoin);

char *svg_style_get_background_color(svg_style *st);
void svg_style_set_background_color(svg_style *st, char *background_color);

char *svg_style_get_font_family(svg_style *st);
void svg_style_set_font_family(svg_style *st, char *font_family);

svg_font_weight_type svg_style_get_font_weight(svg_style *st);
void svg_style_set_font_weight(svg_style *st, svg_font_weight_type font_weight);

svg_font_stretch_type svg_style_get_font_stretch(svg_style *st);
void svg_style_set_font_stretch(svg_style *st, svg_font_stretch_type font_stretch);

svg_font_style_type svg_style_get_font_style(svg_style *st);
void svg_style_set_font_style(svg_style *st, svg_font_style_type font_style);

char *svg_style_get_font_size(svg_style *st);
void svg_style_set_font_size(svg_style *st, char *font_size);

    /*
     *  svg_element
     */

svg_element *svg_element_new(void);
svg_element *svg_element_dup(svg_element *el);
void svg_element_free(svg_element *el);

svg_element_type svg_element_get_type(svg_element *el);
void svg_element_set_type(svg_element *el, svg_element_type type);

void *svg_element_get_element(svg_element *el);
void svg_element_set_element(svg_element *el, svg_element_type type, void *ele);

char *svg_element_get_id(svg_element *el);
void svg_element_set_id(svg_element *el, char *id);

char *svg_element_get_class(svg_element *el);
void svg_element_set_class(svg_element *el, char *class);

svg_style *svg_element_get_style(svg_element *el);
void svg_element_set_style(svg_element *el, svg_style *style);

svg_transforms *svg_element_get_transforms(svg_element *el);
void svg_element_set_transforms(svg_element *el, svg_transforms *str);

    /*
     *  svg_elements
     */

svg_elements *svg_elements_new(void);
svg_elements *svg_elements_dup(svg_elements *els);
void svg_elements_free(svg_elements *els);

int svg_elements_get_size(svg_elements *els);
void svg_elements_set_size(svg_elements *els, int size);

int svg_elements_get_cursor(svg_elements *els);
void svg_elements_set_cursor(svg_elements *els, int cursor);

void svg_elements_add(svg_elements *els, svg_element *el);
void svg_elements_remove(svg_elements *els, int index);

    /*
     *  svg_rect
     */

svg_rect *svg_rect_new(void);
svg_rect *svg_rect_dup(svg_rect *r);
void svg_rect_free(svg_rect *r);

double svg_rect_get_width(svg_rect *r);
void svg_rect_set_width(svg_rect *r, double width);

double svg_rect_get_height(svg_rect *r);
void svg_rect_set_height(svg_rect *r, double height);

svg_point *svg_rect_get_point(svg_rect *r);
void svg_rect_set_point(svg_rect *r, svg_point *p);

double svg_rect_get_rx(svg_rect *r);
void svg_rect_set_rx(svg_rect *r, double rx);

double svg_rect_get_ry(svg_rect *r);
void svg_rect_set_ry(svg_rect *r, double ry);

    /*
     *  svg_circle
     */

svg_circle *svg_circle_new(void);
svg_circle *svg_circle_dup(svg_circle *c);
void svg_circle_free(svg_circle *c);

double svg_circle_get_r(svg_circle *c);
void svg_circle_set_r(svg_circle *c, double r);

svg_point *svg_circle_get_c(svg_circle *c);
void svg_circle_set_c(svg_circle *c, svg_point *center);

    /*
     *  svg_ellipse
     */

svg_ellipse *svg_ellipse_new(void);
svg_ellipse *svg_ellipse_dup(svg_ellipse *e);
void svg_ellipse_free(svg_ellipse *e);

double svg_ellipse_get_rx(svg_ellipse *e);
void svg_ellipse_set_rx(svg_ellipse *e, double rx);

double svg_ellipse_get_ry(svg_ellipse *e);
void svg_ellipse_set_ry(svg_ellipse *e, double ry);

svg_point *svg_ellipse_get_c(svg_ellipse *e);
void svg_ellipse_set_c(svg_ellipse *e, svg_point *c);

    /*
     *  svg_line
     */

svg_line *svg_line_new(void);
svg_line *svg_line_dup(svg_line *l);
void svg_line_free(svg_line *l);

svg_point *svg_line_get_p1(svg_line *l);
void svg_line_set_p1(svg_line *l, svg_point *p1);

svg_point *svg_line_get_p2(svg_line *l);
void svg_line_set_p2(svg_line *l, svg_point *p2);

    /*
     *  svg_polygon
     */

svg_polygon *svg_polygon_new(void);
svg_polygon *svg_polygon_dup(svg_polygon *pg);
void svg_polygon_free(svg_polygon *pg);

svg_points *svg_polygon_get_points(svg_polygon *pg);
void svg_polygon_set_points(svg_polygon *pg, svg_points *pts);

    /*
     *  svg_polyline
     */

svg_polyline *svg_polyline_new(void);
svg_polyline *svg_polyline_dup(svg_polyline *pl);
void svg_polyline_free(svg_polyline *pl);

svg_points *svg_polyline_get_points(svg_polyline *pl);
void svg_polyline_set_points(svg_polyline *pl, svg_points *pts);

    /*
     *  svg_path
     */

svg_path *svg_path_new(void);
svg_path *svg_path_dup(svg_path *ph);
void svg_path_free(svg_path *ph);

char *svg_path_get_d(svg_path *ph);
void svg_path_set_d(svg_path *ph, char *d);

    /*
     *  svg_text
     */

svg_text *svg_text_new(void);
svg_text *svg_text_dup(svg_text *t);
void svg_text_free(svg_text *t);

svg_point *svg_text_get_p(svg_text *t);
void svg_text_set_p(svg_text *t, svg_point *p);

double svg_text_get_dx(svg_text *t);
void svg_text_set_dx(svg_text *t, double dx);

double svg_text_get_dy(svg_text *t);
void svg_text_set_dy(svg_text *t, double dy);

double svg_text_get_rotate(svg_text *t);
void svg_text_set_rotate(svg_text *t, double rotate);

svg_text_length *svg_text_get_text_length(svg_text *t);
void svg_text_set_text_length(svg_text *t, svg_text_length *tl);

svg_length_adjust_type svg_text_get_length_adjust(svg_text *t);
void svg_text_set_length_adjust(svg_text *t, svg_length_adjust_type la);

char *svg_text_get_contents(svg_text *t);
void svg_text_set_contents(svg_text *t, char *contents);

    /*
     *  svg_textpath
     */

svg_textpath *svg_textpath_new(void);
svg_textpath *svg_textpath_dup(svg_textpath *tp);
void svg_textpath_free(svg_textpath *tp);

char *svg_textpath_get_href(svg_textpath *tp);
void svg_textpath_set_href(svg_textpath *tp, char *href);

svg_length_adjust_type svg_textpath_get_length_adjust(svg_textpath *tp);
void svg_textpath_set_length_adjust(svg_textpath *tp, svg_length_adjust_type la);

svg_method_type svg_textpath_get_method(svg_textpath *tp);
void svg_textpath_set_method(svg_textpath *tp, svg_method_type m);

svg_spacing_type svg_textpath_get_spacing(svg_textpath *tp);
void svg_textpath_set_spacing(svg_textpath *tp, svg_spacing_type s);

svg_text_length *svg_textpath_get_start_offset(svg_textpath *tp);
void svg_textpath_set_start_offset(svg_textpath *tp, svg_text_length *so);

svg_text_length *svg_textpath_get_text_length(svg_textpath *tp);
void svg_textpath_set_text_length(svg_textpath *tp, svg_text_length *tl);

char *svg_textpath_get_contents(svg_textpath *tp);
void svg_textpath_set_contents(svg_textpath *tp, char *contents);

    /*
     *  svg_link
     */

svg_link *svg_link_new(void);
svg_link *svg_link_dup(svg_link *lnk);
void svg_link_free(svg_link *lnk);

char *svg_link_get_href(svg_link *lnk);
void svg_link_set_href(svg_link *lnk, char *href);

char *svg_link_get_download(svg_link *lnk);
void svg_link_set_download(svg_link *lnk, char *download);

char *svg_link_get_hreflang(svg_link *lnk);
void svg_link_set_hreflang(svg_link *lnk, char *hreflang);

char *svg_link_get_referrer_policy(svg_link *lnk);
void svg_link_set_referrer_policy(svg_link *lnk, char *referrer_policy);

char *svg_link_get_rel(svg_link *lnk);
void svg_link_set_rel(svg_link *lnk, char *rel);

char *svg_link_get_target(svg_link *lnk);
void svg_link_set_target(svg_link *lnk, char *target);

char *svg_link_get_type(svg_link *lnk);
void svg_link_set_type(svg_link *lnk, char *type);

svg_elements *svg_link_get_elements(svg_link *lnk);
void svg_link_set_elements(svg_link *lnk, svg_elements *ses);

    /*
     *  svg_image
     */

svg_image *svg_image_new(void);
svg_image *svg_image_dup(svg_image *img);
void svg_image_free(svg_image *img);

double svg_image_get_width(svg_image *img);
void svg_image_set_width(svg_image *img, double width);

double svg_image_get_height(svg_image *img);
void svg_image_set_height(svg_image *img, double width);

char *svg_image_get_href(svg_image *img);
void svg_image_set_href(svg_image *img, char *href);

svg_point *svg_image_get_p(svg_image *img);
void svg_image_set_p(svg_image *img, svg_point *p);

    /*
     *  svg_marker
     */

svg_marker *svg_marker_new(void);
svg_marker *svg_marker_dup(svg_marker *m);
void svg_marker_free(svg_marker *m);

double svg_marker_get_marker_height(svg_marker *m);
void svg_marker_set_marker_height(svg_marker *m, double marker_height);

double svg_marker_get_marker_width(svg_marker *m);
void svg_marker_set_marker_width(svg_marker *m, double marker_width);

svg_point *svg_marker_get_ref(svg_marker *m);
void svg_marker_set_ref(svg_marker *m, svg_point *ref);

svg_orient *svg_marker_get_orient(svg_marker *m);
void svg_marker_set_orient(svg_marker *m, svg_orient *orient);

svg_elements *svg_marker_get_elements(svg_marker *m);
void svg_marker_set_elements(svg_marker *m, svg_elements *ses);

    /*
     *  svg_group
     */

svg_group *svg_group_new(void);
svg_group *svg_group_dup(svg_group *m);
void svg_group_free(svg_group *m);

svg_elements *svg_group_get_elements(svg_group *m);
void svg_group_set_elements(svg_group *m, svg_elements *ses);

void svg_group_add(svg_group *g, svg_element *el);
void svg_group_remove(svg_group *g, int index);

    /*
     *  svg_transform and sub types
     */

      /*
       *  svg_transform
       */

svg_transform *svg_transform_new(void);
svg_transform *svg_transform_dup(svg_transform *st);
void svg_transform_free(svg_transform *st);

svg_transform_type svg_transform_get_type(svg_transform *st);
void svg_transform_set_type(svg_transform *st, svg_transform_type type);

svg_transform_matrix *svg_transform_get_matrix(svg_transform *st);
void svg_transform_set_matrix(svg_transform *st, svg_transform_matrix *stm);

svg_transform_translate *svg_transform_get_translate(svg_transform *st);
void svg_transform_set_translate(svg_transform *st, svg_transform_translate *stt);

svg_transform_scale *svg_transform_get_scale(svg_transform *st);
void svg_transform_set_scale(svg_transform *st, svg_transform_scale *sts);

svg_transform_rotate *svg_transform_get_rotate(svg_transform *st);
void svg_transform_set_rotate(svg_transform *st, svg_transform_rotate *str);

svg_transform_skewX *svg_transform_get_skewX(svg_transform *st);
void svg_transform_set_skewX(svg_transform *st, svg_transform_skewX *sts);

svg_transform_skewY *svg_transform_get_skewY(svg_transform *st);
void svg_transform_set_skewY(svg_transform *st, svg_transform_skewY *sts);

      /*
       *  svg_transforms
       */

svg_transforms *svg_transforms_new(void);
svg_transforms *svg_transforms_dup(svg_transforms *sts);
void svg_transforms_free(svg_transforms *sts);

void svg_transforms_add(svg_transforms *sts, svg_transform *st);
void svg_transforms_remove(svg_transforms *sts, int idx);
svg_transform *svg_transforms_next(svg_transforms *sts);
svg_transform *svg_transforms_previous(svg_transforms *sts);

      /*
       *  svg_transform_matrix
       */

svg_transform_matrix *svg_transform_matrix_new(void);
svg_transform_matrix *svg_transform_matrix_new_with_all(double a, double b, double c, double d, double e, double f);
svg_transform_matrix *svg_transform_matrix_dup(svg_transform_matrix *stm);
void svg_transform_matrix_free(svg_transform_matrix *stm);

double svg_transform_matrix_get_a(svg_transform_matrix *stm);
void svg_transform_matrix_set_a(svg_transform_matrix *stm, double a);

double svg_transform_matrix_get_b(svg_transform_matrix *stm);
void svg_transform_matrix_set_b(svg_transform_matrix *stm, double b);

double svg_transform_matrix_get_c(svg_transform_matrix *stm);
void svg_transform_matrix_set_c(svg_transform_matrix *stm, double c);

double svg_transform_matrix_get_d(svg_transform_matrix *stm);
void svg_transform_matrix_set_d(svg_transform_matrix *stm, double d);

double svg_transform_matrix_get_e(svg_transform_matrix *stm);
void svg_transform_matrix_set_e(svg_transform_matrix *stm, double e);

double svg_transform_matrix_get_f(svg_transform_matrix *stm);
void svg_transform_matrix_set_f(svg_transform_matrix *stm, double f);

      /*
       *  svg_transform_translate
       */

svg_transform_translate *svg_transform_translate_new(void);
svg_transform_translate *svg_transform_translate_new_with_all(double x, double y);
svg_transform_translate *svg_transform_translate_dup(svg_transform_translate *stt);
void svg_transform_translate_free(svg_transform_translate *stt);

double svg_transform_translate_get_x(svg_transform_translate *stt);
void svg_transform_translate_set_x(svg_transform_translate *stt, double x);

double svg_transform_translate_get_y(svg_transform_translate *stt);
void svg_transform_translate_set_y(svg_transform_translate *stt, double y);

      /*
       *  svg_transform_scale
       */

svg_transform_scale *svg_transform_scale_new(void);
svg_transform_scale *svg_transform_scale_new_with_all(double x, double y);
svg_transform_scale *svg_transform_scale_dup(svg_transform_scale *sts);
void svg_transform_scale_free(svg_transform_scale *sts);

double svg_transform_scale_get_x(svg_transform_scale *sts);
void svg_transform_scale_set_x(svg_transform_scale *sts, double x);

double svg_transform_scale_get_y(svg_transform_scale *sts);
void svg_transform_scale_set_y(svg_transform_scale *sts, double y);

      /*
       *  svg_transform_rotate
       */

svg_transform_rotate *svg_transform_rotate_new(void);
svg_transform_rotate *svg_transform_rotate_new_with_all(double a, double x, double y);
svg_transform_rotate *svg_transform_rotate_dup(svg_transform_rotate *str);
void svg_transform_rotate_free(svg_transform_rotate *str);

double svg_transform_rotate_get_a(svg_transform_rotate *str);
void svg_transform_rotate_set_a(svg_transform_rotate *str, double a);

double svg_transform_rotate_get_x(svg_transform_rotate *str);
void svg_transform_rotate_set_x(svg_transform_rotate *str, double x);

double svg_transform_rotate_get_y(svg_transform_rotate *str);
void svg_transform_rotate_set_y(svg_transform_rotate *str, double y);

      /*
       *  svg_transform_skewX
       */

svg_transform_skewX *svg_transform_skewX_new(void);
svg_transform_skewX *svg_transform_skewX_new_with_all(double a);
svg_transform_skewX *svg_transform_skewX_dup(svg_transform_skewX *sts);
void svg_transform_skewX_free(svg_transform_skewX *sts);

double svg_transform_skewX_get_a(svg_transform_skewX *sts);
void svg_transform_skewX_set_a(svg_transform_skewX *sts, double a);

      /*
       *  svg_transform_skewY
       */

svg_transform_skewY *svg_transform_skewY_new(void);
svg_transform_skewY *svg_transform_skewY_new_with_all(double a);
svg_transform_skewY *svg_transform_skewY_dup(svg_transform_skewY *sts);
void svg_transform_skewY_free(svg_transform_skewY *sts);

double svg_transform_skewY_get_a(svg_transform_skewY *sts);
void svg_transform_skewY_set_a(svg_transform_skewY *sts, double a);

#endif
