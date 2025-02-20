/* $File: //ASP/Dev/SBS/4_Controls/4_4_Equipment_Control/Common/sw/Interpolation/Dynamic_InterpolationSup/dynamic_interpolation.h $
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

#ifndef _DYNAMIC_INTERPOLATION_H
#define _DYNAMIC_INTERPOLATION_H  1

typedef int bool;

#define FALSE    0
#define TRUE     1

#define ERROR_TEXT_SIZE   500

typedef char Error_Text [ERROR_TEXT_SIZE];

#define MAXIMUM_DIMENSIONS   8

typedef struct Variable_Dimension_Array {
   int ndims;
   int dsize[MAXIMUM_DIMENSIONS];
   int length;
   double *data;
} sVariable_Dimension_Array;


/* returns TRUE if successfull otherwise FALSE
 */
bool interpolate (Error_Text error_text,
                  double *result, bool * out_of_range,
                  const sVariable_Dimension_Array * i_table,
                  const sVariable_Dimension_Array points[],
                  const double x[]);

#endif                          /* dynamic_interpolation.h */
