/* -*- mode: c; style: linux -*- */

/* row-block-layout.c
 * Copyright (C) 2000 Helix Code, Inc.
 *
 * Written by Bradford Hovinen <hovinen@helixcode.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "row-block-layout.h"

enum {
	ARG_0,
	ARG_SAMPLE
};

struct _RowBlockLayoutPrivate 
{
	/* Private data members */
};

static BlockLayoutClass *parent_class;

static void row_block_layout_init        (RowBlockLayout *row_block_layout);
static void row_block_layout_class_init  (RowBlockLayoutClass *class);

static void row_block_layout_set_arg     (GtkObject *object, 
					   GtkArg *arg, 
					   guint arg_id);
static void row_block_layout_get_arg     (GtkObject *object, 
					   GtkArg *arg, 
					   guint arg_id);

static void row_block_layout_finalize    (GtkObject *object);

static void row_block_layout_draw    (MathExpression *expression,
					Renderer *renderer,
					double x, double y);

static void row_block_layout_get_size (MathExpression *expression,
					Renderer *renderer,
					double *width,double *ascent,
					double *descent);


guint
row_block_layout_get_type (void)
{
	static guint row_block_layout_type = 0;

	if (!row_block_layout_type) {
		GtkTypeInfo row_block_layout_info = {
			"RowBlockLayout",
			sizeof (RowBlockLayout),
			sizeof (RowBlockLayoutClass),
			(GtkClassInitFunc) row_block_layout_class_init,
			(GtkObjectInitFunc) row_block_layout_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		row_block_layout_type = 
			gtk_type_unique (block_layout_get_type (), 
					 &row_block_layout_info);
	}

	return row_block_layout_type;
}

static void
row_block_layout_init (RowBlockLayout *row_block_layout)
{
	row_block_layout->p = g_new0 (RowBlockLayoutPrivate, 1);
}

static void
row_block_layout_class_init (RowBlockLayoutClass *class) 
{
	GtkObjectClass *object_class;

	gtk_object_add_arg_type ("RowBlockLayout::sample",
				 GTK_TYPE_POINTER,
				 GTK_ARG_READWRITE,
				 ARG_SAMPLE);

	object_class = GTK_OBJECT_CLASS (class);
	object_class->finalize = row_block_layout_finalize;
	object_class->set_arg = row_block_layout_set_arg;
	object_class->get_arg = row_block_layout_get_arg;

	parent_class = BLOCK_LAYOUT_CLASS
		(gtk_type_class (block_layout_get_type ()));
}

static void
row_block_layout_set_arg (GtkObject *object, GtkArg *arg, guint arg_id) 
{
	RowBlockLayout *row_block_layout;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ROW_BLOCK_LAYOUT (object));

	row_block_layout = ROW_BLOCK_LAYOUT (object);

	switch (arg_id) {
	case ARG_SAMPLE:
		break;

	default:
		g_warning ("Bad argument set");
		break;
	}
}

static void
row_block_layout_get_arg (GtkObject *object, GtkArg *arg, guint arg_id) 
{
	RowBlockLayout *row_block_layout;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ROW_BLOCK_LAYOUT (object));

	row_block_layout = ROW_BLOCK_LAYOUT (object);

	switch (arg_id) {
	case ARG_SAMPLE:
		break;

	default:
		g_warning ("Bad argument get");
		break;
	}
}

static void
row_block_layout_finalize (GtkObject *object) 
{
	RowBlockLayout *row_block_layout;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ROW_BLOCK_LAYOUT (object));

	row_block_layout = ROW_BLOCK_LAYOUT (object);

	g_free (row_block_layout->p);
}


static void
row_block_layout_draw (MathExpression *expression, Renderer *renderer,
			double x, double y)
{
   RowBlockLayout *row_block_layout = ((RowBlockLayout *)expression);
   Glist *children;
   

for(children=row_block_layout->children; children;
	children=g_list_next(children))
   {
      double thisx;
      row_block_layout_draw((MathExpression *)children->data,renderer,x,y);
      row_block_layout_get_size((MathExpression *)children->data,
				renderer, &thisx, NULL, NULL);
      x += thisx;
   }
}

static void
row_block_layout_get_size(MathExpression *expression,Renderer *renderer,
				double *width, double *ascent, double *descent)
{
   RowBlockLayout *row_block_layout = ((RowBlockLayout *)expression);
   Glist *children;
   double thisx, thisa, thisd;
   g_return_if_fail (expression != NULL);
   
   if(ascent)  *ascent = 0;
   if(descent) *descent = 0;
   if(width) *width = 0;

   if(width || ascent || descent)
   {   
  for(children=row_block->children;children;children=g_list_next(children))
      {
         row_block_layout_get_size(MathExpression *)children->data,renderer,
					&thisx,&thisa,&thisd);
         if(ascent && thisa > *ascent)  ascent = thisa;
         if(descent && thisd > *descent) descent = thisd;
         if(width)  *width += thisx;
      }
   }
}


GtkObject *
row_block_layout_new (void) 
{
	return gtk_object_new (row_block_layout_get_type (),
			       NULL);
}
