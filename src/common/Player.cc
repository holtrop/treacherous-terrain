#include "Player.h"
#include <math.h>

Player::Player()
{
    x = 0.0;
    y = 0.0;
    direction = M_PI_2;
    hover = 1.0;
    name =  "";
    w_pressed = KEY_NOT_PRESSED;
    a_pressed = KEY_NOT_PRESSED;
    s_pressed = KEY_NOT_PRESSED;
    d_pressed = KEY_NOT_PRESSED;
    rel_mouse_movement = 0;
    updated = false;
    m_client = NULL;
}
