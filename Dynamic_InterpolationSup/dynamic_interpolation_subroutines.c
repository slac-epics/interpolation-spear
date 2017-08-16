/* $File: //ASP/Dev/SBS/4_Controls/4_4_Equipment_Control/Common/sw/Interpolation/Dynamic_InterpolationSup/dynamic_interpolation_subroutines.c $
 * $Revision: 1.1.1.1 $
 * $DateTime: 2012/06/07 10:13:13 $
 * Last checked in by: $Author: saa $
 *
 * Description
 * Interpolation calculation sub-routine, for use with the aSubRecord.
 * Enables the record to perform 1-D, 2-D .. 8-D table interpolation.
 *
 * INPA: long - number of dimensions (0 .. 8). 0 works but is a degenerate case.
 * INPB: long - first/last interpolation item (D = 4, E = 5, ... J = 10,  ... U = 21)
 *              Encoded as 100*First + Last
 * INPC: long - spare.
 *
 * OUTA: long - indicates if any coordinate out side of region of interest.
 *
 * The interpretation of other inputs depends on the number of dimensions.
 * Less dimensions means that more interpolation tables inputs and interpolated
 * value outputs can be processed by a record instance.
 *
 * INPx/INPy: dimension co-ordinate set (array) and associated co-ordinate.
 * The number of co-ordinate sets and coordinate pairs is defined by the
 * number of dimensions (n). Both field types must be DOUBLE.
 *
 *    INPD/INPE:  n >= 1
 *    INPF/INPG:  n >= 2
 *    INPH/INPI:  n >= 3
 *    INPJ/INPK:  n >= 4
 *    INPL/INPM:  n >= 5
 *    INPN/INPO:  n >= 6
 *    INPP/INPQ:  n >= 7
 *    INPR/INPS:  n >= 8
 *
 * INPz/OUTz: interpolation table, interpolated output value.
 *            Only calculated if both INPz/OUTz are DOUBLE.
 * The number of available interpolation table/interpolated output value
 * pairs also depends on the number of dimensions (n):
 *
 *    INPD/OUTD:  n <= 0
 *    INPE/OUTE:  n <= 0
 *    INPF/OUTF:  n <= 1
 *    INPG/OUTG:  n <= 1
 *    INPH/OUTH:  n <= 2
 *    INPI/OUTI:  n <= 2
 *    INPJ/OUTJ:  n <= 3
 *    INPK/OUTK:  n <= 3
 *    INPL/OUTL:  n <= 4
 *    INPM/OUTM:  n <= 4
 *    INPN/OUTN:  n <= 5
 *    INPO/OUTO:  n <= 5
 *    INPP/OUTP:  n <= 6
 *    INPQ/OUTQ:  n <= 6
 *    INPR/OUTR:  n <= 7
 *    INPS/OUTS:  n <= 7
 *    INPT/OUTT:  n <= 8
 *    INPU/OUTU:  n <= 8
 *
 *
 * Multi dimensional array format within a waveform record.
 * --------------------------------------------------------
 *
 * The expected waveform format is  { n, size [n], data [m] }
 * where
 *    n is the number of dimensions;
 *    size [n] is a list of the sizes of each of the n dimensions;
 *    data [m] is array content, row major; and
 *    m = size [1] * size [2] * ... size [n]
 *
 * Examples:
 * --------
 *
 *  1-D 5-tuple array requires 7 waveform elements.
 *
 *  +--------+--------+--------+--------+--------+--------+--------+
 *  |  1.0   |  5.0   |  a[1]  |  a[2]  |  a[3]  |  a[4]  |  a[5]  |
 *  +--------+--------+--------+--------+--------+--------+--------+
 *
 *
 *  2-D [3x2] array requires 9 waveform elements.
 *
 *  +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 *  |  2.0   |  3.0   |  2.0   | a[1,1] | a[1,2] | a[2,1] | a[2,2] | a[3,1] | a[3,2] |
 *  +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 *
 *
 *  Degenerate (0-D) scaler.
 *
 *  +--------+--------+
 *  |  0.0   |    a   |
 *  +--------+--------+
 *
 * The number of dimensions and dimension size meta data in the first 1 + n
 * waveform elements are still held double variables, but must contain whole
 * number Integer) values.
 *
 *
 * Source code formatting:
 * indent -kr -pcs -i3 -cli3 -nbbo -nut  dynamic_interpolation_subroutines.c
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
 * andrew.starritt@synchrotron.org.au
 * 800 Blackburn Road, Clayton, Victoria 3168, Australia.
 *
 */

#include <stdio.h>

#include <aSubRecord.h>
#include <cantProceed.h>
#include <menuFtype.h>
#include "epicsTypes.h"
#include <errlog.h>
#include <epicsExport.h>
#include <registryFunction.h>

#include "dynamic_interpolation.h"

