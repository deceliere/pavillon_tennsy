

/* PINS */
# define FET 23 // PWM pin > FET

/* OLED */
# define FONT_NORMAL u8g2_font_helvR08_tr

void setup_oled(void);
void loop_oled(const char *soundfile);
void message_oled(const char *message);
void loop_oled_scroll(const char *soundfile);


// void draw(int is_blank);
// void draw_m0_h_with_extra_blank();