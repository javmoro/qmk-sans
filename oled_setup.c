#include "brain.h"
#ifdef OLED_ENABLE
//#include QMK_KEYBOARD_H

#include "quantum.h"
#include <stdio.h>

#define RENDER_LAYER     // render_layer (+372)
uint8_t mod_state;       // get_mods()
uint8_t current_wpm = 0; // current_wpm parte como 0

#ifdef SANS_ENABLE

static uint16_t key_timer; // timer to track the last keyboard activity
static void refresh_rgb(void); // refreshes the activity timer and RGB, invoke whenever activity happens
static void check_rgb_timeout(void); // checks if enough time has passed for RGB to timeout
bool is_rgb_timeout = false; // store if RGB has timed out or not in a boolean

led_t led_usb_state;
#define ANIM_SIZE_SANS 256
// numero de pixeles en cada frame del sans
#define ANIM_FRAME_DURATION_SANS 200 // para bongocat y el sans cuanto tiempo mostrar cada frame
uint32_t anim_sans_timer;
uint32_t anim_sans_sleep;
uint8_t current_sans_frame = 0; // current sans frame
#endif

#ifdef BONGOCAT_KEYBOARD_ENABLE
#define WPM                                                                    \
		  15 // si current_wpm >= WPM entonces el gato empieza a tocar el keyboard