/* Relates to I/O field names: A=1, B=2, ... U=21
 * Defines first/last allowd interpolation as opposed to those actually
 * in use actually in use
 */
#define FIRST_INTERPOLATION    4
#define LAST_INTERPOLATION     21

typedef struct subPvt {
   int number_dimensions;
   int first_interpolation;
   int last_interpolation;
   bool already_error_reported; /* inhibits repeated error reports when set */
   bool interpolation_item_reported[LAST_INTERPOLATION];
} sSubPvt;



/* -----------------------------------------------------------------------------
 * macro assert function - allows inline return, and access to prec/pSubPvt.
 */
#define ASSERT(condition, ...)  {                                              \
   if (!(condition)) {                                                         \
      char msg [81];                                                           \
      snprintf (msg, 80, __VA_ARGS__);                                         \
      if (pSubPvt->already_error_reported == FALSE) {                          \
         errlogPrintf ("%s:%d %s %s\n", __FUNCTION__, __LINE__,                \
                       prec->name, msg);                                       \
         pSubPvt->already_error_reported = TRUE;                               \
      }                                                                        \
      return (-1);                                                             \
   }                                                                           \
}

/* -----------------------------------------------------------------------------
 */
static long Dynamic_Interpolation_Init (aSubRecord * prec)
{
   sSubPvt *pSubPvt;
   int ndims;
   int temp;
   int first;
   int last;
   epicsEnum16 *input_type;
   epicsEnum16 *output_type;
   epicsUInt32 *maximum_input_elements;
   int d;
   int f;
   char field;
   int min_size;

   printf ("+++ %s %s\n", __FUNCTION__, prec->name);

   /* The pact field is only cleared if we successfully reach end of
    * the init function.
    */
   prec->pact = 1;

   /* Allocate and initialise record private data
    */
   pSubPvt = (sSubPvt *) callocMustSucceed
       (1, sizeof (sSubPvt), __FUNCTION__);

   /* Clear flags that inhibit error reports
    */
   pSubPvt->already_error_reported = FALSE;
   for (f = 0; f < LAST_INTERPOLATION; f++) {
      pSubPvt->interpolation_item_reported[f] = FALSE;
   }

   /* Store away the private data for this record into the EPICS record.
    */
   prec->dpvt = pSubPvt;

   /* Verify inputs/output types.
    */
   ASSERT (prec->fta == menuFtypeLONG, "FTA type not LONG");
   ASSERT (prec->ftb == menuFtypeLONG, "FTB type not LONG");
   ASSERT (prec->ftva == menuFtypeLONG, "FVTA type not LONG");

   /* Read/verify constant record parameters.
    */
   pSubPvt->number_dimensions = ndims = *((epicsInt32 *) prec->a);
   ASSERT (ndims >= 1, "INPA number of dimensions (%d) not 1 .. 8", ndims);
   ASSERT (ndims <= 8, "INPA number of dimensions (%d) not 1 .. 8", ndims);
   ASSERT (ndims <= MAXIMUM_DIMENSIONS,
           "INPA number of dimensions (%d) too big", ndims);

   temp = *((epicsInt32 *) prec->b);
   /* Decode 
    */
   first = temp / 100;
   last = temp % 100;

   ASSERT ((first >= FIRST_INTERPOLATION) && (first <= LAST_INTERPOLATION),
           "INPB first interpolation (%d) not in range %d .. %d", first,
           FIRST_INTERPOLATION, LAST_INTERPOLATION);

   ASSERT ((last >= FIRST_INTERPOLATION) && (last <= LAST_INTERPOLATION),
           "INPB last interpolation (%d) not in range %d .. %d", last,
           FIRST_INTERPOLATION, LAST_INTERPOLATION);

   ASSERT (first >= FIRST_INTERPOLATION + (2 * ndims),
           "INPB first interpolation (%d) inconsistent with number of dimensions (%d)",
           first, ndims);

   ASSERT (first <= last, "INPB first (%d) > last (%d)", first, last);

   /* Save - re-origin relative to D.
    */
   pSubPvt->first_interpolation = first - FIRST_INTERPOLATION;
   pSubPvt->last_interpolation = last - FIRST_INTERPOLATION;

   /* Allow array access to set of D fields through to set of U fields.
    */
   input_type = &prec->ftd;
   maximum_input_elements = &prec->nod;
   output_type = &prec->ftvd;

   /* Verify types/sizes.
    */
   min_size = 3;
   for (d = 0; d < ndims; d++) {
      f = 2 * d;
      field = (char) ((int) 'D' + f);
      ASSERT (input_type[f] == menuFtypeDOUBLE, "FT%c type not DOUBLE",
              field);
      ASSERT (maximum_input_elements[f] >= min_size,
              "NO%c too small (< %d)", field, min_size);

      f = 2 * d + 1;
      field = (char) ((int) 'D' + f);
      ASSERT (input_type[f] == menuFtypeDOUBLE, "FT%c type not DOUBLE",
              field);
   }

   /* Verify table inputs and output types/sizes.
    */
   min_size = 1 + ndims + 1;
   for (f = pSubPvt->first_interpolation; f <= pSubPvt->last_interpolation;
        f++) {
      field = (char) ((int) 'D' + f);

      ASSERT (input_type[f] == menuFtypeDOUBLE, "FT%c type not DOUBLE",
              field);
      ASSERT (maximum_input_elements[f] >= min_size,
              "NO%c too small (< %d) for specified number of dimensions (%d)",
              field, min_size, ndims);

      ASSERT (output_type[f] == menuFtypeDOUBLE, "FTV%c type not DOUBLE",
              field);
   }

   /* All okay - no assert failures - allow record to process.
    */
   prec->pact = 0;
   printf ("--- %s %s\n", __FUNCTION__, prec->name);
   return 0;
}                               /* Dynamic_Interpolation_Init */


