/* $File: //ASP/Dev/SBS/4_Controls/4_4_Equipment_Control/Common/sw/Interpolation/Dynamic_InterpolationSup/dynamic_interpolation.c $
 * $Revision: 1.1.1.1 $
 * $DateTime: 2012/06/07 10:13:13 $
 * Last checked in by: $Author: saa $
 *
 * Source code formatting:
 * indent -kr -pcs -i3 -cli3 -nbbo -nut  dynamic_interpolation.c
 *
 * Copyright (c) 2012  Australian Synchrotron
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * Licence as published by the Free Software Foundation; either
 * version 2.1 of the Licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public Licence for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * Licence along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact details:
 * andrew.starritt@synchrotron.vic.gov.au
 * 800 Blackburn Road, Clayton, Victoria 3168, Australia.
 */

#include <stdio.h>
#include <string.h>

#include <errlog.h>

#include "dynamic_interpolation.h"

/*------------------------------------------------------------------------------
 */
static void assert (Error_Text error_text,
                    const char *function, const int line, const char *msg)
{
   Error_Text temp;

   if (strlen (error_text) == 0) {
      snprintf (error_text, ERROR_TEXT_SIZE, "%s:%d: %s",
                function, line, msg);
   } else {
      snprintf (temp, ERROR_TEXT_SIZE, "%s:%d: %s =>\n   %s",
                function, line, msg, error_text);
      strncpy (error_text, temp, ERROR_TEXT_SIZE);
   }
}

#define ASSERT(condition, ...)  {                                     \
   if (!(condition)) {                                                \
      char buffer [100];                                              \
      snprintf (buffer, 100, __VA_ARGS__);                            \
      assert (error_text, __FUNCTION__, __LINE__,   buffer);          \
      return FALSE;                                                   \
   }                                                                  \
}


/*------------------------------------------------------------------------------
 */
static bool slice (Error_Text error_text,
                   sVariable_Dimension_Array * target,
                   const sVariable_Dimension_Array * source,
                   const int index)
{
   size_t n;
   int slice_size;
   int first;
   int d;

   ASSERT (target != NULL, "null target");

   ASSERT (source != NULL, "null source");
   ASSERT (source->data != NULL, "null source data");
   ASSERT ((source->ndims >= 1) &&
           (source->ndims <= MAXIMUM_DIMENSIONS), "ndims");

   ASSERT ((index >= 0) &&
           (index < source->dsize[0]), "slice index range");

   ASSERT (target != source, "slice into same object not allowed");

   slice_size = 1;
   for (d = 1; d < source->ndims; d++) {
      slice_size *= source->dsize[d];
   }
   first = index * slice_size;

   target->ndims = source->ndims - 1;
   n = target->ndims * sizeof (target->dsize[0]);
   memcpy (&(target->dsize[0]), &(source->dsize[1]), n);
   target->length = source->length - first;
   target->data = &(source->data[first]);

   return TRUE;
}                               /* slice */

/*------------------------------------------------------------------------------
 * This function extracts the data [i] value from a 0D or 1D variable
 * dimensional array object. Parameters index only used 1D arrays.
 */
static bool get_value (Error_Text error_text,
                       double *result,
                       const sVariable_Dimension_Array * vda,
                       const int index)
{
   *result = 0.0;

   ASSERT (vda != NULL, "null ");
   ASSERT (vda->data != NULL, "null data");
   ASSERT ((vda->ndims >= 0) && (vda->ndims <= 1), "num dims");

   if (vda->ndims == 0) {
      ASSERT (vda->length >= 1, "length");
      *result = vda->data[0];
   } else {
      ASSERT (vda->dsize[0] >= 0, "dim-size < 0");
      ASSERT ((index >= 0) &&
              (index < vda->dsize[0]), "out of bounds index");
      ASSERT (vda->length >= vda->dsize[0],
              "available data (%d) less than specified dim-size (%d)", vda->length, vda->dsize[0]);
      *result = vda->data[index];
   }
   return TRUE;
}                               /* get_value */


/*------------------------------------------------------------------------------
 */
typedef struct Table_Entry {
   int p;                       /* previuous */
   int n;                       /* next */
   double a;                    /* prev weighting */
   double b;                    /* next weighting : a + b = 1.0 */
} sTable_Entry;


/*------------------------------------------------------------------------------
 * search points array to find se.p and se.n such that:
 *    points [se.p] < v <= points [se.n]
 *
 */
