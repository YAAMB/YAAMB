// set your clock speed here
#define XTAL 16000000

// you may have to experiment a little with this
// in case of heavy disturbance, try values around 60
// try to use odd values (for symmetrical level detection)
#define RC5RESAMPLE 61

# define PRESCALEDIV 64
# define TIMERCR (1<<CS01) | (1<<CS00)

#define RC5TIMERFIRST 0x100-((uint8_t) (XTAL/PRESCALEDIV*0.0008890))
#define RC5TIMERSECOND 0x100-((uint8_t) (XTAL/PRESCALEDIV*0.0004445))
#define RC5TIMERCANCEL 0x100-((uint8_t) (XTAL/PRESCALEDIV*0.0008890))

void rc5_init(void);
uint8_t new_rc5_data(void);
uint16_t get_rc5_code(void);
