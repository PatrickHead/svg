/*
 *  Copyright 2024 Patrick T. Head
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

/** @file svg.c
 *  @brief A simple SVG generation and editing library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#include "xml.h"

#include "svg.h"

static char *strapp(char *s1, char *s2);
static xml_element *element_to_xml(svg_element *el);
static xml_elements *elements_to_xml(svg_elements *els);
static xml_element *rect_to_xml(svg_element *el);
static xml_element *circle_to_xml(svg_element *el);
static xml_element *ellipse_to_xml(svg_element *el);
static xml_element *line_to_xml(svg_element *el);
static xml_element *polygon_to_xml(svg_element *el);
static xml_element *polyline_to_xml(svg_element *el);
static xml_element *path_to_xml(svg_element *el);
static xml_element *text_to_xml(svg_element *el);
static xml_element *textpath_to_xml(svg_element *el);
static xml_element *link_to_xml(svg_element *el);
static xml_element *image_to_xml(svg_element *el);
static xml_element *marker_to_xml(svg_element *el);
static char *transform_matrix_to_xml(svg_transform_matrix *stm);
static char *transform_translate_to_xml(svg_transform_translate *stt);
static char *transform_scale_to_xml(svg_transform_scale *sts);
static char *transform_rotate_to_xml(svg_transform_rotate *str);
static char *transform_skewX_to_xml(svg_transform_skewX *sts);
static char *transform_skewY_to_xml(svg_transform_skewY *sts);
static char *style_to_xml(svg_style *st);
static char *transforms_to_xml(svg_transforms *sts);
static char *transform_to_xml(svg_transform *st);
static char *text_length_to_xml(svg_text_length *tl);
static char *length_adjust_type_to_xml(svg_length_adjust_type lat);
static char *spacing_type_to_xml(svg_spacing_type st);
static char *method_type_to_xml(svg_method_type mt);
static char *font_weight_type_to_xml(svg_font_weight_type fw);
static char *font_stretch_type_to_xml(svg_font_stretch_type fs);
static char *font_style_type_to_xml(svg_font_style_type fs);
static char *orient_to_xml(svg_orient *o);
static svg_elements *parse_elements(xml_elements *es);
static svg_element *parse_element(xml_element *e);
static svg_element *parse_rect(xml_element *e);
static svg_element *parse_circle(xml_element *e);
static svg_element *parse_ellipse(xml_element *e);
static svg_element *parse_line(xml_element *e);
static svg_element *parse_polygon(xml_element *e);
static svg_element *parse_polyline(xml_element *e);
static svg_element *parse_path(xml_element *e);
static svg_element *parse_text(xml_element *e);
static svg_element *parse_textpath(xml_element *e);
static svg_element *parse_link(xml_element *e);
static svg_element *parse_image(xml_element *e);
static svg_element *parse_marker(xml_element *e);
static svg_points *parse_points(char *s);
static svg_text_length *parse_text_length(char *s);
static svg_length_adjust_type parse_length_adjust(char *s);
static svg_method_type parse_method(char *s);
static svg_orient *parse_orient(char *s);
static svg_transforms *parse_transforms(char *s);
static svg_transform *parse_transform(char *s, int *len);
static double parse_number(char *s, int *len);
static void eat_whitespace(char **s);
static svg_style *parse_style(char *s);
static int parse_style_bit(svg_style *st, char *b, int *len);
static svg *xml_to_svg(xml *x);

  /*
   * svg
   */

  /**
   * @fn svg *svg_new(void)
   *
   * @brief creates new @a svg struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to new @a svg struct
   */

svg *svg_new(void)
{
  svg *s = (svg *)malloc(sizeof(svg));
  if (s) memset(s, 0, sizeof(svg));
  svg_set_xmlns(s, "http://www.w3.org/2000/svg");
  s->els = svg_elements_new();
  return s;
}

  /**
   * @fn svg *svg_dup(svg *s)
   *
   * @brief copies contents of @p s to new @a svg struct
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return pointer to new @a svg struct
   */

svg *svg_dup(svg *s)
{
  svg *ns;

  if (!s) return NULL;

  ns = svg_new();
  if (!ns) return NULL;

  svg_set_width(ns, s->width);
  svg_set_height(ns, s->height);
  svg_set_xmlns(ns, s->xmlns);
  svg_set_elements(ns, s->els);

  if (s->els) svg_set_elements(ns, s->els);

  return ns;
}

  /**
   * @fn svg *svg_free(svg *s)
   *
   * @brief frees all memory associate with @p s
   *
   * @param s - pointer to existing @a svg struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_free(svg *s)
{
  if (!s) return;
  if (s->els) svg_elements_free(s->els);
  free(s);
}

  /**
   * @fn svg *svg_parse(char *svg_buf)
   *
   * @brief parses text buffer in XML format to SVG document
   *
   * @param svg_buf - string containing SVG document
   *
   * @return pointer to @a svg struct
   * @return NULL on failure
   */

svg *svg_parse(char *svg_buf)
{
  xml *x = NULL;
  xml_element *e = NULL;
  svg_elements *ses = NULL;
  xml_attribute *a = NULL;
  svg *s = NULL;

  if (!svg_buf) goto exit;

  x = xml_parse(svg_buf);
  if (!x) goto exit;

  e = x->root;
  if (!e) goto exit;
  if (!e->name) goto exit;
  if (strcmp(e->name, "svg")) goto exit;

  s = svg_new();
  if (!s) goto exit;

  if ((a = xml_attributes_find(e->attributes, "width")))
    svg_set_width(s, atoi(a->value));
  if ((a = xml_attributes_find(e->attributes, "height")))
    svg_set_height(s, atoi(a->value));
  if ((a = xml_attributes_find(e->attributes, "xmlns")))
    svg_set_xmlns(s, a->value);

  svg_set_elements(s, ses = parse_elements(e->elements));

exit:
  if (ses) svg_elements_free(ses);
  if (x) xml_free(x);

  return s;
}

  /**
   * @fn char *svg_to_string(svg *s)
   *
   * @brief creates a text buffer containing SVG document in XML format
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return string containing SVG document in XML format
   * @return NULL on failure
   */

char *svg_to_string(svg *s)
{
  char *b = NULL;
  xml *x;

  if (!s) goto exit;
  x = svg_to_xml(s);
  if (!x) goto exit;
  b = xml_to_string(x);
  xml_free(x);

exit:
  return b;
}

  /**
   * @fn svg *svg_read(char *filename)
   *
   * @brief reads SVG document from a file
   *
   * @param filename - path to file to read
   *
   * @return pointer to @a svg struct
   * @return NULL on failure
   */

svg *svg_read(char *filename)
{
  svg *s = NULL;
  xml *x = NULL;

  if (!filename) goto exit;

  x = xml_read(filename);
  if (!x) goto exit;

  s = xml_to_svg(x);

exit:
  return s;
}

  /**
   * @fn int svg_write(svg *s, char *filename)
   *
   * @brief writes SVG document to a file
   *
   * @param s - pointer to existing @a svg struct
   * @param filename - path to file to write
   *
   * @return 0 on success
   * @return -1 on failure
   */

int svg_write(svg *s, char *filename)
{
  xml *x;
  int r = -1;

  if (!s) goto exit;

  x = svg_to_xml(s);
  if (!x) goto exit;

  r = xml_write(x, filename);

  xml_free(x);

exit:
  return r;
}

  /**
   * @fn int svg_get_width(svg *s)
   *
   * @brief returns width attribute of @a svg element
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return value of width attribute
   */

int svg_get_width(svg *s) { return s ? s->width : 0.0; }

  /**
   * @fn int svg_set_width(svg *s, int width)
   *
   * @brief sets width attribute of @a svg element
   *
   * @param s - pointer to existing @a svg struct
   * @param width - width of svg element
   *
   * @par Returns
   *   Nothing.
   */

void svg_set_width(svg *s, int width) { if (s) s->width = width; }

  /**
   * @fn int svg_get_height(svg *s)
   *
   * @brief returns height attribute of @a svg element
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return value of height attribute
   */

int svg_get_height(svg *s) { return s ? s->height : 0.0; }

  /**
   * @fn int svg_set_height(svg *s, int height)
   *
   * @brief sets height attribute of svg element
   *
   * @param s - pointer to existing @a svg struct
   * @param height - width of svg element
   *
   * @par Returns
   *   Nothing.
   */

void svg_set_height(svg *s, int height) { if (s) s->height = height; }

  /**
   * @fn char *svg_get_xmlns(svg *s)
   *
   * @brief returns XML namespace attribute of @a svg element
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return XML namespace string
   */

char *svg_get_xmlns(svg *s) { return s ? s->xmlns : NULL; }

  /**
   * @fn void svg_set_xmlns(svg *s, char *xmlns)
   *
   * @brief sets XML namespace attribute of @a svg element
   *
   * @param s - pointer to existing @a svg struct
   * @param xmlns - string containing XML namespace value
   *
   * @par Returns
   *   Nothing.
   */

void svg_set_xmlns(svg *s, char *xmlns)
{
  if (!s || !xmlns) return;
  if (s->xmlns) free(s->xmlns);
  s->xmlns = strdup(xmlns);
}

  /**
   * @fn svg_elements *svg_get_elements(svg *s)
   *
   * @brief returns elements contained in @a svg element
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return pointer to svg_elements struct
   */

svg_elements *svg_get_elements(svg *s) { return s ? s->els : NULL; }

  /**
   * @fn svg_elements *svg_set_elements(svg *s, svg_elements *els)
   *
   * @brief sets elements contained in @b svg element
   *
   * @details replaces contents of @a svg_elements in @p s with a full depth copy
   *          of @p els.  All memory assocatied with the previous @a svg_elements
   *          are free'd.
   *
   * @warning it is the caller's responsibility to manage the memory associated
   *          with @a svg_elements @p els
   *
   * @param s - pointer to existing @a svg struct
   * @param els - pointer to existing svg_elements struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_set_elements(svg *s, svg_elements *els)
{
  if (!s || !els) return;
  if (s->els) svg_elements_free(s->els);
  s->els = svg_elements_dup(els);
}

  /**
   * @fn svg_style *svg_get_style(svg *s)
   *
   * @brief returns style elements containted in @a svg element
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return pointer to @a svg_style struct
   */

svg_style *svg_get_style(svg *s) { return s ? s->style : NULL; }

  /**
   * @fn void svg_set_style(svg *s, svg_style *st)
   *
   * @brief sets style contained in @a svg element
   *
   * @details
   * Replaces contents of @a svg_style in @p s with a full depth copy of @p st.
   * All memory assocatied with the previous @a svg_style is free'd.
   *
   * @warning it is the caller's responsibility to manage the memory associated
   *          with @a svg_elements @p st
   *
   * @param s - pointer to existing @a svg struct
   * @param st - pointer to existing @a svg_style struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_set_style(svg *s, svg_style *st)
{
  if (!s || !st) return;
  if (s->style) svg_style_free(s->style);
  s->style = svg_style_dup(st);
}

  /**
   * @fn xml *svg_to_xml(svg *s)
   *
   * @brief converts @a svg struct to XML text buffer
   *
   * @param s - pointer to existing @a svg struct
   *
   * @return pointer to new @a xml struct on success
   * @return NULL on failure
   */

xml *svg_to_xml(svg *s)
{
  xml *x = NULL;
  xml_element *root = NULL;
  xml_elements *xes = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  char tbuf[256];

  if (!s) goto exit;

  x = xml_new();
  if (!x) goto exit;

  root = xml_element_new();
  if (!root) goto exit;

  xml_element_set_name(root, "svg");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "width");
  sprintf(tbuf, "%d", s->width);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "height");
  sprintf(tbuf, "%d", s->height);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "xmlns");
  xml_attribute_set_value(a, s->xmlns);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  if (s->style && s->style->background_color)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "style");
    sprintf(tbuf, "background-color: %s", s->style->background_color);
    xml_attribute_set_value(a, tbuf);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  xml_element_set_attributes(root, as);

  xes = elements_to_xml(s->els);
  if (xes) xml_element_set_elements(root, xes);

  xml_set_root(x, root);

exit:
  if (root) xml_element_free(root);
  if (xes) xml_elements_free(xes);
  if (as) xml_attributes_free(as);

  return x;
}

  /*
   *  elements
   */

  /**
   * @fn svg_elements *svg_elements_new(void)
   *
   * @brief creates a new @a svg_elements struct
   *
   * @par @b Parameters
   *   None.
   *
   * @return pointer to new @a svg_elements struct
   */

svg_elements *svg_elements_new(void)
{
  svg_elements *ses = (svg_elements *)malloc(sizeof(svg_elements));
  if (ses) memset(ses, 0, sizeof(svg_elements));
  return ses;
}

  /**
   * @fn svg_elements *svg_elements_dup(svg_elements *els)
   *
   * @brief copies contents of @p s to new @a svg_elements struct
   *
   * @param els - pointer to existing @a svg_elements struct
   *
   * @return pointer to new @a svg_elements struct
   */

svg_elements *svg_elements_dup(svg_elements *els)
{
  svg_elements *nels;
  int i;

  if (!els) return NULL;

  nels = svg_elements_new();
  if (!nels) return NULL;

  for (i = 0; i < els->size; i++) svg_elements_add(nels, els->arr[i]);

  return nels;
}

  /**
   * @fn void svg_elements_free(svg_elements *els)
   *
   * @brief frees all memory associated with @a svg_elements struct
   *
   * @param els - pointer to existing @a svg_elements struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_elements_free(svg_elements *els)
{
  int i;
  if (!els) return;
  if (els->arr) for (i = 0; i < els->size; i++) svg_element_free(els->arr[i]);
  free(els);
}

  /**
   * @fn int svg_elements_get_size(svg_elements *els)
   *
   * @brief returns number of @a svg_element structs contained in @p els
   *
   * @param els - pointer to existing @a svg_elements struct
   *
   * @return number of elements contained in @p els
   */

int svg_elements_get_size(svg_elements *els) { return els ? els->size : 0; }

  /**
   * @fn void svg_elements_set_size(svg_elements *els, int size)
   *
   * @brief sets number of @a svg_element structs contained in @p els
   *
   * @warning This function should @b never be called in normal use cases.
   *
   * @param els - pointer to existing @a svg_elements struct
   * @param size - new number of @a svg_element structs contained in @p els
   *
   * @par Returns
   *   Nothing.
   */

void svg_elements_set_size(svg_elements *els, int size) { if (els) els->size = size; }

  /**
   * @fn int svg_elements_get_cursor(svg_elements *els)
   *
   * @brief returns current @a svg_element cursor in @p els
   *
   * The cursor in an @a svg_elements struct is a convenience feature for noting
   * the most recently referenced @ svg_element item
   *
   * @param els - pointer to existing @a svg_elements struct
   *
   * @return index into @ svg_element array in @p els
   */

int svg_elements_get_cursor(svg_elements *els) { return els ? els->cursor : 0; }

  /**
   * @fn void svg_elements_set_cursor(svg_elements *els, int cursor)
   *
   * @brief sets current @a svg_element cursor in @p els
   *
   * @param els - pointer to existing @ svg_elements struct
   * @param cursor - new index value for cursor
   *
   * @par Returns
   *   Nothing.
   */

void svg_elements_set_cursor(svg_elements *els, int cursor) { if (els) els->cursor = cursor; }

  /**
   * @fn void svg_elements_add(svg_elements *els, svg_element *el)
   *
   * @brief add a new @a svg_element to @p els
   *
   * @details Adds a new full depth copy of @p el to @p els.
   *
   * @warning it is the caller's responsibility to manage the memory associated
   *          with @a svg_element @p el
   *
   * @param els - pointer to existing @a svg_elements struct
   * @param el - pointer to existing @a svg_element to add to @p els
   *
   * @par Returns
   *   Nothing.
   */

void svg_elements_add(svg_elements *els, svg_element *el)
{
  if (!els || !el) return;

  if (els->arr) els->arr = (svg_element **)realloc(els->arr, sizeof(svg_element *) * (els->size + 1));
  else els->arr = (svg_element **)malloc(sizeof(svg_element *));

  els->arr[els->size] = svg_element_dup(el);

  ++els->size;
}

  /**
   * @fn void svg_elements_remove(svg_elements *els, int index)
   *
   * @brief remove an @a svg_element from @p els
   *
   * @param els - pointer to existing @a svg_elements struct
   * @param index - array index of @a svg_element to remove
   *
   * @par Returns
   *   Nothing.
   */

void svg_elements_remove(svg_elements *els, int index)
{
  unsigned int i;

  if (!els) return;
  if (!els->size) return;

  if (index < els->size) svg_element_free(els->arr[index]);

  for (i = index; i < (els->size - 1); i++) els->arr[i] = els->arr[i + 1];

  els->arr = (svg_element **)realloc(els->arr, sizeof(svg_element *) * (els->size - 1));
  --els->size;
}

  /*
   *  element
   */

  /**
   * @fn svg_element *svg_element_new(void)
   *
   * @brief creates a new @a svg_element struct
   *
   * @par @b Parameters
   *   None.
   *
   * @return pointer to new @a svg_element struct
   */

svg_element *svg_element_new(void)
{
  svg_element *se = (svg_element *)malloc(sizeof(svg_element));
  if (se) memset(se, 0, sizeof(svg_element));
  return se;
}

  /**
   * @fn svg_element *svg_element_dup(svg_element *el)
   *
   * @brief copies contents of @p s to new @a svg_element struct
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return pointer to new @a svg_element struct
   */

svg_element *svg_element_dup(svg_element *el)
{
  svg_element *nel;

  if (!el) return NULL;

  nel = svg_element_new();
  if (!nel) return NULL;

  svg_element_set_element(nel, el->type, svg_element_get_element(el));
  svg_element_set_id(nel, el->id);
  svg_element_set_style(nel, el->style);
  svg_element_set_transforms(nel, el->transforms);

  return nel;
}

  /**
   * @fn void svg_element_free(svg_element *el)
   *
   * @brief frees all memory associated with @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_free(svg_element *el)
{
  if (!el) return;

  if (el->any) switch (el->type)
  {
    case svg_element_type_rect: svg_rect_free(el->r); break;
    case svg_element_type_circle: svg_circle_free(el->c); break;
    case svg_element_type_ellipse: svg_ellipse_free(el->e); break;
    case svg_element_type_line: svg_line_free(el->l); break;
    case svg_element_type_polygon: svg_polygon_free(el->pg); break;
    case svg_element_type_polyline: svg_polyline_free(el->pl); break;
    case svg_element_type_path: svg_path_free(el->ph); break;
    case svg_element_type_text: svg_text_free(el->t); break;
    case svg_element_type_textpath: svg_textpath_free(el->tp); break;
    case svg_element_type_link: svg_link_free(el->lnk); break;
    case svg_element_type_image: svg_image_free(el->img); break;
    case svg_element_type_marker: svg_marker_free(el->m); break;

    case svg_element_type_none:
    default:
      break;
  }

  if (el->id) free(el->id);

  free(el);
}

  /**
   * @fn svg_element_type svg_element_get_type(svg_element *el)
   *
   * @brief returns SVG element type of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return @a SVG element type
   */

svg_element_type svg_element_get_type(svg_element *el) { return el ? el->type : svg_element_type_none; }

  /**
   * @fn void svg_element_set_type(svg_element *el, svg_element_type type)
   *
   * @brief sets SVG element type of @p el
   *
   * @param el - pointer to exiting @a svg_element struct
   * @param type - @a svg_element_type
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_set_type(svg_element *el, svg_element_type type) { if (el) el->type = type; }

  /**
   * @fn void *svg_element_get_element(svg_element *el)
   *
   * @brief returns pointer to specific SVG element contained in @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return pointer to contained SVG element
   */

void *svg_element_get_element(svg_element *el) { return el ? el->any : NULL; }

  /**
   * @fn void svg_element_set_element(svg_element *el, svg_element_type type, void *ele)
   *
   * @brief sets SVG element contained in @p el
   *
   * @param el - pointer to existing @a svg_element struct
   * @param type - @a svg_element_type
   * @param ele - polymorphic pointer to specific SVG element
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_set_element(svg_element *el, svg_element_type type, void *ele)
{
  if (!el) return;

  if (el->any) switch (el->type)
  {
    case svg_element_type_rect: svg_rect_free(el->r); break;
    case svg_element_type_circle: svg_circle_free(el->c); break;
    case svg_element_type_ellipse: svg_ellipse_free(el->e); break;
    case svg_element_type_line: svg_line_free(el->l); break;
    case svg_element_type_polygon: svg_polygon_free(el->pg); break;
    case svg_element_type_polyline: svg_polyline_free(el->pl); break;
    case svg_element_type_path: svg_path_free(el->ph); break;
    case svg_element_type_text: svg_text_free(el->t); break;
    case svg_element_type_textpath: svg_textpath_free(el->tp); break;
    case svg_element_type_link: svg_link_free(el->lnk); break;
    case svg_element_type_image: svg_image_free(el->img); break;
    case svg_element_type_marker: svg_marker_free(el->m); break;

    case svg_element_type_none:
    default:
      break;
  }

  svg_element_set_type(el, type);

  switch (el->type)
  {
    case svg_element_type_rect: el->any = svg_rect_dup(ele); break;
    case svg_element_type_circle: el->any = svg_circle_dup(ele); break;
    case svg_element_type_ellipse: el->any = svg_ellipse_dup(ele); break;
    case svg_element_type_line: el->any = svg_line_dup(ele); break;
    case svg_element_type_polygon: el->any = svg_polygon_dup(ele); break;
    case svg_element_type_polyline: el->any = svg_polyline_dup(ele); break;
    case svg_element_type_path: el->any = svg_path_dup(ele); break;
    case svg_element_type_text: el->any = svg_text_dup(ele); break;
    case svg_element_type_textpath: el->any = svg_textpath_dup(ele); break;
    case svg_element_type_link: el->any = svg_link_dup(ele); break;
    case svg_element_type_image: el->any = svg_image_dup(ele); break;
    case svg_element_type_marker: el->any = svg_marker_dup(ele); break;

    case svg_element_type_none:
    default:
      el->any = NULL;
      break;
  }
}

  /**
   * @fn char *svg_element_get_id(svg_element *el)
   *
   * @brief returns id attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return id attribute of @p el
   */

