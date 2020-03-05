/* 
 * Original Author: Julie Zelenski, NeXT Developer Support
 * You may freely copy, distribute and reuse the code in this example.
 * NeXT disclaims any warranty of any kind, expressed or implied, as to
 * its fitness for any particular use.   
 *
 * Adapted by Mark Riordan  14 Sep 1997
 */
#include "stdafx.h"

void ShellSortDwordArray(CDWordArray & ary)
{

#define STRIDE_FACTOR 3
   int c, stride;
   int d;
   int found;
   int nElements = ary.GetSize();
   DWORD dwtemp;

   stride = 1;
   while (stride <= nElements)
      stride = stride * STRIDE_FACTOR + 1;

   while (stride > (STRIDE_FACTOR - 1)) {
      stride = stride / STRIDE_FACTOR;
      for (c = stride; c < nElements; c++) {
         found = 0;
         d = c - stride;
         while ((d >= 0) && !found) {
            if (ary[d + stride] < ary[d]) {
               dwtemp = ary[d];
               ary[d] = ary[d + stride];
               ary[d + stride] = dwtemp;
               d -= stride;
            } else {
               found = 1;
            }
         }
      }
   }
}