#define ANIM_FRAME_DURATION 200 // cuanto tiempo mostrar cada frame
#define ANIM_SIZE_CAT 320       // numero de pixeles en cada frame del gato
uint32_t anim_cat_timer;
uint32_t anim_cat_sleep;
uint8_t current_cat_frame = 0; // current cat frame
char wpm_str[4];
#endif
#    ifdef BONGOCAT_ENABLE
#        include "bongocat.h"
#    endif
#ifdef RENDER_LAYER
// esta funcion muestra las capas en el OLED y resalta la capa presente
static void render_layer(void) {
	if (IS_LAYER_ON(_DEFAULT)) {
		oled_write_P(PSTR("QWER\n"),
		             true);
	} else {
		if (IS_LAYER_ON(_FUNCTION)) {
			oled_write_P(PSTR("FUNC\n"), true);
		} else {
			if (IS_LAYER_ON(_GAMES)) {
				oled_write_P(PSTR("GAME\n"), true);
			} else {
				oled_write_P(PSTR("QWER\n"), false);
			}


		}
	}
	if (IS_LAYER_ON(_NUMBER)) {
		oled_write_P(PSTR("NUM\n"), true);
	} else {
		if (IS_LAYER_ON(_SYMBOL)) {
			oled_write_P(PSTR("SYMB\n"), true);
		} else {
			oled_write_P(PSTR("SYMB\n"), false);
		}
	}
	if (IS_LAYER_ON(_LAYERS)) {
		oled_write_P(PSTR("LAYER\n"), true);
	} else {
		oled_write_P(PSTR("LAYER\n"), false);
	}
}
#endif
void refresh_rgb() {
  key_timer = timer_read(); // store time of last refresh
  if (is_rgb_timeout) { // only do something if rgb has timed out
    print("Activity detected, removing timeout\n");
    is_rgb_timeout = false;
    rgblight_wakeup();
  }
}
void check_rgb_timeout() {
  if (!is_rgb_timeout && timer_elapsed(key_timer) > RGBLIGHT_TIMEOUT) {
    rgblight_suspend();
    is_rgb_timeout = true;
  }
}
void housekeeping_task_user(void) {
  #ifdef RGBLIGHT_TIMEOUT
  check_rgb_timeout();
  #endif
  
  /* rest of the function code here */
}
#ifdef SANS_ENABLE
// ### matrix sans {{{
static void render_sans(int SANS_X,
                        int SANS_Y) { // esta funcion contiene los frames y logica
	// de la animacion del sans

	static const char PROGMEM anNormal[3][ANIM_SIZE_SANS] = {
		{	// 'sans0', 32x32px
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0x38, 0x3c, 0x3c, 0xfc, 0xfc,
			0xfc, 0xfc, 0xfc, 0x3c, 0x3c, 0x38, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x19, 0x1f, 0x32, 0x26, 0x56, 0x47, 0x55,
			0x44, 0x55, 0x47, 0x56, 0x26, 0x32, 0x1f, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x3e, 0x3b, 0x6a, 0x15, 0x7b, 0x7a, 0x01, 0x2d, 0x3d,
			0x3a, 0x3d, 0x2d, 0x01, 0x7a, 0x6b, 0x15, 0x7a, 0x3b, 0x3e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x28, 0x30, 0x30, 0x30, 0x10, 0x08, 0x04,
			0x02, 0x04, 0x08, 0x10, 0x30, 0x30, 0x30, 0x28, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{
			// 'sans1' 32x22px
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0x70, 0x78, 0x78, 0xf8, 0xf8,
			0xf8, 0xf8, 0xf8, 0x78, 0x78, 0x70, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x3f, 0x64, 0x4c, 0xac, 0x8f, 0xab,
			0x89, 0xab, 0x8f, 0xac, 0x4c, 0x64, 0x3f, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7c, 0x77, 0xd4, 0x2a, 0xf6, 0xf4, 0x02, 0x5a, 0x7a,
			0x74, 0x7a, 0x5a, 0x02, 0xf4, 0xd6, 0x2a, 0xf4, 0x77, 0x7c, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x28, 0x30, 0x30, 0x30, 0x10, 0x08, 0x04,
			0x02, 0x04, 0x08, 0x10, 0x30, 0x30, 0x30, 0x28, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{
			// 'sans2', 32x32px
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0x38, 0x3c, 0x3c, 0xfc, 0xfc,
			0xfc, 0xfc, 0xfc, 0x3c, 0x3c, 0x38, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x9f, 0x32, 0x26, 0x56, 0x47, 0x55,
			0x44, 0x55, 0x47, 0x56, 0x26, 0x32, 0x9f, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7c, 0x77, 0xd4, 0x2b, 0xf7, 0xf4, 0x03, 0x5a, 0x7a,
			0x74, 0x7a, 0x5a, 0x03, 0xf4, 0xd7, 0x2b, 0xf4, 0x77, 0x7c, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x28, 0x30, 0x30, 0x30, 0x10, 0x08, 0x04,
			0x02, 0x04, 0x08, 0x10, 0x30, 0x30, 0x30, 0x28, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		}

	};
	static const char PROGMEM anMayus[3][ANIM_SIZE_SANS] = {
		{
			// 'ojo0', 32x32px
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc,
			0xfc, 0xfc, 0xfc, 0x3c, 0x3c, 0x38, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x19, 0x1f, 0x32, 0x26, 0x56, 0x47, 0x55,
			0x44, 0x55, 0x47, 0x56, 0x26, 0x32, 0x1f, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x3e, 0x3b, 0x6a, 0x15, 0x7b, 0x7a, 0x01, 0x2d, 0x3d,
			0x3a, 0x3d, 0x2d, 0x01, 0x7a, 0x6b, 0x15, 0x7a, 0x3b, 0x3e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x28, 0x30, 0x30, 0x30, 0x10, 0x08, 0x04,
			0x02, 0x04, 0x08, 0x10, 0x30, 0x30, 0x30, 0x28, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{
			// 'ojo1', 32x32px
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0xf0, 0xf8, 0xf8, 0xf8, 0xf8,
			0xf8, 0xf8, 0xf8, 0x78, 0x78, 0x70, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x3f, 0x65, 0x4d, 0xad, 0x8f, 0xab,
			0x89, 0xab, 0x8f, 0xac, 0x4c, 0x64, 0x3f, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7c, 0x77, 0xd4, 0x2a, 0xf6, 0xf4, 0x02, 0x5a, 0x7a,
			0x74, 0x7a, 0x5a, 0x02, 0xf4, 0xd6, 0x2a, 0xf4, 0x77, 0x7c, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x28, 0x30, 0x30, 0x30, 0x10, 0x08, 0x04,
			0x02, 0x04, 0x08, 0x10, 0x30, 0x30, 0x30, 0x28, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{
			// 'ojo2', 32x32px
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc,
			0xfc, 0xfc, 0xfc, 0x3c, 0x3c, 0x38, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x9f, 0x32, 0x26, 0x56, 0x47, 0x55,
			0x44, 0x55, 0x47, 0x56, 0x26, 0x32, 0x9f, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7c, 0x77, 0xd4, 0x2b, 0xf7, 0xf4, 0x03, 0x5a, 0x7a,
			0x74, 0x7a, 0x5a, 0x03, 0xf4, 0xd7, 0x2b, 0xf4, 0x77, 0x7c, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x28, 0x30, 0x30, 0x30, 0x10, 0x08, 0x04,
			0x02, 0x04, 0x08, 0x10, 0x30, 0x30, 0x30, 0x28, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		}

	};

	// matrix }}}
	// ### logic animation sans {{{
	void animate_sans(void) { // logica de la animacion del sans
		// cada accion del sans (anNormal, walk, run, bark y sneak) tiene 2 frames
		current_sans_frame = (current_sans_frame + 1) % 3;
		if (led_usb_state.caps_lock) { // si esta activado Caps Lock

			oled_write_raw_P(anMayus[abs(2 - current_sans_frame)], ANIM_SIZE_SANS); // sans ladra

		} else {
			oled_write_raw_P(anNormal[abs(2 - current_sans_frame)], ANIM_SIZE_SANS);
		}
	}

	if (timer_elapsed32(anim_sans_timer) > ANIM_FRAME_DURATION_SANS) {
		anim_sans_timer = timer_read32();
		animate_sans();
		if (current_wpm != 0) {
			anim_sans_sleep = timer_read32();
		}
	}
}
// logic animation }}}
#endif


#ifdef BONGOCAT_KEYBOARD_ENABLE
static void render_cat(void) { // esta funcion contiene los frames y logica de
	// la animacion del gato
	static const char PROGMEM idle[2][ANIM_SIZE_CAT] = {
		// frames de idle
		{
			// 'gato0', 32x80px
			0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x03, 0x3c, 0xc8, 0x08, 0x08, 0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80,
			0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x80, 0x60, 0x10, 0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x1e, 0x21, 0x40, 0x40, 0x20, 0x10, 0x08, 0x38, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x06, 0x38, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x18,
			0xc4, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0x08, 0x10, 0xa0, 0x40,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00,
			0x00, 0x01, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x18, 0x04, 0x02, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x20, 0x10, 0x10, 0x08, 0x07, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x80, 0x60, 0x18, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x18, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04,
			0x3c, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{
			0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x03, 0x3c, 0xc8, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,
			0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x80, 0x60, 0x10, 0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x1e, 0x21, 0x40, 0x40, 0x20, 0x10, 0x08, 0x38, 0xc0, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0e, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x18, 0xc4,
			0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0xa0, 0x40,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00,
			0x01, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x20, 0x10, 0x10, 0x08, 0x07, 0x00, 0x00,
			0x00, 0x00, 0x80, 0x60, 0x18, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x18, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04,
			0x3c, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		}
	};

	static const char PROGMEM tap[2][ANIM_SIZE_CAT] = {
		// frames de tap
		{
// 'gato1', 32x80px
			0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x03, 0x3c, 0xc8, 0x08, 0x08, 0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80,
			0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x80, 0x60, 0x10, 0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xc0, 0x9e, 0x21, 0x40, 0x40, 0x20, 0x10, 0x08, 0x38, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0e, 0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x0f, 0x00, 0x00, 0x38, 0x7f, 0xfc, 0xe0, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x18,
			0xc4, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x08, 0x10, 0xa0, 0x40, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00,
			0x00, 0x01, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x18, 0x06, 0x01, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0,
			0x80, 0x88, 0x08, 0x50, 0x10, 0x28, 0xc4, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
			0x3d, 0xc9, 0x0a, 0x08, 0x05, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{	// 'gato2', 32x80px
			0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x03, 0x3c, 0xc8, 0x08, 0x08, 0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80,
			0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0xc4, 0x04, 0x28, 0x88,
			0x10, 0xe0, 0x00, 0x00, 0x00, 0x78, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc4, 0x04, 0x04, 0x04, 0x06, 0x34, 0x32,
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0e, 0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x18,
			0xc4, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x08, 0x10, 0xa0, 0x40, 0x00,
			0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00,
			0x00, 0x01, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x18, 0x06, 0x01, 0x00, 0x00, 0x00,
			0x70, 0xf8, 0xf8, 0x70, 0x60, 0x41, 0x07, 0x0f, 0xc0, 0x20, 0x10, 0x10, 0x08, 0x07, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x80, 0x60, 0x18, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x60, 0xf0, 0xf8, 0xf8, 0x38, 0x08, 0x00, 0x07, 0x18, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04,
			0x3c, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x03, 0x3c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
	};
	// }}}
	// ### logic animation cat {{{
	void animate_cat(void) { // logica de la animacion del gato
		if (current_wpm < WPM) { // si current_wpm menor a WPM
			current_cat_frame = (current_cat_frame + 1) % 4;
			if (current_cat_frame < 2) {
				oled_write_raw_P(idle[0], ANIM_SIZE_CAT);
			} else {
				oled_write_raw_P(idle[1], ANIM_SIZE_CAT);
			}

		}

		if (current_wpm >= WPM) { // si current_wpm mayor o igual a WPM
			current_cat_frame = (current_cat_frame + 1) %
			                    2; // para alternar entre los 2 frames de tap
			oled_write_raw_P(tap[abs((2 - 1) - current_cat_frame)],
			                 ANIM_SIZE_CAT); // gato haciendo tap
		}
	}

	if (timer_elapsed32(anim_cat_timer) >
	        ANIM_FRAME_DURATION) { // timer_elapsed32(anim_ghost_timer) tiempo
		// transcurrido en ms desde la ultima vez que se
		// actualizo anim_ghost_timer

		anim_cat_timer = timer_read32(); // se actualiza anim_ghost_timer
		animate_cat();                   // se ejecuta animate_ghost()

		if (current_wpm != 0) {
			anim_cat_sleep = timer_read32(); // se actualiza anim_ghost_sleep
		}
	}
}
static void render_wpm(void) {
	oled_write(" WPM\n", false);
	wpm_str[3] = '\0';
	wpm_str[2] = '0' + current_wpm % 10;
	wpm_str[1] = '0' + (current_wpm /= 10) % 10;
	wpm_str[0] = '0' + current_wpm / 10;
	oled_write(" ", false);
	oled_write(wpm_str, false); // printear valor de WPM
}
#endif


char keylog_str[24] = {};
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
	if (is_keyboard_master()) {
		return OLED_ROTATION_270;
	} else {
#ifdef BONGOCAT_KEYBOARD_ENABLE
		return OLED_ROTATION_270;
#else
		return OLED_ROTATION_180; // flips the display 180 degrees if offhand
#endif
	}
	return rotation;
	// rotacion de 270 en ambas pantallas OLED (master y slave)
	// de manera que el punto inicial este en un cierto extremo
	// y se siga cierta orientacion
}

const char code_to_name[60] = {
	' ', ' ',  ' ', ' ', 'a',  'b', 'c', 'd', 'e', 'f', 'g', 'h',
	'i', 'j',  'k', 'l', 'm',  'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v',  'w', 'x', 'y',  'z', '1', '2', '3', '4', '5', '6',
	'7', '8',  '9', '0', 'R',  'E', 'B', 'T', '_', '-', '=', '[',
	']', '\\', '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '
};
void set_keylog(uint16_t keycode, keyrecord_t *record) {

	char name = ' ';
	if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) ||
	        (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
		keycode = keycode & 0xFF;
	}
	if (keycode < 60) {
		name = code_to_name[keycode];
	}

	// update keylog
	// snprintf(keylog_str, sizeof(keylog_str), "%dx%d\nk%2d\n  %c",
	// record->event.key.row, record->event.key.col, keycode, name);
	snprintf(keylog_str, sizeof(keylog_str), "\n KEY\n\n  %c", name);
	// snprintf(keylog_str, sizeof(keylog_str), "KEY %c", name);
}
void oled_render_keylog(void) { oled_write(keylog_str, false); }

bool oled_task_user(void) { // funcion en la cual se indica que poner en cada OLED
	current_wpm = get_current_wpm();
#ifdef SANS_ENABLE // {{{

#ifdef BONGOCAT_KEYBOARD_ENABLE // {{{
	// TODO: con las variables de export se podria
	// solucionar esto de que no se apagan bien las
	// pantallas
	if ((timer_elapsed32(anim_sans_sleep) > 60000) &&
	        (timer_elapsed32(anim_cat_sleep) > 60000) && (current_wpm == 0)) {
		if (is_oled_on()) {
			oled_off();
		}
		timer_init();
		return false;
	}
#endif                        // }}}
#endif                        // }}}
	if (is_keyboard_master()) { // OLED del master
#ifdef SANS_ENABLE // {{{
		// TODO: ver si esto soluciona el problema
		// de que se mantiene encendido el OLED
		if (current_wpm != 0 && !is_oled_on()) {
			oled_on();
		}
		led_usb_state = host_keyboard_led_state();
		render_sans(0, 1); // lo del sans
#else

#endif               // }}}
#ifdef RENDER_LAYER  // {{{
		oled_set_cursor(0, 5);
		render_layer(); // lo de las capas
#endif              // }}}
		// oled_render_keylog {{{
		oled_set_cursor(0, 10);
		oled_render_keylog();

	} else {                // OLED del slave
#ifdef BONGOCAT_KEYBOARD_ENABLE // TODO {{{
		render_cat();               // lo del gato
		oled_set_cursor(0, 11);
		render_wpm(); // lo del WPM
		oled_set_cursor(0, 14);
		oled_write("chow", false);
#endif // }}}
#ifdef BONGOCAT_ENABLE // {{{
		render_anim_bongocat();
		oled_set_cursor(0, 0);
		oled_write_P(PSTR("WPM"), false);
		oled_write(get_u8_str(get_current_wpm(), ' '), false);
#endif
	}
	return false;
}
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	if (!is_oled_on()) {
		anim_sans_sleep = timer_read32();
		anim_cat_sleep = timer_read32();
		oled_on();
	}
	if (record->event.pressed) {
		refresh_rgb();
		set_keylog(keycode, record);
	}
	mod_state = get_mods();
	return true;
}
#endif