static bool find_table_entry (Error_Text error_text,
                              sTable_Entry * pte,
                              const sVariable_Dimension_Array * points,
                              const double val, int *out_of_range)
{
   bool status;
   int last_point;

   double val_first;
   double val_last;
   double val_prev;
   double val_next;

   int prev = 0;
   int next = 0;
   double frac = 0.0;

   ASSERT (points->ndims == 1, "ndims not 1");

   status = get_value (error_text, &val_first, points, 0);
   ASSERT (status == TRUE, "get first point");

   last_point = points->dsize[0] - 1;

   status = get_value (error_text, &val_last, points, last_point);
   ASSERT (status == TRUE, "get last point");

   if (val_first < val_last) {
      /* ascending  - test special cases first
       */
      if (val <= val_first) {
         prev = 0;
         next = 0;
         frac = 0.0;

         if (val < val_first) {
            *out_of_range = TRUE;
         }

      } else if (val >= val_last) {
         prev = last_point;
         next = last_point;
         frac = 0.0;

         if (val > val_last) {
            *out_of_range = TRUE;
         }

      } else {
         /* search x_points table to find p and next such that prev_x < x <= xnext
          */
         prev = 0;
         val_prev = val_first;

         for (next = 1; next <= last_point; next++) {

            status = get_value (error_text, &val_next, points, next);
            ASSERT (status == TRUE, "get ascending point");

            if (val <= val_next) {
               frac = (val - val_prev) / (val_next - val_prev);
               break;
            }

            prev = next;
            val_prev = val_next;
         }
      }

   } else {
      /* decending  - test special cases first
       */
      if (val >= val_first) {
         prev = 0;
         next = 0;
         frac = 0.0;

         if (val > val_first) {
            *out_of_range = TRUE;
         }

      } else if (val <= val_last) {
         prev = last_point;
         next = last_point;
         frac = 0.0;

         if (val < val_last) {
            *out_of_range = TRUE;
         }

      } else {
         /* search x_points table to find p and next such that prev_x < x <= xnext
          */
         prev = 0;
         val_prev = val_first;

         for (next = 1; next <= last_point; next++) {

            status = get_value (error_text, &val_next, points, next);
            ASSERT (status == TRUE, "get decending point");

            if (val >= val_next) {
               frac = (val - val_prev) / (val_next - val_prev);
               break;
            }

            prev = next;
            val_prev = val_next;
         }
      }
   }

   pte->p = prev;
   pte->n = next;
   pte->a = 1.0 - frac;
   pte->b = frac;

   return TRUE;
}                               /* find_table_entry */


/*------------------------------------------------------------------------------
 */
static bool sub_interpolate (Error_Text error_text,
                             double *result, int *out_of_range,
                             const sVariable_Dimension_Array * i_table,
                             const sTable_Entry te[])
{
   bool status;
   sVariable_Dimension_Array p_table;
   sVariable_Dimension_Array n_table;
   double pv, nv;

   if (i_table->ndims >= 1) {

      /* For 1D we could recurse down to 0D but just as east to do directly
       */
      if (i_table->ndims == 1) {

         status = get_value (error_text, &pv, i_table, te[0].p);
         ASSERT (status == TRUE, "get_value (prev)");

         status = get_value (error_text, &nv, i_table, te[0].n);
         ASSERT (status == TRUE, "get_value (next)");

      } else {

         status = slice (error_text, &p_table, i_table, te[0].p);
         ASSERT (status == TRUE, "slice (prev)");

         status =
             sub_interpolate (error_text, &pv, out_of_range, &p_table,
                              &te[1]);
         ASSERT (status == TRUE, "recursive interpolate (prev)");

         status = slice (error_text, &n_table, i_table, te[0].n);
         ASSERT (status == TRUE, "slice (next)");

         status =
             sub_interpolate (error_text, &nv, out_of_range, &n_table,
                              &te[1]);
         ASSERT (status == TRUE, "recursive interpolate (next)");
      }

      /* Interpolate.
       */
      *result = te[0].a * pv + te[0].b * nv;

   } else {
      /* Degenerate zero dimensions - scalar - easy.
       */
      *result = i_table->data[0];
      status = TRUE;
   }

   return status;
}                               /* sub_interpolate */

/*------------------------------------------------------------------------------
 */
bool interpolate (Error_Text error_text,
                  double *result, bool * out_of_range,
                  const sVariable_Dimension_Array * i_table,
                  const sVariable_Dimension_Array points[],
                  const double x[])
{
   bool status;
   sTable_Entry te[MAXIMUM_DIMENSIONS];
   int d;

   ASSERT (i_table != NULL, "null int table");
   ASSERT (i_table->data != NULL, "null int table data");
   ASSERT ((i_table->ndims >= 0) &&
           (i_table->ndims <= MAXIMUM_DIMENSIONS), "int table ndims");

   ASSERT (points != NULL, "null points");

   /* Find bounds around point of interest, together with
    * associated weighting factors.
    */
   for (d = 0; d < i_table->ndims; d++) {
      status =
          find_table_entry (error_text, &te[d], &points[d], x[d],
                            out_of_range);
      ASSERT (status == TRUE, "find table entry");
   }

   status =
       sub_interpolate (error_text, result, out_of_range, i_table, te);
   return status;
}                               /* interpolate */

/* end */