char *svg_element_get_id(svg_element *el) { return el ? el->id : NULL; }

  /**
   * @fn void svg_element_set_id(svg_element *el, char *id)
   *
   * @brief sets id attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   * @param id - id attribute string
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_set_id(svg_element *el, char *id)
{
  if (!el || !id) return;
  if (el->id) free(el->id);
  el->id = strdup(id);
}

  /**
   * @fn char *svg_element_get_class(svg_element *el)
   *
   * @brief returns class attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return class attribute of @p el
   */

char *svg_element_get_class(svg_element *el) { return el ? el->class : NULL; }

  /**
   * @fn void svg_element_set_class(svg_element *el, char *class)
   *
   * @brief sets class attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   * @param class - class attribute string
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_set_class(svg_element *el, char *class)
{
  if (!el || !class) return;
  if (el->class) free(el->class);
  el->class = strdup(class);
}

  /**
   * @fn svg_style *svg_element_get_style(svg_element *el)
   *
   * @brief returns style attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return @a svg_style struct pointer
   */

svg_style *svg_element_get_style(svg_element *el) { return el ? el->style : NULL; }

  /**
   * @fn void svg_element_set_style(svg_element *el, svg_style *style)
   *
   * @brief sets style attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   * @param style - pointer to existing @a svg_style struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_set_style(svg_element *el, svg_style *style)
{
  if (!el || !style) return;
  if (el->style) svg_style_free(el->style);
  el->style = svg_style_dup(style);
}

  /**
   * @fn svg_transforms *svg_element_get_transforms(svg_element *el)
   *
   * @brief returns transforms attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   *
   * @return @a svg_transforms struct pointer
   */

svg_transforms *svg_element_get_transforms(svg_element *el) { return el ? el->transforms : NULL; }

  /**
   * @fn void svg_element_set_transforms(svg_element *el, svg_transforms *str)
   *
   * @brief sets transforms attribute of @p el
   *
   * @param el - pointer to existing @a svg_element struct
   * @param str - string containing SVG transforms
   *
   * @par Returns
   *   Nothing.
   */

void svg_element_set_transforms(svg_element *el, svg_transforms *str)
{
  if (!el || !str) return;
  if (el->transforms) svg_transforms_free(el->transforms);
  el->transforms = svg_transforms_dup(str);
}

  /*
   *  style
   */

  /**
   * @fn svg_style *svg_style_new(void)
   *
   * @brief creates a new @a svg_style struct
   *
   * @par @b Parameters
   *   None.
   *
   * @return pointer to new @a svg_style struct
   */

svg_style *svg_style_new(void)
{
  svg_style *st = (svg_style *)malloc(sizeof(svg_style));
  if (st) memset(st, 0, sizeof(svg_style));
  st->fill_opacity = -1;
  st->stroke_width = 1;
  st->stroke_opacity = -1;
  return st;
}

  /**
   * @fn svg_style *svg_style_dup(svg_style *st)
   *
   * @brief copies contents of @p st to new @a svg_style struct
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return pointer to new @a svg_style struct
   */

svg_style *svg_style_dup(svg_style *st)
{
  svg_style *nst;

  if (!st) return NULL;

  nst = svg_style_new();
  if (!nst) return NULL;

  svg_style_set_fill(nst, st->fill);
  svg_style_set_fill_opacity(nst, st->fill_opacity);
  svg_style_set_fill_rule(nst, st->fill_rule);
  svg_style_set_stroke(nst, st->stroke);
  svg_style_set_stroke_width(nst, st->stroke_width);
  svg_style_set_stroke_opacity(nst, st->stroke_opacity);
  svg_style_set_stroke_linecap(nst, st->stroke_linecap);
  svg_style_set_stroke_dash_array(nst, st->stroke_dash_array);
  svg_style_set_stroke_linejoin(nst, st->stroke_linejoin);
  svg_style_set_background_color(nst, st->background_color);
  svg_style_set_font_family(nst, st->font_family);
  svg_style_set_font_weight(nst, st->font_weight);
  svg_style_set_font_stretch(nst, st->font_stretch);
  svg_style_set_font_style(nst, st->font_style);
  svg_style_set_font_size(nst, st->font_size);

  return nst;
}

  /**
   * @fn void svg_style_free(svg_style *st)
   *
   * @brief frees all memory associated with @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_free(svg_style *st)
{
  if (!st) return;
  if (st->fill) free(st->fill);
  if (st->stroke) free(st->stroke);
  if (st->stroke_dash_array) free(st->stroke_dash_array);
  if (st->background_color) free(st->background_color);
  free(st);
}

  /**
   * @fn char *svg_style_get_fill(svg_style *st)
   *
   * @brief returns SVG fill attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return string containing fill attribute of @p st
   */

char *svg_style_get_fill(svg_style *st) { return st ? st->fill : NULL; }

  /**
   * @fn void svg_style_set_fill(svg_style *st, char *fill)
   *
   * @brief sets SVG fill attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param fill - string containing fill attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_fill(svg_style *st, char *fill)
{
  if (!st || !fill) return;
  if (st->fill) free(st->fill);
  st->fill = strdup(fill);
}

  /**
   * @fn double svg_style_get_fill_opacity(svg_style *st)
   *
   * @brief returns fill-opacity attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return value of fill-opacity attribute
   */

double svg_style_get_fill_opacity(svg_style *st) { return st ? st->fill_opacity : 0.0; }

  /**
   * @fn void svg_style_set_fill_opacity(svg_style *st, double fill_opacity)
   *
   * @brief sets fill-opacity value of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param fill_opacity - new value of fill-opacity attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_fill_opacity(svg_style *st, double fill_opacity) { if (st) st->fill_opacity = fill_opacity; }

  /**
   * @fn svg_fill_rule_type svg_style_get_fill_rule(svg_style *st)
   *
   * @brief returns fill-rule attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return @a svg_fill_rule
   */

svg_fill_rule_type svg_style_get_fill_rule(svg_style *st) { return st ? st->fill_rule : svg_fill_rule_type_nonzero; }

  /**
   * @fn void svg_style_set_fill_rule(svg_style *st, svg_fill_rule_type fill_rule)
   *
   * @brief sets fill-rule attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param fill_rule - @a svg_fill_rule
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_fill_rule(svg_style *st, svg_fill_rule_type fill_rule) { if (st) st->fill_rule = fill_rule; }

  /**
   * @fn char *svg_style_get_stroke(svg_style *st)
   *
   * @brief returns stroke attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return string containint stroke attribute
   */

char *svg_style_get_stroke(svg_style *st) { return st ? st->stroke : NULL; }

  /**
   * @fn void svg_style_set_stroke(svg_style *st, char *stroke)
   *
   * @brief sets stroke attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param stroke - string containing stroke attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_stroke(svg_style *st, char *stroke)
{
  if (!st || !stroke) return;
  if (st->stroke) free(st->stroke);
  st->stroke = strdup(stroke);
}

  /**
   * @fn double svg_style_get_stroke_width(svg_style *st)
   *
   * @brief returns stroke-width attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return value of stroke-width attribute
   */

double svg_style_get_stroke_width(svg_style *st) { return st ? st->stroke_width : 0.0; }

  /**
   * @fn void svg_style_set_stroke_width(svg_style *st, double stroke_width)
   *
   * @brief sets stroke-width attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param stroke_width - value of stroke-width attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_stroke_width(svg_style *st, double stroke_width) { if (st) st->stroke_width = stroke_width; }

  /**
   * @fn double svg_style_get_stroke_opacity(svg_style *st)
   *
   * @brief returns stroke-opacity attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return value of stroke-opacity attribute
   */

double svg_style_get_stroke_opacity(svg_style *st) { return st ? st->stroke_opacity : 0.0; }

  /**
   * @fn void svg_style_set_stroke_opacity(svg_style *st, double stroke_opacity)
   *
   * @brief sets stroke-opacity attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param stroke_opacity - value of stroke-opacity attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_stroke_opacity(svg_style *st, double stroke_opacity) { if (st) st->stroke_opacity = stroke_opacity; }

  /**
   * @fn svg_stroke_linecap_type svg_style_get_stroke_linecap(svg_style *st)
   *
   * @brief gets stroke-linecap attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return @a svg_stroke_linecap 
   */

svg_stroke_linecap_type svg_style_get_stroke_linecap(svg_style *st) { return st ? st->stroke_linecap : svg_stroke_linecap_type_butt; }

  /**
   * @fn void svg_style_set_stroke_linecap(svg_style *st, svg_stroke_linecap_type stroke_linecap)
   *
   * @brief sets stroke-linecap attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param stroke_linecap - @a svg_stroke_linecap value
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_stroke_linecap(svg_style *st, svg_stroke_linecap_type stroke_linecap)
{
  if (st) st->stroke_linecap = stroke_linecap;
}

  /**
   * @fn char *svg_style_get_stroke_dash_array(svg_style *st)
   *
   * @brief returns stroke-dash-array attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return string containing stroke-dash-array attribute
   */

char *svg_style_get_stroke_dash_array(svg_style *st) { return st ? st->stroke_dash_array : NULL; }

  /**
   * @fn void svg_style_set_stroke_dash_array(svg_style *st, char *stroke_dash_array)
   *
   * @brief sets stroke-dash-array attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param stroke_dash_array - string containing stroke-dash-array attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_stroke_dash_array(svg_style *st, char *stroke_dash_array)
{
  if (!st || !stroke_dash_array) return;
  if (st->stroke_dash_array) free(st->stroke_dash_array);
  st->stroke_dash_array = strdup(stroke_dash_array);
}

  /**
   * @fn svg_stroke_linejoin_type svg_style_get_stroke_linejoin(svg_style *st)
   *
   * @brief returns stroke-linejoin attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return @a svg_stroke_linejoin_type value of stroke-linejoin attribute
   */

svg_stroke_linejoin_type svg_style_get_stroke_linejoin(svg_style *st)
{
  return st ? st->stroke_linejoin : svg_stroke_linejoin_type_miter;
}

  /**
   * @fn void svg_style_set_stroke_linejoin(svg_style *st, svg_stroke_linejoin_type stroke_linejoin)
   *
   * @brief sets stroke-linejoin attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param stroke_linejoin - @a svg_stroke_linejoin_type value of stroke-linejoin attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_stroke_linejoin(svg_style *st, svg_stroke_linejoin_type stroke_linejoin)
{
  if (st) st->stroke_linejoin = stroke_linejoin;
}

  /**
   * @fn char *svg_style_get_background_color(svg_style *st)
   *
   * @brief returns background-color attribute from @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return string containing background-color attribute
   */
   
char *svg_style_get_background_color(svg_style *st) { return st && st->background_color ? st->background_color : NULL; }

  /**
   * @fn void svg_style_set_background_color(svg_style *st, char *background_color)
   *
   * @brief sets background-color attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param background_color - string containing background-color attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_background_color(svg_style *st, char *background_color)
{
  if (!st || !background_color) return;
  if (st->background_color) free(st->background_color);
  st->background_color = strdup(background_color);
}

  /**
   * @fn char *svg_style_get_font_family(svg_style *st)
   *
   * @brief returns font-family attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return string containing font-family attribute
   */

char *svg_style_get_font_family(svg_style *st) { return st ? st->font_family : NULL; }

  /**
   * @fn void svg_style_set_font_family(svg_style *st, char *font_family)
   *
   * @brief sets font-family attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param font_family - string containing font-family attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_font_family(svg_style *st, char *font_family)
{
  if (!st) return;
  if (st->font_family) free(st->font_family);
  if (font_family) st->font_family = strdup(font_family);
}

  /**
   * @fn svg_font_weight_type svg_style_get_font_weight(svg_style *st)
   *
   * @brief returns font-weight attrib of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return @a svg_font_weight_type value of font-weight attribute
   */

svg_font_weight_type svg_style_get_font_weight(svg_style *st) { return st ? st->font_weight : svg_font_weight_type_normal; }

  /**
   * @fn void svg_style_set_font_weight(svg_style *st, svg_font_weight_type font_weight)
   *
   * @brief sets font-weight attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param font_weight - @a svg_font_weight_type value of font-weight attribute
   *
   * @par Returns
   *   Nothing.
   */
 
void svg_style_set_font_weight(svg_style *st, svg_font_weight_type font_weight) { if (st) st->font_weight = font_weight; } 

  /**
   * @fn svg_font_stretch_type svg_style_get_font_stretch(svg_style *st)
   *
   * @brief returns font-stretch attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return @ svg_font_stretch_type value of font-stretch attribute
   */

svg_font_stretch_type svg_style_get_font_stretch(svg_style *st) { return st ? st->font_stretch : svg_font_stretch_type_normal; }

  /**
   * @fn void svg_style_set_font_stretch(svg_style *st, svg_font_stretch_type font_stretch)
   *
   * @brief sets font-stretch attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param font_stretch - @ svg_font_stretch_type value of font-stretch attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_font_stretch(svg_style *st, svg_font_stretch_type font_stretch) { if (st) st->font_stretch = font_stretch; } 

  /**
   * @fn svg_font_style_type svg_style_get_font_style(svg_style *st)
   *
   * @brief returns font-style attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return @ svg_font_style_type value of font-style attribute
   */

svg_font_style_type svg_style_get_font_style(svg_style *st) { return st ? st->font_style : svg_font_style_type_normal; }

  /**
   * @fn void svg_style_set_font_style(svg_style *st, svg_font_style_type font_style)
   *
   * brief sets font-style attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param font_style - @ svg_font_style_type value of font-style attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_font_style(svg_style *st, svg_font_style_type font_style) { if (st) st->font_style = font_style; } 

  /**
   * @fn char *svg_style_get_font_size(svg_style *st)
   *
   * @brief returns font-size attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   *
   * @return string containing font-size attribute
   */

char *svg_style_get_font_size(svg_style *st) { return st ? st->font_family : NULL; }

  /**
   * @fn void svg_style_set_font_size(svg_style *st, char *font_size)
   *
   * @brief sets font-size attribute of @p st
   *
   * @param st - pointer to existing @a svg_style struct
   * @param font_size - string containing font-size attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_style_set_font_size(svg_style *st, char *font_size)
{
  if (!st) return;
  if (st->font_size) free(st->font_size);
  if (font_size) st->font_size = strdup(font_size);
}

  /*
   *  orient
   */

  /**
   * @fn svg_orient *svg_orient_new(void)
   *
   * @brief creates a new @a svg_orient struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_orient struct
   */

svg_orient *svg_orient_new(void)
{
  svg_orient *o;

  o = (svg_orient *)malloc(sizeof(svg_orient));
  if (o) memset(o, 0, sizeof(svg_orient));

  return o;
}

  /**
   * @fn svg_orient *svg_orient_dup(svg_orient *o)
   *
   * @brief copies contents of @p o to new @a svg_orient struct
   *
   * @param o - pointer to existing @a svg_orient struct
   *
   * @return pointer to new @a svg_orient struct
   */

svg_orient *svg_orient_dup(svg_orient *o)
{
  svg_orient *ntl;

  if (!o) return NULL;

  ntl = svg_orient_new();
  if (!ntl) return NULL;

  svg_orient_set_type(ntl, o->type);
  svg_orient_set_value(ntl, o->val);

  return ntl;
}

  /**
   * @fn void svg_orient_free(svg_orient *o)
   *
   * @brief frees all memory associated with @p o
   *
   * @param o - pointer to existing @a svg_orient struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_orient_free(svg_orient *o) { if (o) free(o); }

  /**
   * @fn svg_orient_type svg_orient_get_type(svg_orient *o)
   *
   * @brief returns @a svg_orient_type attribute of @p o
   *
   * @param o - pointer to existing @a svg_orient struct
   *
   * @return @ svg_orient_type value of orient attribute
   */

svg_orient_type svg_orient_get_type(svg_orient *o) { return o ? o->type : svg_orient_type_auto; }

  /**
   * @fn void svg_orient_set_type(svg_orient *o, svg_orient_type type)
   *
   * @brief sets @ svg_orient_type value of orient@p o
   *
   * @param o - pointer to existing @a svg_orient struct
   * @param type - @a svg_orient_type of orient attribute
   */

void svg_orient_set_type(svg_orient *o, svg_orient_type type) { if (o) o->type = type; }

  /**
   * @fn double svg_orient_get_value(svg_orient *o)
   *
   * @brief returns @a value of @p o
   *
   * @param o - pointer to existing @a svg_orient struct
   *
   * @return value of @ svg_orient
   */

double svg_orient_get_value(svg_orient *o) { return o ? o->val : 0.0; }

  /**
   * @fn void svg_orient_set_value(svg_orient *o, double val)
   *
   * @brief sets  value of @p o
   *
   * @param o - pointer to existing @a svg_orient struct
   * @param val - value of orient type
   *
   * @par Returns
   *   Nothing.
   */

void svg_orient_set_value(svg_orient *o, double val) { if (o) o->val = val; }

  /*
   *  point
   */

  /**
   * @fn svg_point *svg_point_new(void)
   *
   * @brief creates a new @a svg_point struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_point struct
   */

svg_point *svg_point_new(void)
{
  svg_point *sp = (svg_point *)malloc(sizeof(svg_point));
  if (sp) memset(sp, 0, sizeof(svg_point));
  return sp;
}

  /**
   * @fn svg_point *svg_point_new_with_all(double x, double y)
   *
   * @brief creates a new @a svg_point struct with all values
   *
   * @param x - x value
   * @param y - y value
   *
   * @return pointer to @a svg_point struct
   */

svg_point *svg_point_new_with_all(double x, double y)
{
  svg_point *sp = svg_point_new();
  if (!sp) return NULL;
  
  svg_point_set_x(sp, x);
  svg_point_set_y(sp, y);

  return sp;
}

  /**
   * @fn svg_point *svg_point_dup(svg_point *sp)
   *
   * @brief copies contents of @p sp to new @a svg_point struct
   *
   * @param sp - pointer to existing @a svg_point struct
   *
   * @return pointer to new @a svg_point struct
   */

svg_point *svg_point_dup(svg_point *sp)
{
  if (!sp) return NULL;
  return svg_point_new_with_all(sp->x, sp->y);
}

  /**
   * @fn void svg_point_free(svg_point *sp)
   *
   * @brief frees all memory associated with @p sp
   *
   * @param sp - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_point_free(svg_point *sp) { if (sp) free(sp); }

  /**
   * @fn double svg_point_get_x(svg_point *sp)
   *
   * @brief returns x value of @p sp
   *
   * @param sp - pointer to existing @a svg_point struct
   *
   * @return value of x
   */

double svg_point_get_x(svg_point *sp) { return sp ? sp->x : 0; }

  /**
   * @fn void svg_point_set_x(svg_point *sp, double x)
   *
   * @brief sets x value of @p sp
   *
   * @param sp - pointer to existing @a svg_point struct
   * @param x - new value of x
   *
   * @par Returns
   *   Nothing.
   */
  
void svg_point_set_x(svg_point *sp, double x) { if (sp) sp->x = x; }

  /**
   * @fn double svg_point_get_y(svg_point *sp)
   *
   * @brief returns y value of @p sp
   *
   * @param sp - pointer to existing @a svg_point struct
   *
   * @return value of y
   */

double svg_point_get_y(svg_point *sp) { return sp ? sp->y : 0; }

  /**
   * @fn void svg_point_set_y(svg_point *sp, double y)
   *
   * @brief sets y value of @p sp
   *
   * @param sp - pointer to existing @a svg_point struct
   * @param y - new value of y
   *
   * @par Returns
   *   Nothing.
   */
  
void svg_point_set_y(svg_point *sp, double y) { if (sp) sp->y = y; }

  /*
   *  points
   */

  /**
   * @fn svg_points *svg_points_new(void)
   *
   * @brief creates a new @a svg_points struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_points struct
   */

svg_points *svg_points_new(void)
{
  svg_points *sps = (svg_points *)malloc(sizeof(svg_points));
  if (sps) memset(sps, 0, sizeof(svg_points));
  return sps;
}

  /**
   * @fn svg_points *svg_points_dup(svg_points *sps)
   *
   * @brief copies contents of @p sps to new @a svg_points struct
   *
   * @param sps - pointer to existing @a svg_points struct
   *
   * @return pointer to new @a svg_points struct
   */

svg_points *svg_points_dup(svg_points *sps)
{
  svg_points *nsps;
  int i;

  if (!sps) return NULL;

  nsps = svg_points_new();
  if (!nsps) return NULL;

  for (i = 0; i < sps->size; i++) svg_points_add(nsps, sps->arr[i]);

  return nsps;
}

  /**
   * @fn void svg_points_free(svg_points *sps)
   *
   * @brief frees all memory associated with @p sps
   *
   * @param sps - pointer to existing @a svg_points struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_points_free(svg_points *sps)
{
  int i;

  if (!sps) return;

  for (i = 0; i < sps->size; i++) svg_point_free(sps->arr[i]);
  free(sps);
}

  /**
   * @fn void svg_points_add(svg_points *sps, svg_point *sp)
   *
   * @brief add a new @a svg_point to @p sps
   *
   * @details Adds a new full depth copy of @p sp to @p sps.
   *
   * @warning it is the caller's responsibility to manage the memory associated
   *          with @a svg_point @p sp
   *
   * @param sps - pointer to existing @a svg_points struct
   * @param sp - pointer to existing @a svg_point to add to @p sps
   *
   * @par Returns
   *   Nothing.
   */

void svg_points_add(svg_points *sps, svg_point *sp)
{
  if (!sps || !sp) return;

  if (sps->arr) sps->arr = (svg_point **)realloc(sps->arr, sizeof(svg_point *) * (sps->size + 1));
  else sps->arr = (svg_point **)malloc(sizeof(svg_point *));

  sps->arr[sps->size] = svg_point_dup(sp);

  ++sps->size;
}

  /**
   * @fn void svg_points_remove(svg_points *sps, int index)
   *
   * @brief remove an @a svg_point from @p sps
   *
   * @param sps - pointer to existing @a svg_points struct
   * @param index - array index of @a svg_point to remove
   *
   * @par Returns
   *   Nothing.
   */

