#ifndef MATH_DEF_H
#define MATH_DEF_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#define EPS_1E_12 1e-12
#define EPS_1E_8  1e-8
#define EPS_1E_5  1e-5


#define DYNAMIC_TOLERANCE \
        1.05; /*!< Tolerable dynamic difference for segment transitions */
#define DYNAMIC_TOLERANCE_VEL  DYNAMIC_TOLERANCE
#define DYNAMIC_TOLERANCE_ACC  DYNAMIC_TOLERANCE
#define DYNAMIC_TOLERANCE_JERK DYNAMIC_TOLERANCE

#define M_PI2     2.0 * M_PI  //!< 2*PI
#define M_PI2_INV 1.0 / M_PI2 //!< 1/(2.0*PI)

#define SQRT_2    M_SQRT2   //!< sqrt(2.0)
#define SQRT2_INV M_SQRT1_2 //!< sqrt(0.5)

#endif