/* -----------------------------------------------------------------------------
 */
static long Dynamic_Interpolation_Process (aSubRecord * prec)
{
   sSubPvt *pSubPvt;
   int ndims;
   sVariable_Dimension_Array points[MAXIMUM_DIMENSIONS];
   double x[MAXIMUM_DIMENSIONS];
   sVariable_Dimension_Array i_table;

   void **input_value;
   epicsUInt32 *number_of_input_elements;
   void **output_value;
   epicsUInt32 *number_of_output_elements;

   int d;
   int f;
   char field;
   double *double_ref;
   double value;
   bool status;
   bool out_of_range;
   Error_Text error_text;

   /* Fetch the private data for this record.
    */
   pSubPvt = (sSubPvt *) prec->dpvt;
   ASSERT (pSubPvt != NULL, "record private data does not exist");

   ndims = pSubPvt->number_dimensions;  /* alias */

   /* Allow array access to set of fields from D through to U.
    */
   input_value = &prec->d;
   number_of_input_elements = &prec->ned;
   output_value = &prec->vald;
   number_of_output_elements = &prec->nevd;

   for (d = 0; d < ndims; d++) {
      field = (char) ((int) 'D' + 2 * d);

      double_ref = (double *) input_value[2 * d];

      ASSERT ((int) double_ref[0] == 1, "INP%c not 1D array", field);
      ASSERT (number_of_input_elements[2 * d] >= 3,
              "INP%c too small", field);

      points[d].ndims = 1;
      points[d].dsize[0] = (int) double_ref[1];
      points[d].length = number_of_input_elements [2 * d] - 2;
      points[d].data = &double_ref[2];

      double_ref = (double *) input_value[2 * d + 1];
      x[d] = double_ref[0];
   }

   out_of_range = FALSE;
   for (f = pSubPvt->first_interpolation;
        f <= pSubPvt->last_interpolation; f++) {
      field = (char) ((int) 'D' + f);

      double_ref = (double *) input_value[f];

      d = (int) double_ref[0];
      ASSERT (d == ndims, "INP%c incorrect dimensionality (%d)", field, d);

      ASSERT (number_of_input_elements[f] >= 2 + ndims,
              "INP%c interpolation table array too small", field);

      i_table.ndims = ndims;
      for (d = 0; d < ndims; d++) {
         i_table.dsize[d] = (int) double_ref[1 + d];
      }
      i_table.length = number_of_input_elements[f] - (1 + ndims);
      i_table.data = &double_ref[1 + ndims];

      error_text[0] = '\0';
      status = interpolate
          (error_text, &value, &out_of_range, &i_table, points, x);

      if (status == FALSE) {
         if (pSubPvt->interpolation_item_reported[f] == FALSE) {
            printf  ("Dynamic interpolation %s [%c] failed:\n   %s\n\n",
                     prec->name, field, error_text);
            pSubPvt->interpolation_item_reported[f] = TRUE;
         }
      }

      double_ref = (double *) output_value[f];
      double_ref[0] = value;
      number_of_output_elements[f] = 1;
   }

   /* Set out of range indicator.
    */
   *((epicsInt32 *) prec->vala) = out_of_range;

   /* All okay - clear already reported flag so that next fault is reported.
    */
   if (pSubPvt->already_error_reported == TRUE) {
      errlogPrintf ("%s:%d %s %s\n", __FUNCTION__, __LINE__,
                    prec->name, "Information - error cleared.");
      pSubPvt->already_error_reported = FALSE;
   }
   return 0;
}                               /* Dynamic_Interpolation_Process */


/* -----------------------------------------------------------------------------
 */
epicsRegisterFunction (Dynamic_Interpolation_Init);
epicsRegisterFunction (Dynamic_Interpolation_Process);

/* end */