void svg_points_remove(svg_points *sps, int index)
{
  int i;

  if (!sps) return;
  if (!sps->size) return;
  if (index > sps->size - 1) return;
  if (index < 0) return;

  svg_point_free(sps->arr[index]);
  for (i = index + 1; i < sps->size; i++) sps->arr[i - 1] = sps->arr[i];
  sps->arr = (svg_point **)realloc(sps->arr, sizeof(svg_point *) * (sps->size - 1));
  --sps->size;
}

  /*
   *  text_length
   */

  /**
   * @fn svg_text_length *svg_text_length_new(void)
   *
   * @brief creates a new @a svg_text_length struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_text_length struct
   */

svg_text_length *svg_text_length_new(void)
{
  svg_text_length *tl;

  tl = (svg_text_length *)malloc(sizeof(svg_text_length));
  if (tl) memset(tl, 0, sizeof(svg_text_length));

  return tl;
}

  /**
   * @fn svg_text_length *svg_text_length_dup(svg_text_length *tl)
   *
   * @brief copies contents of @p tl to new @a svg_text_length struct
   *
   * @param tl - pointer to existing @a svg_text_length struct
   *
   * @return pointer to new @a svg_text_length struct
   */

svg_text_length *svg_text_length_dup(svg_text_length *tl)
{
  svg_text_length *ntl;

  if (!tl) return NULL;

  ntl = svg_text_length_new();
  if (!ntl) return NULL;

  svg_text_length_set_type(ntl, tl->type);
  svg_text_length_set_value(ntl, tl->val);

  return ntl;
}

  /**
   * @fn void svg_text_length_free(svg_text_length *tl)
   *
   * @brief frees all memory associated with @p tl
   *
   * @param tl - pointer to existing @a svg_text_length struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_length_free(svg_text_length *tl) { if (tl) free(tl); }

  /**
   * @fn svg_text_length_type svg_text_length_get_type(svg_text_length *tl)
   *
   * @brief returns type attribute of @p tl
   *
   * @param tl - pointer to existing @a svg_text_length struct
   *
   * @return @a svg_text_length_type of @p tl
   */

svg_text_length_type svg_text_length_get_type(svg_text_length *tl) { return tl ? tl->type : svg_text_length_type_none; }

  /**
   * @fn void svg_text_length_set_type(svg_text_length *tl, svg_text_length_type type)
   *
   * @brief sets type attribute of @p tl
   *
   * @param tl - pointer to existing @a svg_text_length struct
   * @param type - @a svg_text_length_type
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_length_set_type(svg_text_length *tl, svg_text_length_type type) { if (tl) tl->type = type; }

  /**
   * @fn double svg_text_length_get_value(svg_text_length *tl)
   *
   * @brief returns value attribute of @p tl
   *
   * @param tl - pointer to existing @a svg_text_length struct
   *
   * @return value of @p tl
   */

double svg_text_length_get_value(svg_text_length *tl) { return tl ? tl->val : 0.0; }

  /**
   * @fn void svg_text_length_set_value(svg_text_length *tl, double value)
   *
   * @brief sets value attribute of @p tl
   *
   * @param tl - pointer to existing @a svg_text_length struct
   * @param value 
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_length_set_value(svg_text_length *tl, double value) { if (tl) tl->val = value; }

  /*
   *  rect
   */

  /**
   * @fn svg_rect *svg_rect_new(void)
   *
   * @brief creates a new @a svg_rect struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_rect struct
   */

svg_rect *svg_rect_new(void)
{
  svg_rect *r = (svg_rect *)malloc(sizeof(svg_rect));
  if (r) memset(r, 0, sizeof(svg_rect));
  return r;
}

  /**
   * @fn svg_rect *svg_rect_dup(svg_rect *r)
   *
   * @brief copies contents of @p r to new @a svg_rect struct
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @return pointer to new @a svg_rect struct
   */

svg_rect *svg_rect_dup(svg_rect *r)
{
  svg_rect *nr;

  if (!r) return NULL;

  nr = svg_rect_new();
  if (!nr) return NULL;

  svg_rect_set_width(nr, r->width);
  svg_rect_set_height(nr, r->height);
  svg_rect_set_point(nr, &r->p);
  svg_rect_set_rx(nr, r->rx);
  svg_rect_set_ry(nr, r->ry);

  return nr;
}

  /**
   * @fn void void svg_rect_free(svg_rect *r)
   *
   * @brief frees all memory associated with @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_rect_free(svg_rect *r) { if (r) free(r); }

  /**
   * @fn double svg_rect_get_width(svg_rect *r)
   *
   * @brief returns width attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @return width value of @p r
   */

double svg_rect_get_width(svg_rect *r) { return r ? r->width : 0.0; }

  /**
   * @fn void svg_rect_set_width(svg_rect *r, double width)
   *
   * @brief sets width attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   * @param width - new width value of @p r
   *
   * @par Returns
   *   Nothing.
   */

void svg_rect_set_width(svg_rect *r, double width) { if (r) r->width = width; }

  /**
   * @fn double svg_rect_get_height(svg_rect *r)
   *
   * @brief returns height attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @return height value of @p r
   */

double svg_rect_get_height(svg_rect *r) { return r ? r->height : 0.0; }

  /**
   * @fn void svg_rect_set_height(svg_rect *r, double height)
   *
   * @brief sets height attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   * @param height - new width value of @p r
   *
   * @par Returns
   *   Nothing.
   */

void svg_rect_set_height(svg_rect *r, double height) { if (r) r->height = height; }

  /**
   * @fn svg_point *svg_rect_get_point(svg_rect *r)
   *
   * @brief returns point attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @return pointer to @a svg_point struct
   */

svg_point *svg_rect_get_point(svg_rect *r) { return r ? &r->p : NULL; }

  /**
   * @fn void svg_rect_set_point(svg_rect *r, svg_point *p)
   *
   * @brief sets point attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   * @param p - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_rect_set_point(svg_rect *r, svg_point *p) { if (r && p) memcpy(&r->p, p, sizeof(svg_point)); }

  /**
   * @fn double svg_rect_get_rx(svg_rect *r)
   *
   * @brief returns rx attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @return value or rx attribute
   */

double svg_rect_get_rx(svg_rect *r) { return r ? r->rx : 0.0; }

  /**
   * @fn void svg_rect_set_rx(svg_rect *r, double rx)
   *
   * @brief sets rx attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   * @param rx - new value of rx attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_rect_set_rx(svg_rect *r, double rx) { if (r) r->rx = rx; }

  /**
   * @fn double svg_rect_get_ry(svg_rect *r)
   *
   * @brief returns ry attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   *
   * @return value or ry attribute
   */

double svg_rect_get_ry(svg_rect *r) { return r ? r->ry : 0.0; }

  /**
   * @fn void svg_rect_set_ry(svg_rect *r, double ry)
   *
   * @brief sets ry attribute of @p r
   *
   * @param r - pointer to existing @a svg_rect struct
   * @param ry - new value of ry attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_rect_set_ry(svg_rect *r, double ry) { if (r) r->ry = ry; }

  /*
   *  circle
   */

  /**
   * @fn svg_circle *svg_circle_new(void)
   *
   * @brief creates a new @a svg_circle struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_circle struct
   */

svg_circle *svg_circle_new(void)
{
  svg_circle *c = (svg_circle *)malloc(sizeof(svg_circle));
  if (c) memset(c, 0, sizeof(svg_circle));
  return c;
}

  /**
   * @fn svg_circle *svg_circle_dup(svg_circle *c)
   *
   * @brief copies contents of @p c to new @a svg_circle struct
   *
   * @param c - pointer to existing @a svg_circle struct
   *
   * @return pointer to new @a svg_circle struct
   */

svg_circle *svg_circle_dup(svg_circle *c)
{
  svg_circle *nc;

  if (!c) return NULL;

  nc = svg_circle_new();
  if (!nc) return NULL;

  svg_circle_set_r(nc, c->r);
  svg_circle_set_c(nc, &c->c);

  return nc;
}

  /**
   * @fn void svg_circle_free(svg_circle *c)
   *
   * @brief frees all memory associated with @p c
   *
   * @param c - pointer to existing @a svg_circle struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_circle_free(svg_circle *c) { if (c) free(c); }

  /**
   * @fn double svg_circle_get_r(svg_circle *c)
   *
   * @brief returns value r attribute of @p c
   *
   * @param c - pointer to existing @a svg_circle struct
   *
   * @return value of r attribute
   */

double svg_circle_get_r(svg_circle *c) { return c ? c->r : 0.0; }

  /**
   * @fn void svg_circle_set_r(svg_circle *c, double r)
   *
   * @brief sets value of r attribute of @p c
   *
   * @param c - pointer to existing @a svg_circle struct
   * @param r - new value of r attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_circle_set_r(svg_circle *c, double r) { if (c) c->r = r; }

  /**
   * @fn svg_point *svg_circle_get_c(svg_circle *c)
   *
   * @brief returns c attribute of @p c
   *
   * @param c - pointer to existing @a svg_circle struct
   *
   * @return pointer to @a svg_circle struct
   */

svg_point *svg_circle_get_c(svg_circle *c) { return c ? &c->c : NULL; }

  /**
   * @fn void svg_circle_set_c(svg_circle *c, svg_point *center)
   *
   * @brief sets c attribute of @p c
   *
   * @param c - pointer to existing @a svg_circle struct
   * @param center - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_circle_set_c(svg_circle *c, svg_point *center) { if (c) memcpy(&c->c, center, sizeof(svg_point)); }

  /*
   *  ellipse
   */

  /**
   * @fn svg_ellipse *svg_ellipse_new(void)
   *
   * @brief creates a new @a svg_ellipse struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_ellipse struct
   */

svg_ellipse *svg_ellipse_new(void)
{
  svg_ellipse *e = (svg_ellipse *)malloc(sizeof(svg_ellipse));
  if (e) memset(e, 0, sizeof(svg_ellipse));
  return e;
}

  /**
   * @fn svg_ellipse *svg_ellipse_dup(svg_ellipse *e)
   *
   * @brief copies contents of @p e to new @a svg_ellipse struct
   *
   * @param e - pointer to existing @a svg_ellipse struct
   *
   * @return pointer to new @a svg_ellipse struct
   */

svg_ellipse *svg_ellipse_dup(svg_ellipse *e)
{
  svg_ellipse *ne;

  if (!e) return NULL;

  ne = svg_ellipse_new();
  if (!ne) return NULL;

  svg_ellipse_set_rx(ne, e->rx);
  svg_ellipse_set_ry(ne, e->ry);
  svg_ellipse_set_c(ne, &e->c);

  return ne;
}

  /**
   * @fn void svg_ellipse_free(svg_ellipse *e)
   *
   * @brief frees all memory associated with @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_ellipse_free(svg_ellipse *e) { if (e) free(e); }

  /**
   * @fn double svg_ellipse_get_rx(svg_ellipse *e)
   *
   * @brief returns rx attribute of @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   *
   * @return value of rx attribute
   */

double svg_ellipse_get_rx(svg_ellipse *e) { return e ? e->rx : 0.0; }

  /**
   * @fn void svg_ellipse_set_rx(svg_ellipse *e, double rx)
   *
   * @brief sets rx attribute of @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   * @param rx - new value of rx attribute
   *
   * @par Returns
   *   Nothing.
   */
  
void svg_ellipse_set_rx(svg_ellipse *e, double rx) { if (e) e->rx = rx; }

  /**
   * @fn double svg_ellipse_get_ry(svg_ellipse *e)
   *
   * @brief returns ry attribute of @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   *
   * @return value of ry attribute
   */

double svg_ellipse_get_ry(svg_ellipse *e) { return e ? e->ry : 0.0; }

  /**
   * @fn void svg_ellipse_set_ry(svg_ellipse *e, double ry)
   *
   * @brief sets ry attribute of @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   * @param ry - new value of rx attribute
   *
   * @par Returns
   *   Nothing.
   */
  
void svg_ellipse_set_ry(svg_ellipse *e, double ry) { if (e) e->ry = ry; }

  /**
   * @fn double svg_ellipse_get_c(svg_ellipse *e)
   *
   * @brief returns c attribute of @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   *
   * @return point to @a svg_point struct
   */

svg_point *svg_ellipse_get_c(svg_ellipse *e) { return e ? &e->c : NULL; }

  /**
   * @fn void svg_ellipse_set_c(svg_ellipse *e, svg_point *c)
   *
   * @brief sets c attribute of @p e
   *
   * @param e - pointer to existing @a svg_ellipse struct
   * @param c - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */
  
void svg_ellipse_set_c(svg_ellipse *e, svg_point *c)
{
  if (!e || !c) return;
  memcpy(&e->c, c, sizeof(svg_point));
}

  /*
   *  line
   */

  /**
   * @fn svg_line *svg_line_new(void)
   *
   * @brief creates a new @a svg_line struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_line struct
   */

svg_line *svg_line_new(void)
{
  svg_line *l = (svg_line *)malloc(sizeof(svg_line));
  if (l) memset(l, 0, sizeof(svg_line));
  return l;
}

  /**
   * @fn svg_line *svg_line_dup(svg_line *l)
   *
   * @brief copies contents of @p l to new @a svg_line struct
   *
   * @param l - pointer to existing @a svg_line struct
   *
   * @return pointer to new @a svg_line struct
   */

svg_line *svg_line_dup(svg_line *l)
{
  svg_line *nl;

  if (!l) return NULL;

  nl = svg_line_new();
  if (!nl) return NULL;

  svg_line_set_p1(nl, &l->p1);
  svg_line_set_p2(nl, &l->p2);

  return nl;
}

  /**
   * @fn void svg_line_free(svg_line *l)
   *
   * @brief frees all memory associated with @p l
   *
   * @param l - pointer to existing @a svg_line struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_line_free(svg_line *l) { if (l) free(l); }

  /**
   * @fn svg_point *svg_line_get_p1(svg_line *l)
   *
   * @brief returns p1 attribute of @p l
   *
   * @param l - pointer to existing @a svg_line struct
   *
   * @return pointer to @ svg_point struct
   */

svg_point *svg_line_get_p1(svg_line *l) { return l ? &l->p1 : NULL; }

  /**
   * @fn void svg_line_set_p1(svg_line *l, svg_point *p1)
   *
   * @brief sets p1 attribute of @p l
   *
   * @param l - pointer to existing @a svg_line struct
   * @param p1 - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_line_set_p1(svg_line *l, svg_point *p1) { if (l) memcpy(&l->p1, p1, sizeof(svg_point)); }

  /**
   * @fn svg_point *svg_line_get_p2(svg_line *l)
   *
   * @brief returns p2 attribute of @p l
   *
   * @param l - pointer to existing @a svg_line struct
   *
   * @return pointer to @ svg_point struct
   */

svg_point *svg_line_get_p2(svg_line *l) { return l ? &l->p2 : NULL; }

  /**
   * @fn void svg_line_set_p2(svg_line *l, svg_point *p2)
   *
   * @brief sets p2 attribute of @p l
   *
   * @param l - pointer to existing @a svg_line struct
   * @param p2 - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_line_set_p2(svg_line *l, svg_point *p2) { if (l) memcpy(&l->p2, p2, sizeof(svg_point)); }

  /*
   *  polygon
   */

  /**
   * @fn svg_polygon *svg_polygon_new(void)
   *
   * @brief creates a new @a svg_polygon struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_polygon struct
   */

svg_polygon *svg_polygon_new(void)
{
  svg_polygon *pg = (svg_polygon *)malloc(sizeof(svg_polygon));
  if (pg) memset(pg, 0, sizeof(svg_polygon));
  return pg;
}

  /**
   * @fn svg_polygon *svg_polygon_dup(svg_polygon *pg)
   *
   * @brief copies contents of @p pg to new @a svg_polygon struct
   *
   * @param pg - pointer to existing @a svg_polygon struct
   *
   * @return pointer to new @a svg_polygon struct
   */

svg_polygon *svg_polygon_dup(svg_polygon *pg)
{
  svg_polygon *npg;

  if (!pg) return NULL;

  npg = svg_polygon_new();
  if (!npg) return NULL;

  svg_polygon_set_points(npg, pg->pts);

  return npg;
}

  /**
   * @fn void svg_polygon_free(svg_polygon *pg)
   *
   * @brief frees all memory associated with @p pg
   *
   * @param pg - pointer to existing @a svg_polygon struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_polygon_free(svg_polygon *pg)
{
  if (!pg) return;
  if (pg->pts) svg_points_free(pg->pts);
  free(pg);
}

  /**
   * @fn svg_points *svg_polygon_get_points(svg_polygon *pg)
   *
   * @brief returns points attribute of @p pg
   *
   * @param pg - pointer to existing @a svg_polygon struct
   *
   * @return pointer to @a svg_points struct
   */

svg_points *svg_polygon_get_points(svg_polygon *pg) { return pg ? pg->pts : NULL; }

  /**
   * @fn void svg_polygon_set_points(svg_polygon *pg, svg_points *pts)
   *
   * @brief sets points attribute of @p pg
   *
   * @param pg - pointer to existing @a svg_polygon struct
   * @param pts - pointer to existing @a svg_points struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_polygon_set_points(svg_polygon *pg, svg_points *pts)
{
  if (!pg || !pts) return;
  if (pg->pts) svg_points_free(pg->pts);
  pg->pts = svg_points_dup(pts);
}

  /*
   *  polyline
   */

  /**
   * @fn svg_polyline *svg_polyline_new(void)
   *
   * @brief creates a new @a svg_polyline struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_polyline struct
   */

svg_polyline *svg_polyline_new(void)
{
  svg_polyline *pl = (svg_polyline *)malloc(sizeof(svg_polyline));
  if (pl) memset(pl, 0, sizeof(svg_polyline));
  return pl;
}

  /**
   * @fn svg_polyline *svg_polyline_dup(svg_polyline *pl)
   *
   * @brief copies contents of @p pl to new @a svg_polyline struct
   *
   * @param pl - pointer to existing @a svg_polyline struct
   *
   * @return pointer to new @a svg_polyline struct
   */

svg_polyline *svg_polyline_dup(svg_polyline *pl)
{
  svg_polyline *npl;

  if (!pl) return NULL;

  npl = svg_polyline_new();
  if (!npl) return NULL;

  svg_polyline_set_points(npl, pl->pts);

  return npl;
}

  /**
   * @fn void svg_polyline_free(svg_polyline *pl)
   *
   * @brief frees all memory associated with @p pl
   *
   * @param pl - pointer to existing @a svg_polyline struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_polyline_free(svg_polyline *pl)
{
  if (!pl) return;
  if (pl->pts) svg_points_free(pl->pts);
  free(pl);
}

  /**
   * @fn svg_points *svg_polyline_get_points(svg_polyline *pl)
   *
   * @brief returns points attribute of @p pl
   *
   * @param pl - pointer to existing @a svg_polyline struct
   *
   * @return pointer to @a svg_points struct
   */

svg_points *svg_polyline_get_points(svg_polyline *pl) { return pl ? pl->pts : NULL; }

  /**
   * @fn void svg_polyline_set_points(svg_polyline *pl, svg_points *pts)
   *
   * @brief sets points attribute of @p pl
   *
   * @param pl - pointer to existing @a svg_polyline struct
   * @param pts - pointer to existing @a svg_points struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_polyline_set_points(svg_polyline *pl, svg_points *pts)
{
  if (!pl || !pts) return;
  if (pl->pts) svg_points_free(pl->pts);
  pl->pts = svg_points_dup(pts);
}

  /*
   *  path
   */

  /**
   * @fn svg_path *svg_path_new(void)
   *
   * @brief creates a new @a svg_path struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_path struct
   */

svg_path *svg_path_new(void)
{
  svg_path *ph = (svg_path *)malloc(sizeof(svg_path));
  if (ph) memset(ph, 0, sizeof(svg_path));
  return ph;
}

  /**
   * @fn svg_path *svg_path_dup(svg_path *ph)
   *
   * @brief copies contents of @p ph to new @a svg_path struct
   *
   * @param ph - pointer to existing @a svg_path struct
   *
   * @return pointer to new @a svg_path struct
   */

svg_path *svg_path_dup(svg_path *ph)
{
  svg_path *nph;

  if (!ph) return NULL;

  nph = svg_path_new();
  if (!nph) return NULL;

  svg_path_set_d(nph, ph->d);

  return nph;
}

  /**
   * @fn void svg_path_free(svg_path *ph)
   *
   * @brief frees all memory associated with @p ph
   *
   * @param ph - pointer to existing @a svg_path struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_path_free(svg_path *ph)
{
  if (!ph) return;
  if (ph->d) free(ph->d);
  free(ph);
}

  /**
   * @fn char *svg_path_get_d(svg_path *ph)
   *
   * @brief returns d attribute of @p ph
   *
   * @param ph - pointer to existing @a svg_path struct
   *
   * @return string containing d attribute value
   */

char *svg_path_get_d(svg_path *ph) { return ph ? ph->d : NULL; }

  /**
   * @fn void svg_path_set_d(svg_path *ph, char *d)
   *
   * @brief sets d attribute of @p ph
   *
   * @param ph - pointer to existing @a svg_path struct
   * @param d - string containing new value for d attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_path_set_d(svg_path *ph, char *d)
{
  if (!ph || !d) return;
  if (ph->d) free(ph->d);
  ph->d = strdup(d);
}

  /*
   *  text
   */

  /**
   * @fn svg_text *svg_text_new(void)
   *
   * @brief creates a new @a svg_text struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_text struct
   */

svg_text *svg_text_new(void)
{
  svg_text *t = (svg_text *)malloc(sizeof(svg_text));
  if (t) memset(t, 0, sizeof(svg_text));
  return t;
}

  /**
   * @fn svg_text *svg_text_dup(svg_text *t)
   *
   * @brief copies contents of @p t to new @a svg_text struct
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return pointer to new @a svg_text struct
   */

