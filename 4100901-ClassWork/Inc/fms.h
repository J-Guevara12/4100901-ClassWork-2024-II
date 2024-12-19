#define LONG_BUTTON_PERIOD_MAX_MS 300
typedef enum {
    LEFT = 'L',
    RIGHT = 'R',
    HAZARD = 'P'
} BUTTON;

void update_fms(BUTTON b);
