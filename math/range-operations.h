#ifndef RANGE_OPERATIONS_H
#define RANGE_OPERATIONS_H

#include <math.h>
#include <stdbool.h>

//! @ingroup range operations
//! @brief Check if a value is within a lower and upper limit
static inline bool is_in_range(double val, double lowerlimit, double upperlimit)
{
        return (val >= lowerlimit) && (val <= upperlimit);
}

//! @ingroup range operations
//! @brief constrains a value with a lower and upper limit
static inline void clip_to_range(double *val, double lowerlimit,
                                 double upperlimit)
{
        *val = fmax(*val, lowerlimit);
        *val = fmin(*val, upperlimit);
}

//! @ingroup range operations
//! @brief constrains a value with a lower and upper limit
static inline double clip_val_to_range(double val, double lowerlimit,
                                       double upperlimit)
{
        clip_to_range(&val, lowerlimit, upperlimit);
        return val;
}

//! @ingroup range operations
//! @brief Applies an absolute maximum to input value
static inline double clip_abs_val(double val, double absmax)
{
        return (val < 0.0) ? fmax(val, -absmax) : fmin(val, absmax);
}

#define MAXVAL(x, y) (x) > (y) ? (x) : (y)
#define MINVAL(x, y) (x) < (y) ? (x) : (y)

#endif