svg_text *svg_text_dup(svg_text *t)
{
  svg_text *nt;

  if (!t) return NULL;

  nt = svg_text_new();
  if (!nt) return NULL;

  svg_text_set_p(nt, &t->p);
  svg_text_set_dx(nt, t->dx);
  svg_text_set_dy(nt, t->dy);
  svg_text_set_rotate(nt, t->rotate);
  svg_text_set_text_length(nt, &t->text_length);
  svg_text_set_length_adjust(nt, t->length_adjust);
  svg_text_set_contents(nt, t->contents);

  return nt;
}

  /**
   * @fn void svg_text_free(svg_text *t)
   *
   * @brief frees all memory associated with @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_free(svg_text *t)
{
  if (!t) return;
  if (t->contents) free(t->contents);
  free(t);
}

  /**
   * @fn svg_point *svg_text_get_p(svg_text *t)
   *
   * @brief returns p attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return pointer to @a svg_point struct
   */
   
svg_point *svg_text_get_p(svg_text *t) { return t ? &t->p : NULL; }

  /**
   * @fn void svg_text_set_p(svg_text *t, svg_point *p)
   *
   * @brief sets p attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param p - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_p(svg_text *t, svg_point *p) { if (t) memcpy(&t->p, p, sizeof(svg_point)); }

  /**
   * @fn double svg_text_get_dx(svg_text *t)
   *
   * @brief returns dx attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return value of dx attribute
   */
  
double svg_text_get_dx(svg_text *t) { return t ? t->dx : 0.0; }

  /**
   * @fn void svg_text_set_dx(svg_text *t, double dx)
   *
   * @brief sets dx attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param dx - new value of dx attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_dx(svg_text *t, double dx) { if (t) t->dx = dx; }

  /**
   * @fn double svg_text_get_dy(svg_text *t)
   *
   * @brief returns dy attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return value of dy attribute
   */
  
double svg_text_get_dy(svg_text *t) { return t ? t->dy : 0.0; }

  /**
   * @fn void svg_text_set_dy(svg_text *t, double dy)
   *
   * @brief sets dy attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param dy - new value of dx attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_dy(svg_text *t, double dy) { if (t) t->dy = dy; }

  /**
   * @fn double svg_text_get_rotate(svg_text *t)
   *
   * @brief returns rotate attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return value of rotate attribute
   */
  
double svg_text_get_rotate(svg_text *t) { return t ? t->rotate : 0.0; }

  /**
   * @fn void svg_text_set_rotate(svg_text *t, double rotate)
   *
   * @brief sets rotate attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param rotate - new value of dx attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_rotate(svg_text *t, double rotate) { if (t) t->rotate = rotate; }

  /**
   * @fn double svg_text_get_text_length(svg_text *t)
   *
   * @brief returns text-length attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return pointer to @a svg_text_length struct
   */
  
svg_text_length *svg_text_get_text_length(svg_text *t) { return t ? &t->text_length : NULL; }

  /**
   * @fn void svg_text_set_text_length(svg_text *t, svg_text_length *tl)
   *
   * @brief sets text-length attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param tl - pointer to existing @a svt_text_length struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_text_length(svg_text *t, svg_text_length *tl)
{
  if (!t || !tl) return;
  memcpy(&t->text_length, tl, sizeof(svg_text_length));
}

  /**
   * @fn svg_length_adjust_type svg_text_get_length_adjust(svg_text *t)
   *
   * @brief returns length-adjust attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return @a svg_length_adjust_type value of lenght-adjust attribute
   */

svg_length_adjust_type svg_text_get_length_adjust(svg_text *t) { return t ? t->length_adjust : svg_length_adjust_type_spacing; }

  /**
   * @fn void svg_text_set_length_adjust(svg_text *t, svg_length_adjust_type la) { if (t) t->length_adjust = la; }
   *
   * @brief sets length-adjust attribute of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param la - new @ svg_length_adjust_type value of length-adjust attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_length_adjust(svg_text *t, svg_length_adjust_type la) { if (t) t->length_adjust = la; }

  /**
   * @fn char *svg_text_get_contents(svg_text *t)
   *
   * @brief returns text contents of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   *
   * @return string containing text contents
   */

char *svg_text_get_contents(svg_text *t) { return t ? t->contents : NULL; }

  /**
   * @fn void svg_text_set_contents(svg_text *t, char *contents)
   *
   * @brief sets text contents of @p t
   *
   * @param t - pointer to existing @a svg_text struct
   * @param contents - string containing new text contents
   *
   * @par Returns
   *   Nothing.
   */

void svg_text_set_contents(svg_text *t, char *contents)
{
  if (!t || !contents) return;
  if (t->contents) free(t->contents);
  t->contents = strdup(contents);
}

  /*
   *  textpath
   */

  /**
   * @fn svg_textpath *svg_textpath_new(void)
   *
   * @brief creates a new @a svg_textpath struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_textpath struct
   */

svg_textpath *svg_textpath_new(void)
{
  svg_textpath *tp = (svg_textpath *)malloc(sizeof(svg_textpath));
  if (tp) memset(tp, 0, sizeof(svg_textpath));
  return tp;
}

  /**
   * @fn svg_textpath *svg_textpath_dup(svg_textpath *tp)
   *
   * @brief copies contents of @p tp to new @a svg_textpath struct
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return pointer to new @a svg_textpath struct
   */

svg_textpath *svg_textpath_dup(svg_textpath *tp)
{
  svg_textpath *ntp;

  if (!tp) return NULL;

  ntp = svg_textpath_new();
  if (!ntp) return NULL;

  svg_textpath_set_href(ntp, tp->href);
  svg_textpath_set_length_adjust(ntp, tp->length_adjust);
  svg_textpath_set_method(ntp, tp->method);
  svg_textpath_set_spacing(ntp, tp->spacing);
  svg_textpath_set_start_offset(ntp, &tp->start_offset);
  svg_textpath_set_text_length(ntp, &tp->text_length);

  return ntp;
}

  /**
   * @fn void svg_textpath_free(svg_textpath *tp)
   *
   * @brief frees all memory associated with @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_free(svg_textpath *tp)
{
  if (!tp) return;
  if (tp->href) free(tp->href);
  if (tp->contents) free(tp->contents);
  free(tp);
}

  /**
   * @fn char *svg_textpath_get_href(svg_textpath *tp)
   *
   * @brief returns href attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return string containing value of href attribute
   */
  
char *svg_textpath_get_href(svg_textpath *tp) { return tp ? tp->href : NULL; }

  /**
   * @fn void svg_textpath_set_href(svg_textpath *tp, char *href)
   *
   * @brief sets href attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param href - string containing new value of href attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_set_href(svg_textpath *tp, char *href)
{
  if (!tp || !href) return;
  if (tp->href) free(tp->href);
  tp->href = strdup(href);
}

  /**
   * @fn svg_length_adjust_type svg_textpath_get_length_adjust(svg_textpath *tp)
   *
   * @brief returns length-adjust attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return @a svg_length_adjust_type value of length-adjust attribute
   */

svg_length_adjust_type svg_textpath_get_length_adjust(svg_textpath *tp)
{
  return tp ? tp->length_adjust : svg_length_adjust_type_spacing;
}

  /**
   * @fn void svg_textpath_set_length_adjust(svg_textpath *tp, svg_length_adjust_type la)
   *
   * @brief sets length-adjust attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param la - @a svg_length_adjust_type value
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_set_length_adjust(svg_textpath *tp, svg_length_adjust_type la) { if (tp) tp->length_adjust = la; }

  /**
   * @fn svg_method_type svg_textpath_get_method(svg_textpath *tp)
   *
   * @brief returns method attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return @a svg_method_type value of method attribute
   */

svg_method_type svg_textpath_get_method(svg_textpath *tp) { return tp ? tp->method : svg_method_type_align; }

  /**
   * @fn void svg_textpath_set_method(svg_textpath *tp, svg_method_type m)
   *
   * @brief sets method attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param m - @a svg_method_type value of method attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_set_method(svg_textpath *tp, svg_method_type m) { if (tp) tp->method = m; }

  /**
   * @fn svg_spacing_type svg_textpath_get_spacing(svg_textpath *tp)
   *
   * @brief returns spacing attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return @a svg_spacing_type value of spacing attribute
   */

svg_spacing_type svg_textpath_get_spacing(svg_textpath *tp) { return tp ? tp->spacing : svg_spacing_type_auto; }

  /**
   * @fn void svg_textpath_set_spacing(svg_textpath *tp, svg_spacing_type s)
   *
   * @brief sets spacing attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param s - @a svg_spacing_type value of spacing attribute
   *
   * @par Returns
   *   Nothing.
   */
  
void svg_textpath_set_spacing(svg_textpath *tp, svg_spacing_type s) { if (tp) tp->spacing = s; }

  /**
   * @fn svg_text_length *svg_textpath_get_start_offset(svg_textpath *tp)
   *
   * @brief returns start-offset attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return pointer to @a svg_text_length struct
   */

svg_text_length *svg_textpath_get_start_offset(svg_textpath *tp) { return tp ? &tp->start_offset : NULL; }

  /**
   * @fn void svg_textpath_set_start_offset(svg_textpath *tp, svg_text_length *so)
   *
   * @brief set text-length attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param so - pointer to existing @a svg_text_length struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_set_start_offset(svg_textpath *tp, svg_text_length *so)
{
  if (!tp || !so) return;
  memcpy(&tp->start_offset, so, sizeof(svg_text_length));
}

  /**
   * @fn svg_text_length *svg_textpath_get_text_length(svg_textpath *tp)
   *
   * @brief returns text-length attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return pointer to @a svg_text_length struct
   */

svg_text_length *svg_textpath_get_text_length(svg_textpath *tp) { return tp ? &tp->text_length : NULL; }

  /**
   * @fn void svg_textpath_set_text_length(svg_textpath *tp, svg_text_length *tl)
   *
   * @brief returns text-length attribute of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param tl - pointer to existing @a svg_text_length struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_set_text_length(svg_textpath *tp, svg_text_length *tl)
{
  if (!tp || !tl) return;
  memcpy(&tp->text_length, tl, sizeof(svg_text_length));
}

  /**
   * @fn char *svg_textpath_get_contents(svg_textpath *tp)
   *
   * @brief returns text contents of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   *
   * @return string containing text contents
   */

char *svg_textpath_get_contents(svg_textpath *tp) { return tp ? tp->contents : NULL; }

  /**
   * @fn void svg_textpath_set_contents(svg_textpath *tp, char *contents)
   *
   * @brief sets text contents of @p tp
   *
   * @param tp - pointer to existing @a svg_textpath struct
   * @param contents - string containing new text contents
   *
   * @par Returns
   *   Nothing.
   */

void svg_textpath_set_contents(svg_textpath *tp, char *contents)
{
  if (!tp || !contents) return;
  if (tp->contents) free(tp->contents);
  tp->contents = strdup(contents);
}

  /*
   *  link
   */

  /**
   * @fn svg_link *svg_link_new(void)
   *
   * @brief creates a new @a svg_link struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_link struct
   */

svg_link *svg_link_new(void)
{
  svg_link *lnk = (svg_link *)malloc(sizeof(svg_link));
  if (lnk) memset(lnk, 0, sizeof(svg_link));
  return lnk;
}

  /**
   * @fn svg_link *svg_link_dup(svg_link *lnk)
   *
   * @brief copies contents of @p lnk to new @a svg_link struct
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return pointer to new @a svg_link struct
   */

svg_link *svg_link_dup(svg_link *lnk)
{
  svg_link *nlnk;

  if (!lnk) return NULL;

  nlnk = svg_link_new();
  if (!nlnk) return NULL;

  svg_link_set_href(nlnk, lnk->href);
  svg_link_set_download(nlnk, lnk->download);
  svg_link_set_hreflang(nlnk, lnk->hreflang);
  svg_link_set_referrer_policy(nlnk, lnk->referrer_policy);
  svg_link_set_rel(nlnk, lnk->rel);
  svg_link_set_target(nlnk, lnk->target);
  svg_link_set_type(nlnk, lnk->type);

  return nlnk;
}

  /**
   * @fn void svg_link_free(svg_link *lnk)
   *
   * @brief frees all memory associated with @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_free(svg_link *lnk)
{
  if (!lnk) return;
  if (lnk->href) free(lnk->href);
  if (lnk->download) free(lnk->download);
  if (lnk->hreflang) free(lnk->hreflang);
  if (lnk->referrer_policy) free(lnk->referrer_policy);
  if (lnk->rel) free(lnk->rel);
  if (lnk->target) free(lnk->target);
  if (lnk->type) free(lnk->type);
  if (lnk->els) svg_elements_free(lnk->els);

  free(lnk);
}

  /**
   * @fn char *svg_link_get_href(svg_link *lnk)
   *
   * @brief returns href attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return string containing value of href attribute
   */

char *svg_link_get_href(svg_link *lnk) { return lnk ? lnk->href : NULL; }

  /**
   * @fn void svg_link_set_href(svg_link *lnk, char *href)
   *
   * @brief sets href attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param href - string containing new value of href attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_href(svg_link *lnk, char *href)
{
  if (!lnk || !href) return;
  if (lnk->href) free(lnk->href);
  lnk->href = strdup(href);
}

  /**
   * @fn char *svg_link_get_download(svg_link *lnk)
   *
   * @brief returns download attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return string containing value of download attribute
   */

char *svg_link_get_download(svg_link *lnk) { return lnk ? lnk->download : NULL; }

  /**
   * @fn void svg_link_set_download(svg_link *lnk, char *download)
   *
   * @brief sets download attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param download - string containing new value of download attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_download(svg_link *lnk, char *download)
{
  if (!lnk) return;
  if (lnk->download) free(lnk->download);
  lnk->download = NULL;
  if (download) lnk->download = strdup(download);
}

  /**
   * @fn char *svg_link_get_hreflang(svg_link *lnk)
   *
   * @brief returns hreflang attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return string containing value of hreflang attribute
   */
   
char *svg_link_get_hreflang(svg_link *lnk) { return lnk ? lnk->hreflang : NULL; }

  /**
   * @fn void svg_link_set_hreflang(svg_link *lnk, char *hreflang)
   *
   * @brief sets hreflang attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param hreflang - string containing new value of hreflang attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_hreflang(svg_link *lnk, char *hreflang)
{
  if (!lnk || !hreflang) return;
  if (lnk->hreflang) free(lnk->hreflang);
  lnk->hreflang = strdup(hreflang);
}

  /**
   * @fn char *svg_link_get_referrer_policy(svg_link *lnk)
   *
   * @brief returns referrrer-policy attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * 
   * @return string containing value of referrer-policy attribute
   */

char *svg_link_get_referrer_policy(svg_link *lnk) { return lnk ? lnk->referrer_policy : NULL; }

  /**
   * @fn void svg_link_set_referrer_policy(svg_link *lnk, char *referrer_policy)
   *
   * @brief sets referrrer-policy attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param referrer_policy - string containing new value of referrer-policy attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_referrer_policy(svg_link *lnk, char *referrer_policy)
{
  if (!lnk || !referrer_policy) return;
  if (lnk->referrer_policy) free(lnk->referrer_policy);
  lnk->referrer_policy = strdup(referrer_policy);
}

  /**
   * @fn char *svg_link_get_rel(svg_link *lnk)
   *
   * @brief returns rel attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return string containing value of rel attribute
   */

char *svg_link_get_rel(svg_link *lnk) { return lnk ? lnk->rel : NULL; }

  /**
   * @fn void svg_link_set_rel(svg_link *lnk, char *rel)
   *
   * @brief sets rel attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param rel - string containing new value of rel attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_rel(svg_link *lnk, char *rel)
{
  if (!lnk || !rel) return;
  if (lnk->rel) free(lnk->rel);
  lnk->rel = strdup(rel);
}

  /**
   * @fn char *svg_link_get_target(svg_link *lnk)
   *
   * @brief returns target attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return string containing value of target attribute
   */

char *svg_link_get_target(svg_link *lnk) { return lnk ? lnk->target : NULL; }

  /**
   * @fn void svg_link_set_target(svg_link *lnk, char *target)
   *
   * @brief sets target attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param target - string containing new value of target attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_target(svg_link *lnk, char *target)
{
  if (!lnk || !target) return;
  if (lnk->target) free(lnk->target);
  lnk->target = strdup(target);
}

  /**
   * @fn char *svg_link_get_type(svg_link *lnk)
   *
   * @brief returns type attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return string containing value of type attribute
   */

char *svg_link_get_type(svg_link *lnk) { return lnk ? lnk->type : NULL; }

  /**
   * @fn void svg_link_set_type(svg_link *lnk, char *type)
   *
   * @brief sets type attribute of @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param type - string containing new value of type attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_type(svg_link *lnk, char *type)
{
  if (!lnk || !type) return;
  if (lnk->type) free(lnk->type);
  lnk->type = strdup(type);
}

  /**
   * @fn svg_elements *svg_link_get_elements(svg_link *lnk)
   *
   * @brief returns returns elements contained in @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   *
   * @return pointer to @a svg_elements struct
   */

svg_elements *svg_link_get_elements(svg_link *lnk) { return lnk ? lnk->els : NULL; }

  /**
   * @fn void svg_link_set_elements(svg_link *lnk, svg_elements *ses)
   *
   * @brief sets elements contained in @p lnk
   *
   * @param lnk - pointer to existing @a svg_link struct
   * @param ses - pointer to existing @a svg_elements struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_link_set_elements(svg_link *lnk, svg_elements *ses)
{
  if (!lnk) return;
  if (lnk->els) svg_elements_free(lnk->els);
  lnk->els = NULL;
  lnk->els = svg_elements_dup(ses);
}

  /*
   *  image
   */

  /**
   * @fn svg_image *svg_image_new(void)
   *
   * @brief creates a new @a svg_image struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_image struct
   */

svg_image *svg_image_new(void)
{
  svg_image *img = (svg_image *)malloc(sizeof(svg_image));
  if (img) memset(img, 0, sizeof(svg_image));
  return img;
}

  /**
   * @fn svg_image *svg_image_dup(svg_image *img)
   *
   * @brief copies contents of @p img to new @a svg_image struct
   *
   * @param img - pointer to existing @a svg_image struct
   *
   * @return pointer to new @a svg_image struct
   */

svg_image *svg_image_dup(svg_image *img)
{
  svg_image *nimg;

  if (!img) return NULL;

  nimg = svg_image_new();
  if (nimg) return NULL;

  svg_image_set_width(nimg, img->width);
  svg_image_set_height(nimg, img->height);
  svg_image_set_href(nimg, img->href);
  svg_image_set_p(nimg, &img->p);

  return nimg;
}

  /**
   * @fn void svg_image_free(svg_image *img)
   *
   * @brief frees all memory associated with @p img
   *
   * @param img - pointer to existing @a svg_image struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_image_free(svg_image *img)
{
  if (!img) return;
  if (img->href) free(img->href);
  free(img);
}

  /**
   * @fn double svg_image_get_width(svg_image *img)
   *
   * @brief returns width attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   *
   * @return value of width attribute
   */

double svg_image_get_width(svg_image *img) { return img ? img->width : 0.0; }

  /**
   * @fn void svg_image_set_width(svg_image *img, double width)
   *
   * @brief sets width attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   * @param width - new value of width attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_image_set_width(svg_image *img, double width) { if (img) img->width = width; }

  /**
   * @fn double svg_image_get_height(svg_image *img)
   *
   * @brief returns height attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   *
   * @return value of height attribute
   */

double svg_image_get_height(svg_image *img) { return img ? img->height : 0.0; }

  /**
   * @fn void svg_image_set_height(svg_image *img, double height)
   *
   * @brief sets height attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   * @param height - new value of width attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_image_set_height(svg_image *img, double height) { if (img) img->height = height; }

  /**
   * @fn char *svg_image_get_href(svg_image *img)
   *
   * @brief returns href attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   *
   * @return string containing value of href attribute
   */

char *svg_image_get_href(svg_image *img) { return img ? img->href : NULL; }

  /**
   * @fn void svg_image_set_href(svg_image *img, char *href)
   *
   * @brief sets href attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   * @param href - string containing new value of href attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_image_set_href(svg_image *img, char *href)
{
  if (!img || !href) return;
  if (img->href) free(img->href);
  img->href = strdup(href);
}

  /**
   * @fn svg_point *svg_image_get_p(svg_image *img)
   *
   * @brief returns p attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   *
   * @return pointer to @a svg_point struct
   */

svg_point *svg_image_get_p(svg_image *img) { return img ? &img->p : NULL; }

  /**
   * @fn void svg_image_set_p(svg_image *img, svg_point *p)
   *
   * @brief sets p attribute of @p img
   *
   * @param img - pointer to existing @a svg_image struct
   * @param p - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_image_set_p(svg_image *img, svg_point *p)
{
  if (!img || !p) return;
  memcpy(&img->p, p, sizeof(svg_point));
}

  /*
   *  marker
   */

  /**
   * @fn svg_marker *svg_marker_new(void)
   *
   * @brief creates a new @a svg_marker struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_marker struct
   */

svg_marker *svg_marker_new(void)
{
  svg_marker *m = (svg_marker *)malloc(sizeof(svg_marker));
  if (m) memset(m, 0, sizeof(svg_marker));
  return m;
}

  /**
   * @fn svg_marker *svg_marker_dup(svg_marker *m)
   *
   * @brief copies contents of @p m to new @a svg_marker struct
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @return pointer to new @a svg_marker struct
   */

svg_marker *svg_marker_dup(svg_marker *m)
{
  svg_marker *nm;

  if (!m) return NULL;

  nm = svg_marker_new();
  if (!nm) return NULL;

  svg_marker_set_marker_height(nm, m->marker_height);
  svg_marker_set_marker_width(nm, m->marker_width);
  svg_marker_set_ref(nm, &m->ref);
  svg_marker_set_orient(nm, m->orient);

  return nm;
}

  /**
   * @fn void svg_marker_free(svg_marker *m)
   *
   * @brief frees all memory associated with @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_marker_free(svg_marker *m)
{
  if (m->els) svg_elements_free(m->els);
  if (m) free(m);
}

  /**
   * @fn double svg_marker_get_marker_height(svg_marker *m)
   *
   * @brief returns marker-height attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @return value of marker-height attribute
   */

double svg_marker_get_marker_height(svg_marker *m) { return m ? m->marker_height : 0.0; }

  /**
   * @fn void svg_marker_set_marker_height(svg_marker *m, double marker_height)
   *
   * @brief sets marker-height attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   * @param marker_height - new value of marker-height attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_marker_set_marker_height(svg_marker *m, double marker_height) { if (m) m->marker_height = marker_height; }

  /**
   * @fn double svg_marker_get_marker_width(svg_marker *m)
   *
   * @brief returns marker-width attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @return value of marker-width attribute
   */

