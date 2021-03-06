/* =====================================
=== FIMREG - Fast Image Registration ===
========================================

Written by Roelof Berg, Berg Solutions (rberg@berg-solutions.de) with support from
Lars Koenig, Fraunhofer MEVIS (lars.koenig@mevis.fraunhofer.de) Jan Ruehaak, Fraunhofer
MEVIS (jan.ruehaak@mevis.fraunhofer.de).

THIS IS A LIMITED RESEARCH PROTOTYPE. Documentation: www.berg-solutions.de/fimreg.html

------------------------------------------------------------------------------

Copyright (c) 2014, Roelof Berg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of the owner nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------------*/

/*
 * calcMarginAddition.cpp
 *
 * CODE GENERATED BY MATLAB CODER (THE HUMAN READABILITY IS THEREFORE LIMITED)
 *
 */

#include "stdafx.h"

/* Include files */
#include "rt_nonfinite.h"
#include "calcDSPLayout.h"
#include "diffimg.h"
#include "dstr_ssd.h"
#include "gaussnewton.h"
#include "gen_example_data.h"
#include "generatePyramidPC.h"
#include "get_current_time_in_sec.h"
#include "jacobian.h"
#include "jacobianOnTarget.h"
#include "myprintf.h"
#include "notiifyFinishedOnTarget.h"
#include "sendToTarget.h"
#include "ssd.h"
#include "ssdOnTarget.h"
#include "start_jacobianOnTarget.h"
#include "start_ssdOnTarget.h"
#include "transform.h"
#include "transmitImageData.h"
#include "waitUntilTargetReady.h"
#include "calcMarginAddition.h"
#include "fimreg_rtwutil.h"

/* Custom Source Code */
#include "pseudo_stdafx.h"             //precompiled header not possible because of include position of matlab

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */

