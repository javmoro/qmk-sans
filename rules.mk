MOUSEKEY_ENABLE = no    # Mouse keys
RGBLIGHT_ENABLE = yes    # Enable WS2812 RGB underlight.
OLED_ENABLE     = yes
OLED_DRIVER     = SSD1306
LTO_ENABLE      = yes
OLED_DRIVER_ENABLE = yes
WPM_ENABLE = yes

SANS_ENABLE = yes
BONGOCAT_KEYBOARD_ENABLE = yes

ifeq ($(strip $(OLED_ENABLE)), yes)
  SRC += oled_setup.c
  ifdef SANS_ENABLE
      ifeq ($(strip $(SANS_ENABLE)), yes)
        OPT_DEFS += -DSANS_ENABLE
      endif
  endif
  ifndef S_ENABLE
    OPT_DEFS += -DSANS_ENABLE
  endif

  ifdef BONGOCAT_KEYBOARD_ENABLE
      ifeq ($(strip $(BONGOCAT_KEYBOARD_ENABLE)), yes)
        OPT_DEFS += -DBONGOCAT_KEYBOARD_ENABLE
      endif
  endif
  ifndef BONGOCAT_KEYBOARD_ENABLE
    OPT_DEFS += -DBONGOCAT_KEYBOARD_ENABLE
  endif
endif