double svg_marker_get_marker_width(svg_marker *m) { return m ? m->marker_width : 0.0; }

  /**
   * @fn void svg_marker_set_marker_width(svg_marker *m, double marker_width)
   *
   * @brief sets marker-width attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   * @param marker_width - new value of marker-width attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_marker_set_marker_width(svg_marker *m, double marker_width) { if (m) m->marker_width = marker_width; }

  /**
   * @fn svg_point *svg_marker_get_ref(svg_marker *m)
   *
   * @brief returns ref attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @return pointer to @a svg_point struct
   */

svg_point *svg_marker_get_ref(svg_marker *m) { return m ? &m->ref : NULL; }

  /**
   * @fn void svg_marker_set_ref(svg_marker *m, svg_point *ref)
   *
   * @brief sets ref attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   * @param ref - pointer to existing @a svg_point struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_marker_set_ref(svg_marker *m, svg_point *ref)
{
  if (!m || !ref) return;
  memcpy(&m->ref, ref, sizeof(svg_point));
}

  /**
   * @fn svg_orient *svg_marker_get_orient(svg_marker *m)
   *
   * @brief returns orient attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @return pointer to @a svg_orient struct
   */

svg_orient *svg_marker_get_orient(svg_marker *m) { return m ? m->orient : NULL; }

  /**
   * @fn void svg_marker_set_orient(svg_marker *m, svg_orient *orient)
   *
   * @brief sets orient attribute of @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   * @param orient - pointer to existing @a svg_orient struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_marker_set_orient(svg_marker *m, svg_orient *orient)
{
  if (!m || !orient) return;
  memcpy(&m->orient, orient, sizeof(svg_orient));
}

  /**
   * @fn svg_elements *svg_marker_get_elements(svg_marker *m)
   *
   * @brief returns returns elements contained in @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   *
   * @return pointer to @a svg_elements struct
   */

svg_elements *svg_marker_get_elements(svg_marker *m) { return m ? m->els : NULL; }

  /**
   * @fn void svg_marker_set_elements(svg_marker *m, svg_elements *ses)
   *
   * @brief sets elements contained in @p m
   *
   * @param m - pointer to existing @a svg_marker struct
   * @param ses - pointer to existing @a svg_elements struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_marker_set_elements(svg_marker *m, svg_elements *ses)
{
  if (!m) return;
  if (m->els) svg_elements_free(m->els);
  m->els = NULL;
  m->els = svg_elements_dup(ses);
}

  /*
   *  transform and sub types
   */

    /*
     *  svg_transform
     */

  /**
   * @fn svg_transform *svg_transform_new(void)
   *
   * @brief creates a new @a svg_transform struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform struct
   */

svg_transform *svg_transform_new(void)
{
  svg_transform *st = (svg_transform *)malloc(sizeof(svg_transform));
  if (st) memset(st, 0, sizeof(svg_transform));
  return st;
}

  /**
   * @fn svg_transform *svg_transform_dup(svg_transform *st)
   *
   * @brief copies contents of @p st to new @a svg_transform struct
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to new @a svg_transform struct
   */

svg_transform *svg_transform_dup(svg_transform *st)
{
  svg_transform *nst;

  if (!st) return NULL;

  nst = svg_transform_new();
  if (!nst) return NULL;

  memcpy(nst, st, sizeof(svg_transform));

  return nst;
}

  /**
   * @fn void svg_transform_free(svg_transform *st)
   *
   * @brief frees all memory associated with @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_free(svg_transform *st) { if (st) free(st); }

  /**
   * @fn svg_transform_type svg_transform_get_type(svg_transform *st)
   *
   * @brief returns type of transform @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return @a svg_transform_type value of type attribute
   */

svg_transform_type svg_transform_get_type(svg_transform *st) { return st ? st->type : svg_transform_type_none; }

  /**
   * @fn void svg_transform_set_type(svg_transform *st, svg_transform_type type)
   *
   * @brief sets type of transform @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param type - @ svg_transform_type value of type attribute
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_type(svg_transform *st, svg_transform_type type) { if (st) st->type = type; }

  /**
   * @fn svg_transform_matrix *svg_transform_get_matrix(svg_transform *st)
   *
   * @brief returns matrix transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to @a svg_transform_matrix struct
   */

svg_transform_matrix *svg_transform_get_matrix(svg_transform *st)
{
  if (!st) return NULL;
  if (st->type != svg_transform_type_matrix) return NULL;
  return &st->matrix;
}

  /**
   * @fn void svg_transform_set_matrix(svg_transform *st, svg_transform_matrix *stm)
   *
   * @brief sets matrix transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_matrix(svg_transform *st, svg_transform_matrix *stm)
{
  if (!st || !stm) return;
  st->type = svg_transform_type_matrix;
  memcpy(&st->matrix, stm, sizeof(svg_transform_matrix));
}

  /**
   * @fn svg_transform_translate *svg_transform_get_translate(svg_transform *st)
   *
   * @brief returns translate transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to @a svg_transform_translate struct
   */

svg_transform_translate *svg_transform_get_translate(svg_transform *st)
{
  if (!st) return NULL;
  if (st->type != svg_transform_type_translate) return NULL;
  return &st->translate;
}

  /**
   * @fn void svg_transform_set_translate(svg_transform *st, svg_transform_translate *stt)
   *
   * @brief sets translate transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param stt - pointer to existing @a svg_transform_translate struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_translate(svg_transform *st, svg_transform_translate *stt)
{
  if (!st || !stt) return;
  st->type = svg_transform_type_translate;
  memcpy(&st->translate, stt, sizeof(svg_transform_translate));
}

  /**
   * @fn svg_transform_scale *svg_transform_get_scale(svg_transform *st)
   *
   * @brief returns scale transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to @a svg_transform_scale struct
   */

svg_transform_scale *svg_transform_get_scale(svg_transform *st)
{
  if (!st) return NULL;
  if (st->type != svg_transform_type_scale) return NULL;
  return &st->scale;
}

  /**
   * @fn void svg_transform_set_scale(svg_transform *st, svg_transform_scale *sts)
   *
   * @brief sets scale transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param sts - pointer to existing @a svg_transform_scale struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_scale(svg_transform *st, svg_transform_scale *sts)
{
  if (!st || !sts) return;
  st->type = svg_transform_type_scale;
  memcpy(&st->scale, sts, sizeof(svg_transform_scale));
}

  /**
   * @fn svg_transform_rotate *svg_transform_get_rotate(svg_transform *st)
   *
   * @brief returns rotate transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to @a svg_transform_rotate struct
   */

svg_transform_rotate *svg_transform_get_rotate(svg_transform *st)
{
  if (!st) return NULL;
  if (st->type != svg_transform_type_rotate) return NULL;
  return &st->rotate;
}

  /**
   * @fn void svg_transform_set_rotate(svg_transform *st, svg_transform_rotate *str)
   *
   * @brief sets rotate transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param str - pointer to existing @a svg_transform_rotate struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_rotate(svg_transform *st, svg_transform_rotate *str)
{
  if (!st || !str) return;
  st->type = svg_transform_type_rotate;
  memcpy(&st->rotate, str, sizeof(svg_transform_rotate));
}

  /**
   * @fn svg_transform_skewX *svg_transform_get_skewX(svg_transform *st)
   *
   * @brief returns skewX transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to @a svg_transform_skewX struct
   */

svg_transform_skewX *svg_transform_get_skewX(svg_transform *st)
{
  if (!st) return NULL;
  if (st->type != svg_transform_type_skew_x) return NULL;
  return &st->skew_x;
}

  /**
   * @fn void svg_transform_set_skewX(svg_transform *st, svg_transform_skewX *sts)
   *
   * @brief sets skewX transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param sts - pointer to existing @a svg_transform_skewX struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_skewX(svg_transform *st, svg_transform_skewX *sts)
{
  if (!st || !sts) return;
  st->type = svg_transform_type_skew_x;
  memcpy(&st->skew_x, sts, sizeof(svg_transform_skewX));
}

  /**
   * @fn svg_transform_skewY *svg_transform_get_skewY(svg_transform *st)
   *
   * @brief returns skewY transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   *
   * @return pointer to @a svg_transform_skewY struct
   */

svg_transform_skewY *svg_transform_get_skewY(svg_transform *st)
{
  if (!st) return NULL;
  if (st->type != svg_transform_type_skew_y) return NULL;
  return &st->skew_y;
}

  /**
   * @fn void svg_transform_set_skewY(svg_transform *st, svg_transform_skewY *sts)
   *
   * @brief sets skewY transform of @p st
   *
   * @param st - pointer to existing @a svg_transform struct
   * @param sts - pointer to existing @a svg_transform_skewY struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_set_skewY(svg_transform *st, svg_transform_skewY *sts)
{
  if (!st || !sts) return;
  st->type = svg_transform_type_skew_y;
  memcpy(&st->skew_y, sts, sizeof(svg_transform_skewY));
}

    /*
     *  svg_transforms
     */

  /**
   * @fn svg_transforms *svg_transforms_new(void)
   *
   * @brief creates a new @a svg_transforms struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transforms struct
   */

svg_transforms *svg_transforms_new(void)
{
  svg_transforms *sts = (svg_transforms *)malloc(sizeof(svg_transforms));
  if (sts) memset(sts, 0, sizeof(svg_transforms));
  return sts;
}

  /**
   * @fn svg_transforms *svg_transforms_dup(svg_transforms *sts)
   *
   * @brief copies contents of @p sts to new @a svg_transforms struct
   *
   * @param sts - pointer to existing @a svg_transforms struct
   *
   * @return pointer to new @a svg_transforms struct
   */

svg_transforms *svg_transforms_dup(svg_transforms *sts)
{
  svg_transforms *nsts;
  int i;

  if (!sts) return NULL;

  nsts = svg_transforms_new();
  if (!nsts) return NULL;

  if (sts->arr)
    for (i = 0; i < sts->size; i++) svg_transforms_add(nsts, sts->arr[i]);

  return nsts;
}

  /**
   * @fn void svg_transforms_free(svg_transforms *sts)
   *
   * @brief frees all memory associated with @p sts
   *
   * @param sts - pointer to existing @a svg_transforms struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transforms_free(svg_transforms *sts)
{
  int i;

  if (!sts) return;

  for (i = 0; i < sts->size; i++) svg_transform_free(sts->arr[i]);
}

  /**
   * @fn void svg_transforms_add(svg_transforms *sts, svg_transform *st)
   *
   * @brief appends a transform to @p sts
   *
   * @param sts - pointer to existing @a svg_transforms struct
   * @param st - pointer to existing @a svg_transform struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transforms_add(svg_transforms *sts, svg_transform *st)
{
  if (!sts || !st) return;

  if (sts->arr) sts->arr = (svg_transform **)realloc(sts->arr, sizeof(svg_transform *) * (sts->size + 1));
  else sts->arr = (svg_transform **)malloc(sizeof(svg_transform *));

  sts->arr[sts->size] = svg_transform_dup(st);

  ++sts->size;
}

  /**
   * @fn void svg_transforms_remove(svg_transforms *sts, int idx)
   *
   * @brief removes tranform at index @p idx from @p sts
   *
   * @param sts - pointer to existing @a svg_transforms struct
   * @param idx - index of transform
   *
   * @par return
   *   Nothing.
   */

void svg_transforms_remove(svg_transforms *sts, int idx)
{
  int i;

  if (!sts) return;
  if (!sts->size) return;
  if (idx > sts->size - 1) return;
  if (idx < 0) return;

  svg_transform_free(sts->arr[idx]);
  for (i = idx + 1; i < sts->size; i++) sts->arr[i - 1] = sts->arr[i];
  sts->arr = (svg_transform **)realloc(sts->arr, sizeof(svg_transform *) * (sts->size - 1));
  --sts->size;
}

  /**
   * @fn svg_transform *svg_transforms_next(svg_transforms *sts)
   *
   * @brief returns next transform in @p sts
   *
   * @param sts - pointer to existing @a svg_transforms struct
   *
   * @return pointer to @a svg_transform struct, NULL if at end of list
   */

svg_transform *svg_transforms_next(svg_transforms *sts)
{
  svg_transform *st;

  if (!sts) return NULL;
  if (!sts->size) return NULL;

  st = sts->arr[sts->cursor];
  ++sts->cursor;
  if (sts->cursor > sts->size)
  {
    st = NULL;
    sts->cursor = 0;
  }

  return st;
}

  /**
   * @fn svg_transform *svg_transforms_previous(svg_transforms *sts)
   *
   * @brief returns previous transform in @p sts
   *
   * @param sts - pointer to existing @a svg_transforms struct
   *
   * @return pointer to @a svg_transform struct, NULL if at beginning of list
   */

svg_transform *svg_transforms_previous(svg_transforms *sts)
{
  svg_transform *st;

  if (!sts) return NULL;
  if (!sts->size) return NULL;

  st = sts->arr[sts->cursor];
  --sts->cursor;
  if (sts->cursor < -1)
  {
    st = NULL;
    sts->cursor = sts->size - 1;
  }

  return st;
}

    /*
     *  matrix
     */

  /**
   * @fn svg_transform_matrix *svg_transform_matrix_new(void)
   *
   * @brief creates a new @a svg_transform_matrix struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform_matrix struct
   */

svg_transform_matrix *svg_transform_matrix_new(void)
{
  svg_transform_matrix *stm = (svg_transform_matrix *)malloc(sizeof(svg_transform_matrix));
  if (stm) memset(stm, 0, sizeof(svg_transform_matrix));
  return stm;
}

  /**
   * @fn svg_transform_matrix *svg_transform_matrix_new_with_all(double a, double b, double c, double d, double e, double f)
   *
   * @brief creates a new @a svg_transform_matrix struct
   *
   * The newly created structure will contain the values passed to the function
   *
   * @param a - value for a element of matrix
   * @param b - value for b element of matrix
   * @param c - value for c element of matrix
   * @param d - value for d element of matrix
   * @param e - value for e element of matrix
   * @param f - value for f element of matrix
   *
   * @return pointer to @a svg_transform_matrix struct
   */

svg_transform_matrix *svg_transform_matrix_new_with_all(double a, double b, double c, double d, double e, double f)
{
  svg_transform_matrix *stm;

  stm = svg_transform_matrix_new();
  if (!stm) return NULL;

  svg_transform_matrix_set_a(stm, a);
  svg_transform_matrix_set_b(stm, b);
  svg_transform_matrix_set_c(stm, c);
  svg_transform_matrix_set_d(stm, d);
  svg_transform_matrix_set_e(stm, e);
  svg_transform_matrix_set_f(stm, f);

  return stm;
}

  /**
   * @fn svg_transform_matrix *svg_transform_matrix_dup(svg_transform_matrix *stm)
   *
   * @brief copies contents of @p stm to new @a svg_transform_matrix struct
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return pointer to new @a svg_transform_matrix struct
   */

svg_transform_matrix *svg_transform_matrix_dup(svg_transform_matrix *stm)
{
  svg_transform_matrix *nstm;

  if (!stm) return NULL;

  nstm = svg_transform_matrix_new();
  if (!nstm) return NULL;

  svg_transform_matrix_set_a(nstm, stm->a);
  svg_transform_matrix_set_b(nstm, stm->b);
  svg_transform_matrix_set_c(nstm, stm->c);
  svg_transform_matrix_set_d(nstm, stm->d);
  svg_transform_matrix_set_e(nstm, stm->e);
  svg_transform_matrix_set_f(nstm, stm->f);

  return nstm;
}

  /**
   * @fn void svg_transform_matrix_free(svg_transform_matrix *stm)
   *
   * @brief frees all memory associated with @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_free(svg_transform_matrix *stm) { if (stm) free(stm); }

  /**
   * @fn double svg_transform_matrix_get_a(svg_transform_matrix *stm)
   *
   * @brief gets a element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return value of a element
   */

double svg_transform_matrix_get_a(svg_transform_matrix *stm) { return stm ? stm->a : 0; }

  /**
   * @fn void svg_transform_matrix_set_a(svg_transform_matrix *stm, double a)
   *
   * @brief sets a element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   * @param a - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_set_a(svg_transform_matrix *stm, double a) { if (stm) stm->a = a; }

  /**
   * @fn double svg_transform_matrix_get_b(svg_transform_matrix *stm)
   *
   * @brief gets b element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return value of b element
   */

double svg_transform_matrix_get_b(svg_transform_matrix *stm) { return stm ? stm->b : 0; }

  /**
   * @fn void svg_transform_matrix_set_b(svg_transform_matrix *stm, double b)
   *
   * @brief sets b element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   * @param b - new value of b element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_set_b(svg_transform_matrix *stm, double b) { if (stm) stm->a = b; }

  /**
   * @fn double svg_transform_matrix_get_c(svg_transform_matrix *stm)
   *
   * @brief gets c element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return value of c element
   */

double svg_transform_matrix_get_c(svg_transform_matrix *stm) { return stm ? stm->c : 0; }

  /**
   * @fn void svg_transform_matrix_set_c(svg_transform_matrix *stm, double c)
   *
   * @brief sets c element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   * @param c - new value of c element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_set_c(svg_transform_matrix *stm, double c) { if (stm) stm->a = c; }

  /**
   * @fn double svg_transform_matrix_get_d(svg_transform_matrix *stm)
   *
   * @brief gets d element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return value of d element
   */

double svg_transform_matrix_get_d(svg_transform_matrix *stm) { return stm ? stm->d : 0; }

  /**
   * @fn void svg_transform_matrix_set_d(svg_transform_matrix *stm, double d)
   *
   * @brief sets d element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   * @param d - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_set_d(svg_transform_matrix *stm, double d) { if (stm) stm->a = d; }

  /**
   * @fn double svg_transform_matrix_get_e(svg_transform_matrix *stm)
   *
   * @brief gets e element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return value of e element
   */

double svg_transform_matrix_get_e(svg_transform_matrix *stm) { return stm ? stm->e : 0; }

  /**
   * @fn void svg_transform_matrix_set_e(svg_transform_matrix *stm, double e)
   *
   * @brief sets e element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   * @param e - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_set_e(svg_transform_matrix *stm, double e) { if (stm) stm->a = e; }

  /**
   * @fn double svg_transform_matrix_get_f(svg_transform_matrix *stm)
   *
   * @brief gets f element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   *
   * @return value of f element
   */

double svg_transform_matrix_get_f(svg_transform_matrix *stm) { return stm ? stm->f : 0; }

  /**
   * @fn void svg_transform_matrix_set_f(svg_transform_matrix *stm, double f)
   *
   * @brief sets f element of @p stm
   *
   * @param stm - pointer to existing @a svg_transform_matrix struct
   * @param f - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_matrix_set_f(svg_transform_matrix *stm, double f) { if (stm) stm->a = f; }

    /*
     *  translate
     */

  /**
   * @fn svg_transform_translate *svg_transform_translate_new(void)
   *
   * @brief creates a new @a svg_transform_translate struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform_translate struct
   */

svg_transform_translate *svg_transform_translate_new(void)
{
  svg_transform_translate *stt = (svg_transform_translate *)malloc(sizeof(svg_transform_translate));
  if (stt) memset(stt, 0, sizeof(svg_transform_translate));
  return stt;
}
  
  /**
   * @fn svg_transform_translate *svg_transform_translate_new_with_all(double x, double y)
   *
   * @brief creates a new @a svg_transform_translate struct
   *
   * The newly created structure will contain the values passed to the function
   *
   * @param x - value for x element of translate
   * @param y - value for y element of translate
   *
   * @return pointer to @a svg_transform_translate struct
   */

svg_transform_translate *svg_transform_translate_new_with_all(double x, double y)
{
  svg_transform_translate *stt;

  stt = svg_transform_translate_new();
  if (!stt) return NULL;

  svg_transform_translate_set_x(stt, x);
  svg_transform_translate_set_y(stt, y);

  return stt;
}

  /**
   * @fn svg_transform_translate *svg_transform_translate_dup(svg_transform_translate *stt)
   *
   * @brief copies contents of @p stt to new @a svg_transform_translate struct
   *
   * @param stt - pointer to existing @a svg_transform_translate struct
   *
   * @return pointer to new @a svg_transform_translate struct
   */

svg_transform_translate *svg_transform_translate_dup(svg_transform_translate *stt)
{
  svg_transform_translate *nstt;

  if (!stt) return NULL;

  nstt = svg_transform_translate_new();
  if (!nstt) return NULL;

  svg_transform_translate_set_x(nstt, stt->x);
  svg_transform_translate_set_y(nstt, stt->y);

  return nstt;
}

  /**
   * @fn void svg_transform_translate_free(svg_transform_translate *stt)
   *
   * @brief frees all memory associated with @p stt
   *
   * @param stt - pointer to existing @a svg_transform_translate struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_translate_free(svg_transform_translate *stt) { if (stt) free(stt); }

  /**
   * @fn double svg_transform_translate_get_x(svg_transform_translate *stt)
   *
   * @brief gets x element of @p stt
   *
   * @param stt - pointer to existing @a svg_transform_translate struct
   *
   * @return value of x element
   */

double svg_transform_translate_get_x(svg_transform_translate *stt) { return stt ? stt->x : 0; }

  /**
   * @fn void svg_transform_translate_set_x(svg_transform_translate *stt, double x)
   *
   * @brief sets value of x element of @p stt
   *
   * @param stt - pointer to existing @a svg_transform_translate struct
   * @param x - new value of x element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_translate_set_x(svg_transform_translate *stt, double x) { if (stt) stt->x = x; }

  /**
   * @fn double svg_transform_translate_get_y(svg_transform_translate *stt)
   *
   * @brief gets y element of @p stt
   *
   * @param stt - pointer to existing @a svg_transform_translate struct
   *
   * @return value of y element
   */