/* Function Definitions */
void calcMarginAddition(real32_T da, real32_T dtr, uint32_T d, uint32_T
  MarginAddition[3])
{
  real32_T y;
  real32_T dmax;
  real32_T angles[6];
  int32_T i;
  int32_T ixstart;
  real32_T l;
  int32_T angle_size;
  int32_T Box_size_idx_0;
  real32_T Box_data[48];
  real32_T uBox[8];
  int32_T ix;
  boolean_T exitg4;
  uint32_T marginL;
  boolean_T exitg3;
  uint32_T marginR;
  boolean_T exitg2;
  uint32_T marginO;
  boolean_T exitg1;
  uint32_T marginU;

  y = (real32_T)d / 2.0F;
  dmax = (real32_T)d / 2.0F - 0.5F;

  /* Shifting, um negative Koordinaten in Matrixbereich zu bringen */
  /* ======================================================================== */
  /* = Relevante Bildteile berechnen, Bilder übertragen */
  /* ======================================================================== */
  /* Winkelstützstellen berechnen. */
  /* Einerseits die erlaubten Maximalwinkel w-da, w+da */
  /* Ferner alle Extrema zwischen den Stützstelen ((x-45) mod 90 = 0) */
  /* ensure angle to be in range 0 ... 360 deg */
  for (i = 0; i < 6; i++) {
    angles[i] = 0.0F;
  }

  angles[0] = 0.0F - da;
  angles[1] = da;
  i = 3;

  /* Maximum diagonal extend is at 45deg, (90+45)deg, (190+45)deg etc. */
  /* wMax=single(0); */
  for (ixstart = 0; ixstart < 12; ixstart++) {
    l = -5.497787F + (real32_T)ixstart * 1.57079637F;

    /* because dt is allowed to be 0 ... 360 deg */
    if ((angles[0] < l) && (angles[1] > l)) {
      angles[(int32_T)(uint32_T)i - 1] = l;
      i = (int32_T)((uint32_T)i + 1U);
    }
  }

  angle_size = (int32_T)((uint32_T)i - 1U);

  /* DSPResponsibilityBox=zeros(4,'single'); */
  /* BoundBox=zeros(4,'uint32'); */
  i = (int32_T)(((uint32_T)angle_size - 1U) << 2);
  Box_size_idx_0 = (int32_T)((uint32_T)i + 4U);
  i = ((int32_T)((uint32_T)i + 4U) << 1) - 1;
  for (ixstart = 0; ixstart <= i; ixstart++) {
    Box_data[ixstart] = 0.0F;
  }

  for (ixstart = 0; ixstart < 8; ixstart++) {
    uBox[ixstart] = 0.0F;
  }

  /*  Untransformed x,y edges per DSP ------------------- */
  uBox[0] = -dmax;
  uBox[4] = -dmax;
  uBox[1] = y - 0.5F;
  uBox[5] = -dmax;
  uBox[2] = -dmax;
  uBox[6] = y - 0.5F;
  uBox[3] = y - 0.5F;
  uBox[7] = y - 0.5F;

  /*  Transform edge coordinates -------------------------- */
  for (i = 1; (uint32_T)i <= (uint32_T)angle_size; i = (int32_T)((uint32_T)i +
        1U)) {
    for (ixstart = 0; ixstart < 4; ixstart++) {
      Box_data[(int32_T)(((uint32_T)i - 1U) << 2) + ixstart] = (real32_T)cos
        (angles[i - 1]) * uBox[ixstart] - (real32_T)sin(angles[i - 1]) * uBox[4
        + ixstart];
      Box_data[((int32_T)(((uint32_T)i - 1U) << 2) + ixstart) + Box_size_idx_0] =
        (real32_T)sin(angles[i - 1]) * uBox[ixstart] + (real32_T)cos(angles[i -
        1]) * uBox[4 + ixstart];
    }
  }

  /*  X Bounding Box ------------------------------- */
  i = (int32_T)((uint32_T)angle_size << 2);
  ixstart = 1;
  l = Box_data[0];
  if (rtIsNaNF(Box_data[0])) {
    ix = 2;
    exitg4 = FALSE;
    while ((exitg4 == 0U) && (ix <= i)) {
      ixstart = ix;
      if (!rtIsNaNF(Box_data[ix - 1])) {
        l = Box_data[ix - 1];
        exitg4 = TRUE;
      } else {
        ix++;
      }
    }
  }

  if (ixstart < i) {
    while (ixstart + 1 <= i) {
      if (Box_data[ixstart] < l) {
        l = Box_data[ixstart];
      }

      ixstart++;
    }
  }

  l = (l - 1.0F) - dtr;

  /* 1 more pixel for bilinear filtering             */
  marginL = 0U;
  if (l < -dmax) {
    /* clipping */
    marginL = (uint32_T)rt_roundf_snf((-dmax - l) + 1.0F);

    /* +1 = ceil */
  }

  ixstart = 1;
  l = Box_data[0];
  if (rtIsNaNF(Box_data[0])) {
    ix = 2;
    exitg3 = FALSE;
    while ((exitg3 == 0U) && (ix <= i)) {
      ixstart = ix;
      if (!rtIsNaNF(Box_data[ix - 1])) {
        l = Box_data[ix - 1];
        exitg3 = TRUE;
      } else {
        ix++;
      }
    }
  }

  if (ixstart < i) {
    while (ixstart + 1 <= i) {
      if (Box_data[ixstart] > l) {
        l = Box_data[ixstart];
      }

      ixstart++;
    }
  }

  l = (l + 1.0F) + dtr;

  /* 1 more pixel for bilinear filtering */
  marginR = 0U;
  if (l > y - 0.5F) {
    /* clipping */
    marginR = (uint32_T)rt_roundf_snf((l - (y - 0.5F)) + 1.0F);

    /* +1 = ceil */
  }

  /*  Y Bounding Box ------------------------------- */
  i = (int32_T)((uint32_T)angle_size << 2);
  ixstart = 1;
  l = Box_data[Box_size_idx_0];
  if (rtIsNaNF(Box_data[Box_size_idx_0])) {
    ix = 2;
    exitg2 = FALSE;
    while ((exitg2 == 0U) && (ix <= i)) {
      ixstart = ix;
      if (!rtIsNaNF(Box_data[(ix + Box_size_idx_0) - 1])) {
        l = Box_data[(ix + Box_size_idx_0) - 1];
        exitg2 = TRUE;
      } else {
        ix++;
      }
    }
  }

  if (ixstart < i) {
    while (ixstart + 1 <= i) {
      if (Box_data[ixstart + Box_size_idx_0] < l) {
        l = Box_data[ixstart + Box_size_idx_0];
      }

      ixstart++;
    }
  }

  l = (l - 1.0F) - dtr;

  /* 1 more pixel for bilinear filtering; */
  marginO = 0U;
  if (l < -dmax) {
    /* clipping */
    marginO = (uint32_T)rt_roundf_snf((-dmax - l) + 1.0F);

    /* +1 = ceil */
  }

  ixstart = 1;
  l = Box_data[Box_size_idx_0];
  if (rtIsNaNF(Box_data[Box_size_idx_0])) {
    ix = 2;
    exitg1 = FALSE;
    while ((exitg1 == 0U) && (ix <= i)) {
      ixstart = ix;
      if (!rtIsNaNF(Box_data[(ix + Box_size_idx_0) - 1])) {
        l = Box_data[(ix + Box_size_idx_0) - 1];
        exitg1 = TRUE;
      } else {
        ix++;
      }
    }
  }

  if (ixstart < i) {
    while (ixstart + 1 <= i) {
      if (Box_data[ixstart + Box_size_idx_0] > l) {
        l = Box_data[ixstart + Box_size_idx_0];
      }

      ixstart++;
    }
  }

  l = (l + 1.0F) + dtr;

  /* 1 more pixel for bilinear interpolation; */
  marginU = 0U;
  if (l > y - 0.5F) {
    /* clipping */
    marginU = (uint32_T)rt_roundf_snf((l - (y - 0.5F)) + 1.0F);

    /* +1 = ceil */
  }

  /*  Extract img ------------------------------- */
  /* Keep only the biggest margin of left and right. (We share both */
  /* margins on the right with one exception, we add one times the left */
  /* margin to the space between the top margin and the data start.) */
  if (marginL > marginR) {
    marginR = marginL;
  }

  MarginAddition[0] = marginR;
  MarginAddition[1] = marginO + 1U;
  MarginAddition[2] = marginU;

  /* We add 1 to the upper margin because of the reusal of the right and */
  /* left margin. It would be sufficient to add marginL padding bytes to */
  /* the top, however adding a whole row is more convenient as the */
  /* resulting data buffer remains a square picture and the loss of a few */
  /* KB ram (in a cache-uninteresting area) is neglectible. */
}

/* End of code generation (calcMarginAddition.cpp) */
