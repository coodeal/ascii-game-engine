SET_FOOT_BRUSH("9")

SET_FRAME_RATE(40)

KC_UP = 0
KC_DOWN = 1
KC_LEFT = 2
KC_RIGHT = 3
KC_OK = 4
KC_ESC = 5
KC_P = 6
REG_KEY(0, KC_UP, 72)
REG_KEY(0, KC_DOWN, 80)
REG_KEY(0, KC_LEFT, 75)
REG_KEY(0, KC_RIGHT, 77)
REG_KEY(0, KC_OK, 13)
REG_KEY(0, KC_ESC, 27)
REG_KEY(0, KC_P, 112)

SET_MUTE(FALSE)
AHAT_LOGO = 0
AHAT_TICK = 1
AHAT_COLLIDE = 2
AHAT_GAME_OVER = 3
SET_AUDIO_STRING(AHAT_LOGO, "DEDEC>A<C>A<EDEDEC>A<C>A<EPPPP")
SET_AUDIO_STRING(AHAT_TICK, "C")
SET_AUDIO_STRING(AHAT_COLLIDE, "A")
SET_AUDIO_STRING(AHAT_GAME_OVER, "FAEPEEDDCC")