double svg_transform_translate_get_y(svg_transform_translate *stt) { return stt ? stt->y : 0; }

  /**
   * @fn void svg_transform_translate_set_y(svg_transform_translate *stt, double y)
   *
   * @brief sets value of y element of @p stt
   *
   * @param stt - pointer to existing @a svg_transform_translate struct
   * @param y - new value of y element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_translate_set_y(svg_transform_translate *stt, double y) { if (stt) stt->y = y; }

    /*
     *  scale
     */

  /**
   * @fn svg_transform_scale *svg_transform_scale_new(void)
   *
   * @brief creates a new @a svg_transform_scale struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform_scale struct
   */

svg_transform_scale *svg_transform_scale_new(void)
{
  svg_transform_scale *sts = (svg_transform_scale *)malloc(sizeof(svg_transform_scale));
  if (sts) memset(sts, 0, sizeof(svg_transform_scale));
  return sts;
}

  /**
   * @fn svg_transform_scale *svg_transform_scale_new_with_all(double x, double y)
   *
   * @brief creates a new @a svg_transform_scale struct
   *
   * The newly created structure will contain the values passed to the function
   *
   * @param x - value for x element of scale
   * @param y - value for y element of scale
   *
   * @return pointer to @a svg_transform_scale struct
   */

svg_transform_scale *svg_transform_scale_new_with_all(double x, double y)
{
  svg_transform_scale *sts;

  sts = svg_transform_scale_new();
  if (!sts) return NULL;

  svg_transform_scale_set_x(sts, x);
  svg_transform_scale_set_y(sts, y);

  return sts;
}

  /**
   * @fn svg_transform_scale *svg_transform_scale_dup(svg_transform_scale *sts)
   *
   * @brief copies contents of @p sts to new @a svg_transform_scale struct
   *
   * @param sts - pointer to existing @a svg_transform_scale struct
   *
   * @return pointer to new @a svg_transform_scale struct
   */

svg_transform_scale *svg_transform_scale_dup(svg_transform_scale *sts)
{
  svg_transform_scale *nsts;

  if (!sts) return NULL;

  nsts = svg_transform_scale_new();
  if (!nsts) return NULL;

  svg_transform_scale_set_x(nsts, sts->x);
  svg_transform_scale_set_y(nsts, sts->y);

  return nsts;
}

  /**
   * @fn void svg_transform_scale_free(svg_transform_scale *sts)
   *
   * @brief frees all memory associated with @p sts
   *
   * @param sts - pointer to existing @a svg_transform_scale struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_scale_free(svg_transform_scale *sts) { if (sts) free(sts); }

  /**
   * @fn double svg_transform_scale_get_x(svg_transform_scale *sts)
   *
   * @brief gets x element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_scale struct
   *
   * @return value of x element
   */

double svg_transform_scale_get_x(svg_transform_scale *sts) { return sts ? sts->x : 0; }

  /**
   * @fn void svg_transform_scale_set_x(svg_transform_scale *sts, double x)
   *
   * @brief sets value of x element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_scale struct
   * @param x - new value of x element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_scale_set_x(svg_transform_scale *sts, double x) { if (sts) sts->x = x; }

  /**
   * @fn double svg_transform_scale_get_y(svg_transform_scale *sts)
   *
   * @brief gets y element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_scale struct
   *
   * @return value of y element
   */

double svg_transform_scale_get_y(svg_transform_scale *sts) { return sts ? sts->y : 0; }

  /**
   * @fn void svg_transform_scale_set_y(svg_transform_scale *sts, double y)
   *
   * @brief sets value of y element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_scale struct
   * @param y - new value of y element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_scale_set_y(svg_transform_scale *sts, double y) { if (sts) sts->y = y; }

    /*
     *  rotate
     */

  /**
   * @fn svg_transform_rotate *svg_transform_rotate_new(void)
   *
   * @brief creates a new @a svg_transform_rotate struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform_rotate struct
   */

svg_transform_rotate *svg_transform_rotate_new(void)
{
  svg_transform_rotate *str = (svg_transform_rotate *)malloc(sizeof(svg_transform_rotate));
  if (str) memset(str, 0, sizeof(svg_transform_rotate));
  return str;
}

  /**
   * @fn svg_transform_rotate *svg_transform_rotate_new_with_all(double a, double x, double y)
   *
   * @brief creates a new @a svg_transform_rotate struct
   *
   * The newly created structure will contain the values passed to the function
   *
   * @param a - value for a element of rotate
   * @param x - value for x element of rotate
   * @param y - value for y element of rotate
   *
   * @return pointer to @a svg_transform_rotate struct
   */

svg_transform_rotate *svg_transform_rotate_new_with_all(double a, double x, double y)
{
  svg_transform_rotate *str;

  str = svg_transform_rotate_new();
  if (!str) return NULL;

  svg_transform_rotate_set_a(str, a);
  svg_transform_rotate_set_x(str, x);
  svg_transform_rotate_set_y(str, y);

  return str;
}

  /**
   * @fn svg_transform_rotate *svg_transform_rotate_dup(svg_transform_rotate *str)
   *
   * @brief copies contents of @p str to new @a svg_transform_rotate struct
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   *
   * @return pointer to new @a svg_transform_rotate struct
   */

svg_transform_rotate *svg_transform_rotate_dup(svg_transform_rotate *str)
{
  svg_transform_rotate *nstr;

  if (!str) return NULL;

  nstr = svg_transform_rotate_new();
  if (!nstr) return NULL;

  svg_transform_rotate_set_x(nstr, str->x);
  svg_transform_rotate_set_y(nstr, str->y);

  return nstr;
}

  /**
   * @fn void svg_transform_rotate_free(svg_transform_rotate *str)
   *
   * @brief frees all memory associated with @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_rotate_free(svg_transform_rotate *str) { if (str) free(str); }

  /**
   * @fn double svg_transform_rotate_get_a(svg_transform_rotate *str)
   *
   * @brief gets a element of @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   *
   * @return value of a element
   */

double svg_transform_rotate_get_a(svg_transform_rotate *str) { return str ? str->a : 0; }

  /**
   * @fn void svg_transform_rotate_set_a(svg_transform_rotate *str, double a)
   *
   * @brief sets value of a element of @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   * @param a - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_rotate_set_a(svg_transform_rotate *str, double a) { if (str) str->a = a; }

  /**
   * @fn double svg_transform_rotate_get_x(svg_transform_rotate *str)
   *
   * @brief gets x element of @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   *
   * @return value of x element
   */

double svg_transform_rotate_get_x(svg_transform_rotate *str) { return str ? str->x : 0; }

  /**
   * @fn void svg_transform_rotate_set_x(svg_transform_rotate *str, double x)
   *
   * @brief sets value of x element of @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   * @param x - new value of x element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_rotate_set_x(svg_transform_rotate *str, double x) { if (str) str->x = x; }

  /**
   * @fn double svg_transform_rotate_get_y(svg_transform_rotate *str)
   *
   * @brief gets y element of @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   *
   * @return value of y element
   */

double svg_transform_rotate_get_y(svg_transform_rotate *str) { return str ? str->y : 0; }

  /**
   * @fn void svg_transform_rotate_set_y(svg_transform_rotate *str, double y)
   *
   * @brief sets value of y element of @p str
   *
   * @param str - pointer to existing @a svg_transform_rotate struct
   * @param y - new value of y element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_rotate_set_y(svg_transform_rotate *str, double y) { if (str) str->y = y; }

    /*
     *  skewX
     */

  /**
   * @fn svg_transform_skewX *svg_transform_skewX_new(void)
   *
   * @brief creates a new @a svg_transform_skewX struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform_skewX struct
   */

svg_transform_skewX *svg_transform_skewX_new(void)
{
  svg_transform_skewX *sts = (svg_transform_skewX *)malloc(sizeof(svg_transform_skewX));
  if (sts) memset(sts, 0, sizeof(svg_transform_skewX));
  return sts;
}

  /**
   * @fn svg_transform_skewX *svg_transform_skewX_new_with_all(double a)
   *
   * @brief creates a new @a svg_transform_skewX struct
   *
   * The newly created structure will contain the values passed to the function
   *
   * @param a - value for a element of skewX
   *
   * @return pointer to @a svg_transform_skewX struct
   */

svg_transform_skewX *svg_transform_skewX_new_with_all(double a)
{
  svg_transform_skewX *sts;

  sts = svg_transform_skewX_new();
  if (!sts) return NULL;

  svg_transform_skewX_set_a(sts, a);

  return sts;
}

  /**
   * @fn svg_transform_skewX *svg_transform_skewX_dup(svg_transform_skewX *sts)
   *
   * @brief copies contents of @p sts to new @a svg_transform_skewX struct
   *
   * @param sts - pointer to existing @a svg_transform_skewX struct
   *
   * @return pointer to new @a svg_transform_skewX struct
   */

svg_transform_skewX *svg_transform_skewX_dup(svg_transform_skewX *sts)
{
  svg_transform_skewX *nsts;

  if (!sts) return NULL;

  nsts = svg_transform_skewX_new();
  if (!nsts) return NULL;

  svg_transform_skewX_set_a(nsts, sts->a);

  return nsts;
}

  /**
   * @fn void svg_transform_skewX_free(svg_transform_skewX *sts)
   *
   * @brief frees all memory associated with @p sts
   *
   * @param sts - pointer to existing @a svg_transform_skewX struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_skewX_free(svg_transform_skewX *sts) { if (sts) free(sts); }

  /**
   * @fn double svg_transform_skewX_get_a(svg_transform_skewX *sts)
   *
   * @brief gets a element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_skewX struct
   *
   * @return value of a element
   */

double svg_transform_skewX_get_a(svg_transform_skewX *sts) { return sts ? sts->a : 0; }

  /**
   * @fn void svg_transform_skewX_set_a(svg_transform_skewX *sts, double a)
   *
   * @brief sets value of a element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_skewX struct
   * @param a - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_skewX_set_a(svg_transform_skewX *sts, double a) { if (sts) sts->a = a; }

    /*
     *  skewY
     */

  /**
   * @fn svg_transform_skewY *svg_transform_skewY_new(void)
   *
   * @brief creates a new @a svg_transform_skewY struct
   *
   * @par Parameters
   *   None.
   *
   * @return pointer to @a svg_transform_skewY struct
   */

svg_transform_skewY *svg_transform_skewY_new(void)
{
  svg_transform_skewY *sts = (svg_transform_skewY *)malloc(sizeof(svg_transform_skewY));
  if (sts) memset(sts, 0, sizeof(svg_transform_skewY));
  return sts;
}

  /**
   * @fn svg_transform_skewY *svg_transform_skewY_new_with_all(double a)
   *
   * @brief creates a new @a svg_transform_skewY struct
   *
   * The newly created structure will contain the values passed to the function
   *
   * @param a - value for a element of skewY
   *
   * @return pointer to @a svg_transform_skewY struct
   */

svg_transform_skewY *svg_transform_skewY_new_with_all(double a)
{
  svg_transform_skewY *sts;

  sts = svg_transform_skewY_new();
  if (!sts) return NULL;

  svg_transform_skewY_set_a(sts, a);

  return sts;
}

  /**
   * @fn svg_transform_skewY *svg_transform_skewY_dup(svg_transform_skewY *sts)
   *
   * @brief copies contents of @p sts to new @a svg_transform_skewY struct
   *
   * @param sts - pointer to existing @a svg_transform_skewY struct
   *
   * @return pointer to new @a svg_transform_skewY struct
   */

svg_transform_skewY *svg_transform_skewY_dup(svg_transform_skewY *sts)
{
  svg_transform_skewY *nsts;

  if (!sts) return NULL;

  nsts = svg_transform_skewY_new();
  if (!nsts) return NULL;

  svg_transform_skewY_set_a(nsts, sts->a);

  return nsts;
}

  /**
   * @fn void svg_transform_skewY_free(svg_transform_skewY *sts)
   *
   * @brief frees all memory associated with @p sts
   *
   * @param sts - pointer to existing @a svg_transform_skewY struct
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_skewY_free(svg_transform_skewY *sts) { if (sts) free(sts); }

  /**
   * @fn double svg_transform_skewY_get_a(svg_transform_skewY *sts)
   *
   * @brief gets a element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_skewY struct
   *
   * @return value of a element
   */

double svg_transform_skewY_get_a(svg_transform_skewY *sts) { return sts ? sts->a : 0; }

  /**
   * @fn void svg_transform_skewY_set_a(svg_transform_skewY *sts, double a)
   *
   * @brief sets value of a element of @p sts
   *
   * @param sts - pointer to existing @a svg_transform_skewY struct
   * @param a - new value of a element
   *
   * @par Returns
   *   Nothing.
   */

void svg_transform_skewY_set_a(svg_transform_skewY *sts, double a) { if (sts) sts->a = a; }

  /*
   * Module local functions
   */

char *strapp(char *s1, char *s2)
{
  int len1, len2;

  if (!s1)
  {
    s1 = (char *)malloc(1);
    if (!s1) return NULL;
    *s1 = 0;
  }

  if (!s2) return s1;

  len1 = strlen(s1);
  len2 = strlen(s2);

  s1 = (char *)realloc(s1, len1 + len2 + 1);
  strcat(s1, s2);

  return s1;
}

static xml_element *element_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  char *t;
  char *id;
  char *class;
  svg_transforms *transforms;
  svg_style *style;

  if (!el) goto exit;

  switch (el->type)
  {
    case svg_element_type_rect: xe = rect_to_xml(el); break;
    case svg_element_type_circle: xe = circle_to_xml(el); break;
    case svg_element_type_ellipse: xe = ellipse_to_xml(el); break;
    case svg_element_type_line: xe = line_to_xml(el); break;
    case svg_element_type_polygon: xe = polygon_to_xml(el); break;
    case svg_element_type_polyline: xe = polyline_to_xml(el->any); break;
    case svg_element_type_path: xe = path_to_xml(el); break;
    case svg_element_type_text: xe = text_to_xml(el); break;
    case svg_element_type_textpath: xe = textpath_to_xml(el); break;
    case svg_element_type_link: xe = link_to_xml(el); break;
    case svg_element_type_image: xe = image_to_xml(el); break;
    case svg_element_type_marker: xe = marker_to_xml(el); break;

    case svg_element_type_none:
    default:
      break;
  }

  as = xml_element_get_attributes(xe);
  
  id = svg_element_get_id(el);
  if (id)
  {
    if (!as) as = xml_attributes_new();
    if (!as) goto exit;
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "id");
    xml_attribute_set_value(a, id);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  class = svg_element_get_class(el);
  if (class)
  {
    if (!as) as = xml_attributes_new();
    if (!as) goto exit;
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "class");
    xml_attribute_set_value(a, class);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  transforms = svg_element_get_transforms(el);
  if (transforms)
  {
    if (!as) as = xml_attributes_new();
    if (!as) goto exit;
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "transform");
    xml_attribute_set_value(a, t = transforms_to_xml(transforms));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  style = svg_element_get_style(el);
  if (style)
  {
    if (!as) as = xml_attributes_new();
    if (!as) goto exit;
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "style");
    xml_attribute_set_value(a, t = style_to_xml(style));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

exit:
  //if (as) xml_attributes_free(as);

  return xe;
}

static xml_elements *elements_to_xml(svg_elements *els)
{
  xml_elements *xes = NULL;
  xml_element *xe;
  int i;

  if (!els) goto exit;

  xes = xml_elements_new();
  if (!xes) goto exit;

  for (i = 0; i < els->size; i++)
  {
    xe = element_to_xml(els->arr[i]);
    if (xe)
    {
      xml_elements_add(xes, xe);
      xml_element_free(xe);
    }
  }

exit:
  return xes;
}

static xml_element *rect_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_rect *r;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_rect) goto exit;

  r = el->r;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "rect");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "width");
  sprintf(tbuf, "%g", r->width);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "height");
  sprintf(tbuf, "%g", r->height);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "x");
  sprintf(tbuf, "%g", r->p.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "y");
  sprintf(tbuf, "%g", r->p.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  if (r->rx)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "rx");
    sprintf(tbuf, "%g", r->rx);
    xml_attribute_set_value(a, tbuf);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (r->ry)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "ry");
    sprintf(tbuf, "%g", r->ry);
    xml_attribute_set_value(a, tbuf);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *circle_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_circle *c;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_circle) goto exit;

  c = el->c;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "circle");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "r");
  sprintf(tbuf, "%g", c->r);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "cx");
  sprintf(tbuf, "%g", c->c.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "cy");
  sprintf(tbuf, "%g", c->c.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *ellipse_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_ellipse *e;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_ellipse) goto exit;

  e = el->e;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "ellipse");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "rx");
  sprintf(tbuf, "%g", e->rx);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);
  
  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "ry");
  sprintf(tbuf, "%g", e->ry);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);
  
  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "cx");
  sprintf(tbuf, "%g", e->c.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);
  
  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "cy");
  sprintf(tbuf, "%g", e->c.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);
  
  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *line_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_line *l;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_line) goto exit;

  l = el->l;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "line");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "x1");
  sprintf(tbuf, "%g", l->p1.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "y1");
  sprintf(tbuf, "%g", l->p1.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "x2");
  sprintf(tbuf, "%g", l->p2.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "y2");
  sprintf(tbuf, "%g", l->p2.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *polygon_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_polygon *pg;
  svg_points *pts;
  svg_point *pt;
  char *str = NULL;
  char tbuf[256];
  int i;

  if (!el) goto exit;
  if (el->type != svg_element_type_polygon) goto exit;

  pg = el->pg;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "polygon");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "points");
  pts = pg->pts;
  for (i = 0; i < pts->size; i++)
  {
    pt = pts->arr[i];
    if (i > 0) str = strapp(str, " ");
    sprintf(tbuf, "%g", pt->x);
    str = strapp(str, tbuf);
    str = strapp(str, ",");
    sprintf(tbuf, "%g", pt->y);
    str = strapp(str, tbuf);
  }
  xml_attribute_set_value(a, str);
  free(str);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *polyline_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_polyline *pl;
  svg_points *pts;
  svg_point *pt;
  char *str = NULL;
  char tbuf[256];
  int i;

  if (!el) goto exit;
  if (el->type != svg_element_type_polyline) goto exit;

  pl = el->pl;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "polyline");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "points");
  pts = pl->pts;
  for (i = 0; i < pts->size; i++)
  {
    pt = pts->arr[i];
    if (i > 0) str = strapp(str, " ");
    sprintf(tbuf, "%g", pt->x);
    str = strapp(str, tbuf);
    str = strapp(str, ",");
    sprintf(tbuf, "%g", pt->y);
    str = strapp(str, tbuf);
  }
  xml_attribute_set_value(a, str);
  free(str);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *path_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_path *ph;

  if (!el) goto exit;
  if (el->type != svg_element_type_path) goto exit;

  ph = el->ph;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "path");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "d");
  xml_attribute_set_value(a, ph->d);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *text_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_text *tx;
  char *t;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_text) goto exit;

  tx = el->t;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_type(xe, xml_element_type_text);
  xml_element_set_name(xe, "text");

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "x");
  sprintf(tbuf, "%g", tx->p.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "y");
  sprintf(tbuf, "%g", tx->p.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  if (tx->dx)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "dx");
    sprintf(tbuf, "%g", tx->dx);
    xml_attribute_set_value(a, tbuf);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }
  
  if (tx->dy)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "dy");
    sprintf(tbuf, "%g", tx->dy);
    xml_attribute_set_value(a, tbuf);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }
  
  if (tx->rotate)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "rotate");
    sprintf(tbuf, "%g", tx->rotate);
    xml_attribute_set_value(a, tbuf);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }
  
  if (tx->text_length.type)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "textLength");
    xml_attribute_set_value(a, t = text_length_to_xml(&tx->text_length));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (tx->text_length.type)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "lengthAdjust");
    xml_attribute_set_value(a, t = length_adjust_type_to_xml(tx->length_adjust));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  xml_element_set_attributes(xe, as);

  if (tx->contents) xml_element_set_text(xe, tx->contents);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *textpath_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_textpath *tp;
  char *t;

  if (!el) goto exit;
  if (el->type != svg_element_type_textpath) goto exit;

  tp = el->tp;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_type(xe, xml_element_type_text);
  xml_element_set_name(xe, "textpath");

  as = xml_attributes_new();
  if (!as) goto exit;

  if (tp->href)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "href");
    xml_attribute_set_value(a, tp->href);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (tp->length_adjust)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "lengthAdjust");
    xml_attribute_set_value(a, t = length_adjust_type_to_xml(tp->length_adjust));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (tp->method)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "method");
    xml_attribute_set_value(a, t = method_type_to_xml(tp->method));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (tp->method)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "spacing");
    xml_attribute_set_value(a, t = spacing_type_to_xml(tp->spacing));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (tp->start_offset.type)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "startOffset");
    xml_attribute_set_value(a, t = text_length_to_xml(&tp->start_offset));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (tp->text_length.type)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "textLength");
    xml_attribute_set_value(a, t = text_length_to_xml(&tp->text_length));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  xml_element_set_attributes(xe, as);

  if (tp->contents) xml_element_set_text(xe, tp->contents);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *link_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_elements *xes;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_link *lnk;

  if (!el) goto exit;
  if (el->type != svg_element_type_link) goto exit;

  lnk = el->lnk;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_type(xe, xml_element_type_container);
  xml_element_set_name(xe, "a");

  as = xml_attributes_new();
  if (!as) goto exit;

  if (lnk->href)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "href");
    xml_attribute_set_value(a, lnk->href);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (lnk->download)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "download");
    xml_attribute_set_value(a, lnk->download);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (lnk->hreflang)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "hreflang");
    xml_attribute_set_value(a, lnk->hreflang);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (lnk->referrer_policy)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "referrerpolicy");
    xml_attribute_set_value(a, lnk->referrer_policy);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (lnk->rel)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "rel");
    xml_attribute_set_value(a, lnk->rel);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (lnk->target)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "target");
    xml_attribute_set_value(a, lnk->target);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  if (lnk->type)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "type");
    xml_attribute_set_value(a, lnk->type);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

    /*
     *  include elements that define the visual link
     */

  if (lnk->els)
  {
    xes = elements_to_xml(lnk->els);
    if (xes)
    {
      xml_element_set_elements(xe, xes);
      xml_elements_free(xes);
    }
  }

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *image_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_image *img;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_image) goto exit;

  img = el->img;
  if (!img) goto exit;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_name(xe, "image");

  memset(tbuf, 0, 256);

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "width");
  sprintf(tbuf, "%g", img->width);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "height");
  sprintf(tbuf, "%g", img->height);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "x");
  sprintf(tbuf, "%g", img->p.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "y");
  sprintf(tbuf, "%g", img->p.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  if (img->href)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "href");
    xml_attribute_set_value(a, img->href);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  xml_element_set_attributes(xe, as);

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static xml_element *marker_to_xml(svg_element *el)
{
  xml_element *xe = NULL;
  xml_elements *xes;
  xml_attributes *as = NULL;
  xml_attribute *a;
  svg_marker *m;
  char *t;
  char tbuf[256];

  if (!el) goto exit;
  if (el->type != svg_element_type_marker) goto exit;

  m = el->m;

  xe = xml_element_new();
  if (!xe) goto exit;

  xml_element_set_type(xe, xml_element_type_container);
  xml_element_set_name(xe, "marker");

  memset(tbuf, 0, 256);

  as = xml_attributes_new();
  if (!as) goto exit;

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "markerWidth");
  sprintf(tbuf, "%g", m->marker_width);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "markerHeight");
  sprintf(tbuf, "%g", m->marker_height);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "refX");
  sprintf(tbuf, "%g", m->ref.x);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  a = xml_attribute_new();
  if (!a) goto exit;
  xml_attribute_set_name(a, "refY");
  sprintf(tbuf, "%g", m->ref.y);
  xml_attribute_set_value(a, tbuf);
  xml_attributes_add(as, a);
  xml_attribute_free(a);

  if (m->orient)
  {
    a = xml_attribute_new();
    if (!a) goto exit;
    xml_attribute_set_name(a, "orient");
    xml_attribute_set_value(a, t = orient_to_xml(m->orient));
    if (t) free(t);
    xml_attributes_add(as, a);
    xml_attribute_free(a);
  }

  xml_element_set_attributes(xe, as);

  if (m->els)
  {
    xes = elements_to_xml(m->els);
    if (xes)
    {
      xml_element_set_elements(xe, xes);
      xml_elements_free(xes);
    }
  }

