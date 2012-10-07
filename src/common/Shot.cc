
#include <math.h>
#include "Shot.h"
#include "Types.h"

using namespace sf;

/* INITIAL_SHOT_HEIGHT needs to be set to the height that the shot
 * starts at, which will depend on the tank model in use */
#define INITIAL_SHOT_HEIGHT 30

#define GRAVITY 9.8

/* We model the shot's position using a parametric equation based on time.
 * Assuming a constant 45° shot angle simplifies the equations.
 * x = Vt
 * y = H + Vt - gt²/2
 * where
 * V = shot speed
 * t = time
 * g = gravity
 * H = INITIAL_SHOT_HEIGHT
 *
 * According to the quadratic formula (x = (-b ± sqrt(b²-4ac))/2a), y = 0 when
 * t = (-V + sqrt(V² - 4*g*H/2)) / (2*g/2)
 * We need to solve for V.
 * tg = sqrt(V² - 2gH) - V
 * tg + V = sqrt(V² - 2gH)
 * (tg + V)² = V² - 2gH
 * t²g² + 2tgV + V² = V² - 2gH
 * t²g² + 2tgV + 2gH = 0
 * (t²g² + 2gH) = -2tgV
 * -(t²g² + 2gH) / (2tg) = V
 *
 * So given the time to target (target_dist / PROJECTILE_VELOCITY) we can
 * solve for what the shot's speed should be.
 */
Shot::Shot(const Vector2f & origin, double direction, double target_dist)
{
    m_direction = Vector2f(cos(direction), sin(direction));
    m_origin = origin;
    double t = target_dist / PROJECTILE_VELOCITY;
    m_speed = -(t * t * GRAVITY * GRAVITY + 2 * GRAVITY * INITIAL_SHOT_HEIGHT)
        / (2 * t * GRAVITY);
}

Vector3f Shot::get_position()
{
    float time = m_clock.getElapsedTime().asSeconds();
    float horiz_dist = m_speed * time;
    float z = INITIAL_SHOT_HEIGHT + m_speed * time - GRAVITY * time * time / 2.0;
    Vector2f xy = m_direction * horiz_dist;
    return Vector3f(xy.x, xy.y, z);
}
