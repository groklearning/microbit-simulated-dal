#include "../microbit-dal/inc/MicroBitButton.h"

// Replace button thresholds. We don't need debouncing.
#undef MICROBIT_BUTTON_SIGMA_MAX
#undef MICROBIT_BUTTON_SIGMA_THRESH_HI
#undef MICROBIT_BUTTON_SIGMA_THRESH_LO

#define MICROBIT_BUTTON_SIGMA_MAX               2
#define MICROBIT_BUTTON_SIGMA_THRESH_HI         1
#define MICROBIT_BUTTON_SIGMA_THRESH_LO         1