exit:
  if (as) xml_attributes_free(as);

  return xe;
}

static char *transform_matrix_to_xml(svg_transform_matrix *stm)
{
  char *s = NULL;
  char tbuf[4096];

  s = strapp(s, "");

  if (!stm) return s;

  s = strapp(s, "matrix(");
  sprintf(tbuf, "%g,%g,%g,%g,%g,%g", stm->a, stm->b, stm->c, stm->d, stm->e, stm->f);
  s = strapp(s, tbuf);
  s = strapp(s, ")");

  return s;
}

static char *transform_translate_to_xml(svg_transform_translate *stt)
{
  char *s = NULL;
  char tbuf[4096];

  s = strapp(s, "");

  if (!stt) return s;

  s = strapp(s, "translate(");
  sprintf(tbuf, "%g,%g", stt->x, stt->y);
  s = strapp(s, tbuf);
  s = strapp(s, ")");

  return s;
}

static char *transform_scale_to_xml(svg_transform_scale *sts)
{
  char *s = NULL;
  char tbuf[4096];

  s = strapp(s, "");

  if (!sts) return s;

  s = strapp(s, "scale(");
  sprintf(tbuf, "%g,%g", sts->x, sts->y);
  s = strapp(s, tbuf);
  s = strapp(s, ")");

  return s;
}

static char *transform_rotate_to_xml(svg_transform_rotate *str)
{
  char *s = NULL;
  char tbuf[4096];

  s = strapp(s, "");

  if (!str) return s;

  s = strapp(s, "rotate(");
  sprintf(tbuf, "%g,%g,%g", str->a, str->x, str->y);
  s = strapp(s, tbuf);
  s = strapp(s, ")");

  return s;
}

static char *transform_skewX_to_xml(svg_transform_skewX *sts)
{
  char *s = NULL;
  char tbuf[4096];

  s = strapp(s, "");

  if (!sts) return s;

  s = strapp(s, "skewX(");
  sprintf(tbuf, "%g", sts->a);
  s = strapp(s, tbuf);
  s = strapp(s, ")");

  return s;
}

static char *transform_skewY_to_xml(svg_transform_skewY *sts)
{
  char *s = NULL;
  char tbuf[4096];

  s = strapp(s, "");

  if (!sts) return s;

  s = strapp(s, "skewY(");
  sprintf(tbuf, "%g", sts->a);
  s = strapp(s, tbuf);
  s = strapp(s, ")");

  return s;
}

static char *style_to_xml(svg_style *st)
{
  char *s = NULL;
  char tbuf[256];
  char *t;

  s = strapp(s, "");

  if (!st) return NULL;

  if (st->fill)
  {
    s = strapp(s, "fill:");
    s = strapp(s, st->fill);
    s = strapp(s, ";");
  }

  if ((st->fill_opacity >= 0) && (st->fill_opacity <= 1))
  {
    s = strapp(s, "fill-opacity:");
    sprintf(tbuf, "%g", st->fill_opacity);
    s = strapp(s, tbuf);
    s = strapp(s, ";");
  }

  if (st->fill_rule == svg_fill_rule_type_evenodd) s = strapp(s, "fill-rule:evenodd;");

  if (st->stroke)
  {
    s = strapp(s, "stroke:");
    s = strapp(s, st->stroke);
    s = strapp(s, ";");
  }

  if (st->stroke_width != 1)
  {
    s = strapp(s, "stroke-width:");
    sprintf(tbuf, "%g", st->stroke_width);
    s = strapp(s, tbuf);
    s = strapp(s, ";");
  }

  if ((st->stroke_opacity >= 0) && (st->stroke_opacity <= 1))
  {
    s = strapp(s, "stroke-opacity:");
    sprintf(tbuf, "%g", st->stroke_opacity);
    s = strapp(s, tbuf);
    s = strapp(s, ";");
  }

  switch (st->stroke_linecap)
  {
    case svg_stroke_linecap_type_round:
    case svg_stroke_linecap_type_square:
      s = strapp(s, "stroke-linecap:");
      switch (st->stroke_linecap)
      {
        case svg_stroke_linecap_type_round:
          s = strapp(s, "round");
          break;
        case svg_stroke_linecap_type_square:
          s = strapp(s, "square");
          break;
        default: break;
      }
      s = strapp(s, ";");
      break;

    case svg_stroke_linecap_type_butt:
    default: break;
  }

  if (st->stroke_dash_array)
  {
    s = strapp(s, "dash-array:");
    s = strapp(s, st->stroke_dash_array);
    s = strapp(s, ";");
  }

  switch (st->stroke_linejoin)
  {
    case svg_stroke_linejoin_type_bevel:
    case svg_stroke_linejoin_type_miter:
    case svg_stroke_linejoin_type_miter_clip:
    case svg_stroke_linejoin_type_round:
      s = strapp(s, "stroke-linejoin:");
      switch (st->stroke_linejoin)
      {
        case svg_stroke_linejoin_type_bevel:
          s = strapp(s, "bevel");
          break;
        case svg_stroke_linejoin_type_miter:
          s = strapp(s, "miter");
          break;
        case svg_stroke_linejoin_type_miter_clip:
          s = strapp(s, "clip");
          break;
        case svg_stroke_linejoin_type_round:
          s = strapp(s, "round");
          break;
        default: break;
      }
      s = strapp(s, ";");
      break;

    case svg_stroke_linejoin_type_arcs:
    default: break;
  }

  if (st->font_family)
  {
    s = strapp(s, "font-family:");
    s = strapp(s, st->font_family);
    s = strapp(s, ";");
  }

  s = strapp(s, t = font_weight_type_to_xml(st->font_weight));
  free(t);
  s = strapp(s, ";");

  s = strapp(s, t = font_stretch_type_to_xml(st->font_stretch));
  free(t);
  s = strapp(s, ";");

  s = strapp(s, t = font_style_type_to_xml(st->font_style));
  free(t);
  s = strapp(s, ";");

  if (st->font_size)
  {
    s = strapp(s, "font-size:");
    s = strapp(s, st->font_size);
    s = strapp(s, ";");
  }

  return s;
}

static char *transforms_to_xml(svg_transforms *sts)
{
  char *s = NULL;
  char *t;
  int i;

  s = strapp(s, "");

  if (!sts) return s;
  if (!sts->size) return s;

  for (i = 0; i < sts->size; i++)
  {
    if (i) s = strapp(s, " ");
    s = strapp(s, t = transform_to_xml(sts->arr[i]));
    free(t);
  }

  return s;
}

static char *transform_to_xml(svg_transform *st)
{
  char *s = NULL;
  char *t = NULL;

  s = strapp(s, "");

  if (!st) return s;

  switch (st->type)
  {
    case svg_transform_type_matrix: t = transform_matrix_to_xml(&st->matrix); break;
    case svg_transform_type_translate: t = transform_translate_to_xml(&st->translate); break;
    case svg_transform_type_scale: t = transform_scale_to_xml(&st->scale); break;
    case svg_transform_type_rotate: t = transform_rotate_to_xml(&st->rotate); break;
    case svg_transform_type_skew_x: t = transform_skewX_to_xml(&st->skew_x); break;
    case svg_transform_type_skew_y: t = transform_skewY_to_xml(&st->skew_y); break;

    case svg_transform_type_none:
    default: break;
  }

  if (t)
  {
    s = strapp(s, t);
    free(t);
  }

  return s;
}

static char *text_length_to_xml(svg_text_length *tl)
{
  char *s = NULL;
  char tbuf[256];

  s = strapp(s, "");

  if (!tl) return s;

  sprintf(tbuf, "%g", tl->val);
  s = strapp(s, tbuf);

  switch (tl->type)
  {
    case svg_text_length_type_none: break;

    case svg_text_length_type_ems: s = strapp(s, "ems"); break;
    case svg_text_length_type_exs: s = strapp(s, "exs"); break;
    case svg_text_length_type_px: s = strapp(s, "px"); break;
    case svg_text_length_type_cm: s = strapp(s, "cm"); break;
    case svg_text_length_type_mm: s = strapp(s, "mm"); break;
    case svg_text_length_type_in: s = strapp(s, "in"); break;
    case svg_text_length_type_pc: s = strapp(s, "pc"); break;
    case svg_text_length_type_pt: s = strapp(s, "pt"); break;
    case svg_text_length_type_percentage: s = strapp(s, "%%\""); break;
  }

  return s;
}

static char *length_adjust_type_to_xml(svg_length_adjust_type lat)
{
  char *s = NULL;

  switch (lat)
  {
    case svg_length_adjust_type_spacing:
      s = strapp(s, "spacing");
      break;
    case svg_length_adjust_type_spacing_and_glyphs:
      s = strapp(s, "spacingAndGlyphs");
      break;
    case svg_length_adjust_type_none: break;
  }

  return s;
}

static char *spacing_type_to_xml(svg_spacing_type st)
{
  char *s = NULL;

  s = strapp(s, "spacing=\"");
  switch (st)
  {
    case svg_spacing_type_auto: s = strapp(s, "auto"); break;
    case svg_spacing_type_exact: s = strapp(s, "exact"); break;
    case svg_spacing_type_none: break;
  }
  s = strapp(s, "\"");

  return s;
}

static char *method_type_to_xml(svg_method_type mt)
{
  char *s = NULL;

  switch (mt)
  {
    case svg_method_type_align: s = strapp(s, "align"); break;
    case svg_method_type_stretch: s = strapp(s, "stretch"); break;
    case svg_method_type_none: break;
  }

  return s;
}

static char *font_weight_type_to_xml(svg_font_weight_type fw)
{
  char *s = NULL;

  s = strapp(s, "font-weight:");
  switch (fw)
  {
    case svg_font_weight_type_normal: s = strapp(s, "normal"); break;
    case svg_font_weight_type_bolder: s = strapp(s, "bolder"); break;
    case svg_font_weight_type_bold: s = strapp(s, "bold"); break;
    case svg_font_weight_type_lighter: s = strapp(s, "lighter"); break;
    case svg_font_weight_type_100: s = strapp(s, "100"); break;
    case svg_font_weight_type_200: s = strapp(s, "200"); break;
    case svg_font_weight_type_300: s = strapp(s, "300"); break;
    case svg_font_weight_type_400: s = strapp(s, "400"); break;
    case svg_font_weight_type_500: s = strapp(s, "500"); break;
    case svg_font_weight_type_600: s = strapp(s, "600"); break;
    case svg_font_weight_type_700: s = strapp(s, "700"); break;
    case svg_font_weight_type_800: s = strapp(s, "800"); break;
    case svg_font_weight_type_900: s = strapp(s, "900"); break;
  }

  return s;
}

static char *font_stretch_type_to_xml(svg_font_stretch_type fs)
{
  char *s = NULL;

  s = strapp(s, "font-stretch:");
  switch (fs)
  {
    case svg_font_stretch_type_normal: s = strapp(s, "normal"); break;
    case svg_font_stretch_type_ultra_condensed: s = strapp(s, "ultra-condensed"); break;
    case svg_font_stretch_type_extra_condensed: s = strapp(s, "extra-condensed"); break;
    case svg_font_stretch_type_condensed: s = strapp(s, "condensed"); break;
    case svg_font_stretch_type_semi_condensed: s = strapp(s, "semi-condensed"); break;
    case svg_font_stretch_type_semi_expanded: s = strapp(s, "semi-expanded"); break;
    case svg_font_stretch_type_expanded: s = strapp(s, "expanded"); break;
    case svg_font_stretch_type_extra_expanded: s = strapp(s, "extra-expanded"); break;
    case svg_font_stretch_type_ultra_expanded: s = strapp(s, "ultra-expanded"); break;
  }

  return s;
}

static char *font_style_type_to_xml(svg_font_style_type fs)
{
  char *s = NULL;

  s = strapp(s, "font-style:");
  switch (fs)
  {
    case svg_font_style_type_normal: s = strapp(s, "normal"); break;
    case svg_font_style_type_italic: s = strapp(s, "italic"); break;
    case svg_font_style_type_oblique: s = strapp(s, "oblique"); break;
  }

  return s;
}

static char *orient_to_xml(svg_orient *o)
{
  char *str = NULL;
  char tbuf[256];

  if (!o) goto exit;

  memset(tbuf, 0, 256);

  switch (o->type)
  {
    case svg_orient_type_auto:
      str = strapp(str, "auto");
      break;
    case svg_orient_type_auto_start_reverse:
      str = strapp(str, "auto-start-reverse");
      break;
    case svg_orient_type_degrees:
    case svg_orient_type_radians:
    case svg_orient_type_gradians:
    case svg_orient_type_turns:
      sprintf(tbuf, "%g", o->val);
      str = strapp(str, tbuf);
      switch (o->type)
      {
        case svg_orient_type_degrees: str = strapp(str, "deg"); break;
        case svg_orient_type_radians: str = strapp(str, "rad"); break;
        case svg_orient_type_gradians: str = strapp(str, "grad"); break;
        case svg_orient_type_turns: str = strapp(str, "turns"); break;
        default: break;
      }
      break;
  }

exit:
  return str;
}

static svg_elements *parse_elements(xml_elements *es)
{
  svg_elements *ses = NULL;
  xml_element *e;
  svg_element *se;

  if (!es) goto exit;

  e = xml_elements_first(es);
  while (e)
  {
    se = parse_element(e);
    if (se)
    {
      if (!ses) ses = svg_elements_new();
      if (!ses) break;
      svg_elements_add(ses, se);
      svg_element_free(se);
    }
    e = xml_elements_next(es);
  }

exit:
  return ses;
}

static svg_element *parse_element(xml_element *e)
{
  svg_element *se = NULL;
  svg_style *st = NULL;
  svg_transforms *ts = NULL;
  xml_attribute *a = NULL;

  if (!e) goto exit;
  if (!e->name) goto exit;

  if (!strcmp(e->name, "rect")) se = parse_rect(e);
  else if (!strcmp(e->name, "circle")) se = parse_circle(e);
  else if (!strcmp(e->name, "ellipse")) se = parse_ellipse(e);
  else if (!strcmp(e->name, "line")) se = parse_line(e);
  else if (!strcmp(e->name, "polygon")) se = parse_polygon(e);
  else if (!strcmp(e->name, "polyline")) se = parse_polyline(e);
  else if (!strcmp(e->name, "path")) se = parse_path(e);
  else if (!strcmp(e->name, "text")) se = parse_text(e);
  else if (!strcmp(e->name, "textpath")) se = parse_textpath(e);
  else if (!strcmp(e->name, "link")) se = parse_link(e);
  else if (!strcmp(e->name, "image")) se = parse_image(e);
  else if (!strcmp(e->name, "marker")) se = parse_marker(e);

  if (!se) goto exit;

  a = xml_attributes_find(e->attributes, "id");
  if (a) svg_element_set_id(se, a->value);

  a = xml_attributes_find(e->attributes, "style");
  if (a)
  {
    st = parse_style(a->value);
    svg_element_set_style(se, st);
  }

  a = xml_attributes_find(e->attributes, "transform");
  if (a)
  {
    ts = parse_transforms(a->value);
    svg_element_set_transforms(se, ts);
  }

exit:
  if (st) svg_style_free(st);
  if (ts) svg_transforms_free(ts);

  return se;
}

static svg_element *parse_rect(xml_element *e)
{
  svg_element *se = NULL;
  svg_rect *r = NULL;
  xml_attribute *a;
  svg_point pt;

  if (!e) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  se = svg_element_new();
  if (!se) goto exit;

  r = svg_rect_new();
  if (!r) goto exit;

  if ((a = xml_attributes_find(e->attributes, "width")))
    svg_rect_set_width(r, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "height")))
    svg_rect_set_height(r, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "x")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "y")))
    svg_point_set_y(&pt, atof(a->value));

  svg_rect_set_point(r, &pt);

  if ((a = xml_attributes_find(e->attributes, "rx")))
    svg_rect_set_rx(r, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "ry")))
    svg_rect_set_ry(r, atof(a->value));

  svg_element_set_element(se, svg_element_type_rect, r);

exit:
  if (r) svg_rect_free(r);

  return se;
}

static svg_element *parse_circle(xml_element *e)
{
  svg_element *se = NULL;
  svg_circle *c = NULL;
  xml_attribute *a;
  svg_point pt;

  if (!e) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  se = svg_element_new();
  if (!se) goto exit;

  c = svg_circle_new();
  if (!c) goto exit;

  if ((a = xml_attributes_find(e->attributes, "r")))
    svg_circle_set_r(c, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "cx")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "cy")))
    svg_point_set_y(&pt, atof(a->value));

  svg_circle_set_c(c, &pt);

  svg_element_set_element(se, svg_element_type_circle, c);

exit:
  if (c) svg_circle_free(c);

  return se;
}

static svg_element *parse_ellipse(xml_element *e)
{
  svg_element *se = NULL;
  svg_ellipse *ep = NULL;
  xml_attribute *a;
  svg_point pt;

  if (!e) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  se = svg_element_new();
  if (!se) goto exit;

  ep = svg_ellipse_new();
  if (!ep) goto exit;

  if ((a = xml_attributes_find(e->attributes, "rx")))
    svg_ellipse_set_rx(ep, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "ry")))
    svg_ellipse_set_rx(ep, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "cx")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "cy")))
    svg_point_set_y(&pt, atof(a->value));

  svg_ellipse_set_c(ep, &pt);

  svg_element_set_element(se, svg_element_type_ellipse, ep);

exit:
  if (ep) svg_ellipse_free(ep);

  return se;
}

static svg_element *parse_line(xml_element *e)
{
  svg_element *se = NULL;
  svg_line *l = NULL;
  xml_attribute *a;
  svg_point pt;

  if (!e) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  se = svg_element_new();
  if (!se) goto exit;

  l = svg_line_new();
  if (!l) goto exit;

  if ((a = xml_attributes_find(e->attributes, "x1")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "y1")))
    svg_point_set_y(&pt, atof(a->value));

  svg_line_set_p1(l, &pt);

  if ((a = xml_attributes_find(e->attributes, "x2")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "y2")))
    svg_point_set_y(&pt, atof(a->value));

  svg_line_set_p2(l, &pt);

  svg_element_set_element(se, svg_element_type_line, l);

exit:
  if (l) svg_line_free(l);

  return se;
}

static svg_element *parse_polygon(xml_element *e)
{
  svg_element *se = NULL;
  svg_polygon *pg = NULL;
  svg_points *pts = NULL;
  xml_attribute *a;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  pg = svg_polygon_new();
  if (!pg) goto exit;

  if ((a = xml_attributes_find(e->attributes, "points")))
    pts = parse_points(a->value);

  if (pts) svg_polygon_set_points(pg, pts);

  svg_element_set_element(se, svg_element_type_polygon, pg);

exit:
  if (pts) svg_points_free(pts);
  if (pg) svg_polygon_free(pg);

  return se;
}

static svg_element *parse_polyline(xml_element *e)
{
  svg_element *se = NULL;
  svg_polyline *pl = NULL;
  svg_points *pts = NULL;
  xml_attribute *a;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  pl = svg_polyline_new();
  if (!pl) goto exit;

  if ((a = xml_attributes_find(e->attributes, "points")))
    pts = parse_points(a->value);

  if (pts) svg_polyline_set_points(pl, pts);

  svg_element_set_element(se, svg_element_type_polyline, pl);

exit:
  if (pts) svg_points_free(pts);
  if (pl) svg_polyline_free(pl);

  return se;
}

static svg_element *parse_path(xml_element *e)
{
  svg_element *se = NULL;
  svg_path *p = NULL;
  xml_attribute *a;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  p = svg_path_new();
  if (!p) goto exit;

  if ((a = xml_attributes_find(e->attributes, "d")))
    svg_path_set_d(p, a->value);

  svg_element_set_element(se, svg_element_type_path, p);

exit:
  if (p) svg_path_free(p);

  return se;
}

static svg_element *parse_text(xml_element *e)
{
  svg_element *se = NULL;
  svg_text *t = NULL;
  svg_text_length *tl = NULL;
  xml_attribute *a;
  svg_point pt;

  if (!e) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  se = svg_element_new();
  if (!se) goto exit;

  t = svg_text_new();
  if (!t) goto exit;

  if ((a = xml_attributes_find(e->attributes, "x")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "y")))
    svg_point_set_y(&pt, atof(a->value));

  svg_text_set_p(t, &pt);

  if ((a = xml_attributes_find(e->attributes, "dx")))
    svg_text_set_dx(t, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "dy")))
    svg_text_set_dy(t, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "rotate")))
    svg_text_set_rotate(t, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "textLength")))
  {
    tl = parse_text_length(a->value);
    svg_text_set_text_length(t, tl);
    svg_text_length_free(tl);
    tl = NULL;
  }

  if ((a = xml_attributes_find(e->attributes, "lengthAdjust")))
    svg_text_set_length_adjust(t, parse_length_adjust(a->value));

  svg_text_set_contents(t, xml_element_get_text(e));

  svg_element_set_element(se, svg_element_type_text, t);

exit:
  if (tl) svg_text_length_free(tl);
  if (t) svg_text_free(t);

  return se;
}

static svg_element *parse_textpath(xml_element *e)
{
  svg_element *se = NULL;
  svg_textpath *tp = NULL;
  xml_attribute *a;
  svg_text_length *tl = NULL;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  tp = svg_textpath_new();
  if (!tp) goto exit;

  if ((a = xml_attributes_find(e->attributes, "href")))
    svg_textpath_set_href(tp, a->value);

  if ((a = xml_attributes_find(e->attributes, "lengthAdjust")))
    svg_textpath_set_length_adjust(tp, parse_length_adjust(a->value));

  if ((a = xml_attributes_find(e->attributes, "method")))
    svg_textpath_set_method(tp, parse_method(a->value));

  if ((a = xml_attributes_find(e->attributes, "startOffset")))
  {
    tl = parse_text_length(a->value);
    svg_textpath_set_start_offset(tp, tl);
    svg_text_length_free(tl);
    tl = NULL;
  }

  if ((a = xml_attributes_find(e->attributes, "textLength")))
  {
    tl = parse_text_length(a->value);
    svg_textpath_set_text_length(tp, tl);
    svg_text_length_free(tl);
    tl = NULL;
  }

  svg_textpath_set_contents(tp, xml_element_get_text(e));

  svg_element_set_element(se, svg_element_type_textpath, tp);

exit:
  if (tl) svg_text_length_free(tl);
  if (tp) svg_textpath_free(tp);

  return se;
}

static svg_element *parse_link(xml_element *e)
{
  svg_element *se = NULL;
  svg_link *lnk = NULL;
  svg_elements *ses = NULL;
  xml_attribute *a;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  lnk = svg_link_new();
  if (!lnk) goto exit;

  if ((a = xml_attributes_find(e->attributes, "href")))
    svg_link_set_href(lnk, a->value);

  if ((a = xml_attributes_find(e->attributes, "download")))
    svg_link_set_download(lnk, a->value);

  if ((a = xml_attributes_find(e->attributes, "hreflang")))
    svg_link_set_hreflang(lnk, a->value);

  if ((a = xml_attributes_find(e->attributes, "referrerpolicy")))
    svg_link_set_referrer_policy(lnk, a->value);

  if ((a = xml_attributes_find(e->attributes, "rel")))
    svg_link_set_rel(lnk, a->value);

  if ((a = xml_attributes_find(e->attributes, "target")))
    svg_link_set_target(lnk, a->value);

  if ((a = xml_attributes_find(e->attributes, "type")))
    svg_link_set_type(lnk, a->value);

  ses = parse_elements(e->elements);

  svg_link_set_elements(lnk, ses);

  svg_element_set_element(se, svg_element_type_link, lnk);

exit:
  if (ses) svg_elements_free(ses);
  if (lnk) svg_link_free(lnk);

  return se;
}

static svg_element *parse_image(xml_element *e)
{
  svg_element *se = NULL;
  svg_image *img = NULL;
  svg_point pt;
  xml_attribute *a;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  img = svg_image_new();
  if (!img) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  if ((a = xml_attributes_find(e->attributes, "width")))
    svg_image_set_width(img, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "height")))
    svg_image_set_height(img, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "href")))
    svg_image_set_href(img, a->value);

  if ((a = xml_attributes_find(e->attributes, "x")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "y")))
    svg_point_set_y(&pt, atof(a->value));

  svg_image_set_p(img, &pt);

  svg_element_set_element(se, svg_element_type_image, img);

exit:
  if (img) svg_image_free(img);

  return se;
}

static svg_element *parse_marker(xml_element *e)
{
  svg_element *se = NULL;
  svg_marker *m = NULL;
  svg_elements *ses = NULL;
  svg_orient *or = NULL;
  svg_point pt;
  xml_attribute *a;

  if (!e) goto exit;

  se = svg_element_new();
  if (!se) goto exit;

  m = svg_marker_new();
  if (!m) goto exit;

  memset(&pt, 0, sizeof(svg_point));

  if ((a = xml_attributes_find(e->attributes, "markerWidth")))
    svg_marker_set_marker_width(m, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "markerHeight")))
    svg_marker_set_marker_height(m, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "refX")))
    svg_point_set_x(&pt, atof(a->value));

  if ((a = xml_attributes_find(e->attributes, "refY")))
    svg_point_set_y(&pt, atof(a->value));

  svg_marker_set_ref(m, &pt);

  if ((a = xml_attributes_find(e->attributes, "orient")))
    or = parse_orient(a->value);

  ses = parse_elements(e->elements);

  svg_marker_set_elements(m, ses);

  svg_element_set_element(se, svg_element_type_marker, m);

exit:
  if (m) svg_marker_free(m);
  if (or) svg_orient_free(or);
  if (ses) svg_elements_free(ses);

  return se;
}

static svg_points *parse_points(char *s)
{
  svg_points *pts = NULL;
  svg_point *pt = NULL;
  char *b, *e, *c;
  char part[50];

  if (!s) goto exit;

  pts = svg_points_new();
  if (!pts) goto exit;

  b = s;
  while (*b)
  {
    e = b;
    while (*e && (*e != ' ') && (*e != '\t') && (*e != '\n')) ++e;

    c = b;
    while ((c < e) && (*c != ',')) ++c;
    if (c >= e) break;
    ++c;

    pt = svg_point_new();
    if (!pt) break;

    memset(part, 0, 50);
    strncpy(part, b, (int)(c - b));
    svg_point_set_x(pt, atof(part));

    memset(part, 0, 50);
    strncpy(part, c, (int)(e - c));
    svg_point_set_y(pt, atof(part));

    svg_points_add(pts, pt);

    svg_point_free(pt);
    pt = NULL;

    b = e;
    if (*b) ++b;
  }

exit:
  return pts;
}

static svg_text_length *parse_text_length(char *s)
{
  svg_text_length *tl = NULL;
  char *end = NULL;

  if (!s) goto exit;

  tl = svg_text_length_new();
  if (!tl) goto exit;

  tl->val = strtod(s, &end);

  if (end)
  {
    if (!strcmp(end, "%")) tl->type = svg_text_length_type_percentage;
    else if (!strcmp(end, "ems")) tl->type = svg_text_length_type_ems;
    else if (!strcmp(end, "exs")) tl->type = svg_text_length_type_exs;
    else if (!strcmp(end, "px")) tl->type = svg_text_length_type_px;
    else if (!strcmp(end, "cm")) tl->type = svg_text_length_type_cm;
    else if (!strcmp(end, "mm")) tl->type = svg_text_length_type_mm;
    else if (!strcmp(end, "in")) tl->type = svg_text_length_type_in;
    else if (!strcmp(end, "pc")) tl->type = svg_text_length_type_pc;
    else if (!strcmp(end, "pt")) tl->type = svg_text_length_type_pt;
  }

exit:

  return tl;
}

static svg_length_adjust_type parse_length_adjust(char *s)
{
  if (!s) return svg_length_adjust_type_spacing;

  if (!strcmp(s, "spacing")) return svg_length_adjust_type_spacing;
  if (!strcmp(s, "spacingAndGlyphs")) return svg_length_adjust_type_spacing_and_glyphs;

  return svg_length_adjust_type_spacing;
}

static svg_method_type parse_method(char *s)
{
  if (!s) return svg_method_type_align;

  if (!strcmp(s, "align")) return svg_method_type_align;
  if (!strcmp(s, "stretch")) return svg_method_type_stretch;

  return svg_method_type_align;
}

static svg_orient *parse_orient(char *s)
{
  svg_orient *or = NULL;
  char *end = NULL;

  if (!s) goto exit;

  or = svg_orient_new();
  if (!or) goto exit;

  if (!strcmp(s, "auto"))
  {
    or->type = svg_orient_type_auto;
    goto exit;
  }

  if (!strcmp(s, "auto-start-reverse"))
  {
    or->type = svg_orient_type_auto_start_reverse;
    goto exit;
  }

  or->val = strtod(s, &end);

  if (end)
  {
    if (!strcmp(end, "deg")) or->type = svg_orient_type_degrees;
    else if (!strcmp(end, "rad")) or->type = svg_orient_type_radians;
    else if (!strcmp(end, "grad")) or->type = svg_orient_type_gradians;
    else if (!strcmp(end, "turn")) or->type = svg_orient_type_turns;
  }

exit:
  return or;
}

static svg_transforms *parse_transforms(char *s)
{
  svg_transforms *ts = NULL;
  svg_transform *t = NULL;
  char *b;
  int len = 0;

  if (!s) goto exit;

  b = s;
  while ((t = parse_transform(b, &len)))
  {
    if (!ts) ts = svg_transforms_new();

    svg_transforms_add(ts, t);
    svg_transform_free(t);

    b += len;
 
    eat_whitespace(&b);
  }

exit:
  return ts;
}

static svg_transform *parse_transform(char *s, int *len)
{
  svg_transform *t = NULL;
  char *b;
  int nlen = 0;
  int bad_parse = 0;
  double n;

  if (!s) goto exit;

  b = s;

  eat_whitespace(&b);

  if (!*b) goto exit;

  t = svg_transform_new();
  if (!t) goto exit;

  if (!strcmp(b, "matrix"))
  {
    svg_transform_set_type(t, svg_transform_type_matrix);
    b += 6;
    eat_whitespace(&b);
    if (*b != '(') goto bad;
    eat_whitespace(&b);
    n = parse_number(b, &nlen);
    svg_transform_matrix_set_a(&t->matrix, n);
    b += nlen;
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_matrix_set_b(&t->matrix, n);
      b += nlen;
    }
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_matrix_set_c(&t->matrix, n);
      b += nlen;
    }
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_matrix_set_d(&t->matrix, n);
      b += nlen;
    }
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_matrix_set_e(&t->matrix, n);
      b += nlen;
    }
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_matrix_set_f(&t->matrix, n);
      b += nlen;
    }
  }
  else if (!strcmp(b, "translate"))
  {
    svg_transform_set_type(t, svg_transform_type_translate);
    b += 9;
    eat_whitespace(&b);
    if (*b != '(') goto bad;
    eat_whitespace(&b);
    n = parse_number(b, &nlen);
    svg_transform_translate_set_x(&t->translate, n);
    b += nlen;
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_translate_set_y(&t->translate, n);
      b += nlen;
    }
    eat_whitespace(&b);
    if (*b != ')') goto bad;
  }
  else if (!strcmp(b, "scale"))
  {
    svg_transform_set_type(t, svg_transform_type_scale);
    b += 5;
    eat_whitespace(&b);
    if (*b != '(') goto bad;
    eat_whitespace(&b);
    n = parse_number(b, &nlen);
    svg_transform_scale_set_x(&t->scale, n);
    b += nlen;
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_scale_set_y(&t->scale, n);
      b += nlen;
    }
    eat_whitespace(&b);
    if (*b != ')') goto bad;
  }
  else if (!strcmp(b, "rotate"))
  {
    svg_transform_set_type(t, svg_transform_type_rotate);
    b += 6;
    eat_whitespace(&b);
    if (*b != '(') goto bad;
    eat_whitespace(&b);
    n = parse_number(b, &nlen);
    svg_transform_rotate_set_a(&t->rotate, n);
    b += nlen;
    eat_whitespace(&b);
    if (*b != ')')
    {
      n = parse_number(b, &nlen);
      svg_transform_rotate_set_x(&t->rotate, n);
      b += nlen;
      eat_whitespace(&b);
      if (*b != ')')
      {
        n = parse_number(b, &nlen);
        svg_transform_rotate_set_y(&t->rotate, n);
        b += nlen;
      }
    }
    eat_whitespace(&b);
    if (*b != ')') goto bad;
  }
  else if (!strcmp(b, "skewX"))
  {
    svg_transform_set_type(t, svg_transform_type_skew_x);
    b += 5;
    eat_whitespace(&b);
    if (*b != '(') goto bad;
    eat_whitespace(&b);
    n = parse_number(b, &nlen);
    svg_transform_skewX_set_a(&t->skew_x, n);
    b += nlen;
    eat_whitespace(&b);
    if (*b != ')') goto bad;
  }
  else if (!strcmp(b, "skewY"))
  {
    svg_transform_set_type(t, svg_transform_type_skew_y);
    b += 5;
    eat_whitespace(&b);
    if (*b != '(') goto bad;
    eat_whitespace(&b);
    n = parse_number(b, &nlen);
    svg_transform_skewY_set_a(&t->skew_y, n);
    b += nlen;
    eat_whitespace(&b);
    if (*b != ')') goto bad;
  }
  else goto bad;

  if (len) *len = b - s;

  goto exit;

bad:
  bad_parse = 1;

exit:
  if (bad_parse && t)
  {
    svg_transform_free(t);
    t = NULL;
  }

  return t;
}

static double parse_number(char *s, int *len)
{
  double n = 0.0;
  char *b;
  char num[256];
  int count = 0;

  if (!s) goto exit;

  memset(num, 0, 256);

  b = s;

  eat_whitespace(&b);

  count = 0;
  while (*b && (isdigit(*b) || (*b == '+') || (*b == '-') || (*b == '.')) && (count < 256))
  {
    num[count] = *b;
    ++count;
    ++b;
  }

  if (len) *len = b - s;

  n = strtod(num, NULL);

exit:
  return n;
}

static void eat_whitespace(char **s)
{
  if (!s || !*s) return;

  while (**s && ((**s == ' ') || (**s == '\t') || (**s == '\n'))) ++*s;
}

static svg_style *parse_style(char *s)
{
  svg_style *st = NULL;
  char *b;
  int len = 0;

  if (!s) goto exit;

  st = svg_style_new();
  if (!st) goto exit;

  b = s;

  eat_whitespace(&b);

  while (parse_style_bit(st, b, &len))
  {
    b += len;
  }
  
exit:
  return st;
}

static int parse_style_bit(svg_style *st, char *b, int *len)
{
  char *end;
  char *bit = NULL; 
  char *attribute;
  char *value;
  int retval = 0;
  char *cp;
  double n;
  svg_fill_rule_type frt;
  svg_stroke_linecap_type slc;
  svg_stroke_linejoin_type slj;
  svg_font_weight_type fw;
  svg_font_stretch_type fs;
  svg_font_style_type fst;

  if (!st || !b) goto exit;

  eat_whitespace(&b);

  end = b;
  while (*end && (*end != ';')) ++end;
  *len = end - b;

  if (!*len) goto exit;

  bit = strndup(b, *len);
  if (!bit) goto exit;

  attribute = value = bit;

  while (*value && (*value != ':')) ++value;
  if (*value)
  {
    *value = 0;
    ++value;
  }

    // strip end of 'attribute' part
  cp = attribute - 1;
  while (cp >= attribute)
  {
    if (!*cp || ((*cp != ' ')  && (*cp != '\t') && (*cp != '\n'))) break;
    *cp = 0;
    --cp;
  }

    //strip beginning of 'value' part
  cp = value;
  while (*cp && ((*cp == ' ') || (*cp == '\t') || (*cp == '\n'))) ++cp;
  value = cp;

    //strip end of 'value' part
  cp = value;
  while (*cp) ++cp;
  while (cp >= value)
  {
    if (!*cp || ((*cp != ' ') && (*cp != '\t') && (*cp != '\n'))) break;
    *cp = 0;
    --cp;
  }

  if (!strlen(attribute)) goto exit;
  if (!strlen(value)) goto exit;

  if (!strcmp(attribute, "fill"))
    svg_style_set_fill(st, value);
  else if (!strcmp(attribute, "fill-opacity"))
  {
    n = strtod(value, NULL);
    svg_style_set_fill_opacity(st, n);
  }
  else if (!strcmp(attribute, "fill-rule"))
  {
    frt = svg_fill_rule_type_nonzero;
    if (!strcmp(value, "evenodd")) frt = svg_fill_rule_type_evenodd;
    svg_style_set_fill_rule(st, frt);
  }
  else if (!strcmp(attribute, "stroke"))
    svg_style_set_stroke(st, value);
  else if (!strcmp(attribute, "stroke-width"))
  {
    n = strtod(value, NULL);
    svg_style_set_fill_opacity(st, n);
  }
  else if (!strcmp(attribute, "stroke-opacity"))
  {
    n = strtod(value, NULL);
    svg_style_set_fill_opacity(st, n);
  }
  else if (!strcmp(attribute, "stroke-linecap"))
  {
    slc = svg_stroke_linecap_type_butt;
    if (!strcmp(value, "round")) slc = svg_stroke_linecap_type_round;
    else if (!strcmp(value, "square")) slc = svg_stroke_linecap_type_square;
    svg_style_set_stroke_linecap(st, slc);
  }
  else if (!strcmp(attribute, "stroke-dasharray"))
    svg_style_set_stroke_dash_array(st, value);
  else if (!strcmp(attribute, "stroke-linejoin"))
  {
    slj = svg_stroke_linejoin_type_miter;
    if (!strcmp(value, "arcs")) slj = svg_stroke_linejoin_type_arcs;
    else if (!strcmp(value, "bevel")) slj = svg_stroke_linejoin_type_bevel;
    else if (!strcmp(value, "miter-clip")) slj = svg_stroke_linejoin_type_miter_clip;
    else if (!strcmp(value, "round")) slj = svg_stroke_linejoin_type_round;
    svg_style_set_stroke_linejoin(st, slj);
  }
  else if (!strcmp(attribute, "background-color"))
    svg_style_set_background_color(st, value);
  else if (!strcmp(attribute, "font-family"))
    svg_style_set_font_family(st, value);
  else if (!strcmp(attribute, "font-weight"))
  {
    fw = svg_font_weight_type_normal;
    if (!strcmp(value, "bolder")) fw = svg_font_weight_type_bolder;
    else if (!strcmp(value, "bold")) fw = svg_font_weight_type_bold;
    else if (!strcmp(value, "lighter")) fw = svg_font_weight_type_lighter;
    else if (!strcmp(value, "100")) fw = svg_font_weight_type_100;
    else if (!strcmp(value, "200")) fw = svg_font_weight_type_200;
    else if (!strcmp(value, "300")) fw = svg_font_weight_type_300;
    else if (!strcmp(value, "400")) fw = svg_font_weight_type_400;
    else if (!strcmp(value, "500")) fw = svg_font_weight_type_500;
    else if (!strcmp(value, "600")) fw = svg_font_weight_type_600;
    else if (!strcmp(value, "700")) fw = svg_font_weight_type_700;
    else if (!strcmp(value, "800")) fw = svg_font_weight_type_800;
    else if (!strcmp(value, "900")) fw = svg_font_weight_type_900;
    svg_style_set_font_weight(st, fw);
  }
  else if (!strcmp(attribute, "font-stretch"))
  {
    fs = svg_font_stretch_type_normal;
    if (!strcmp(value, "ultra-condensed")) fs = svg_font_stretch_type_ultra_condensed;
    else if (!strcmp(value, "ultra-condensed")) fs = svg_font_stretch_type_ultra_condensed;
    else if (!strcmp(value, "extra-condensed")) fs = svg_font_stretch_type_extra_condensed;
    else if (!strcmp(value, "condensed")) fs = svg_font_stretch_type_condensed;
    else if (!strcmp(value, "semi-condensed")) fs = svg_font_stretch_type_semi_condensed;
    else if (!strcmp(value, "semi-expanded")) fs = svg_font_stretch_type_semi_expanded;
    else if (!strcmp(value, "expanded")) fs = svg_font_stretch_type_expanded;
    else if (!strcmp(value, "extra-expanded")) fs = svg_font_stretch_type_extra_expanded;
    else if (!strcmp(value, "ultra-expanded")) fs = svg_font_stretch_type_ultra_expanded;
    svg_style_set_font_stretch(st, fs);
  }
  else if (!strcmp(attribute, "font-style"))
  {
    fst = svg_font_style_type_normal;
    if (!strcmp(value, "italic")) fs = svg_font_style_type_italic;
    else if (!strcmp(value, "italic")) fs = svg_font_style_type_oblique;
    svg_style_set_font_style(st, fst);
  }
  else if (!strcmp(attribute, "font-size"))
    svg_style_set_font_size(st, value);
  else goto exit;

  retval = 1;

exit:
  if (bit) free(bit);

  return retval;
}

static svg *xml_to_svg(xml *x)
{
  svg *s = NULL;
  xml_element *e;
  xml_attribute *a;
  xml_attributes *as;
  svg_style *st = NULL;
  svg_elements *ses;

  if (!x) goto exit;

  e = xml_get_root(x);
  if (!e) goto exit;
  if (!e->name) goto exit;

  if (strcmp(e->name, "svg")) goto exit;

  s = svg_new();

  if (e->attributes)
  {
    as = e->attributes;
    a = xml_attributes_find(as, "width");
    if (a) svg_set_width(s, atoi(xml_attribute_get_value(a)));
    a = xml_attributes_find(as, "height");
    if (a) svg_set_height(s, atoi(xml_attribute_get_value(a)));
    a = xml_attributes_find(as, "xmlns");
    if (a) svg_set_xmlns(s, xml_attribute_get_value(a));
    a = xml_attributes_find(as, "style");
    if (a) svg_set_style(s, st = parse_style(xml_attribute_get_value(a)));
    if (st) svg_style_free(st);
  }

  ses = parse_elements(e->elements);
  if (ses)
  {
    svg_set_elements(s, ses);
    svg_elements_free(ses);
  }

exit:
  return s;
}

