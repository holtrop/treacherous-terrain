
#include <math.h>
#include "Shot.h"
#include "Types.h"

using namespace sf;

/* INITIAL_SHOT_HEIGHT needs to be set to the height that the shot
 * starts at, which will depend on the tank model in use */
#define INITIAL_SHOT_HEIGHT 10

/* GRAVITY can really be any arbitrary value that makes the shot's speed
 * feel right. Increasing the gravity will decrease the amount of time
 * it takes the shot to hit its target. */
#define GRAVITY 30

/* We model the shot's position using a parametric equation based on time.
 * Assuming a constant 45° shot angle simplifies the equations.
 * x = vt
 * y = h + vt - gt²/2
 *   = (-g/2)t² + vt + h
 * where
 * v = shot speed
 * t = time
 * g = gravity
 * h = INITIAL_SHOT_HEIGHT
 *
 * Given a target distance of d, we want to figure out a speed that makes
 * (x, y) = (d, 0) a valid point on the trajectory.
 * Then:
 * d = vt
 * 0 = (-g/2)t² + vt + h
 * So:
 * v = d/t
 * 0 = (-g/2)t² + (d/t)t + h
 * 0 = (-g/2)t² + d + h
 * According to the quadratic formula (x = (-b ± sqrt(b² - 4ac))/2a),
 * t = ±sqrt(-4(-g/2)(d+h)) / 2(-g/2)
 * -tg = ±sqrt(2g(d+h))
 * t²g² = 2g(d+h)
 * t² = 2(d+h)/g
 * t = sqrt(2(d+h)/g)
 * Now that we know the time at which this point occurs, we can solve for
 * the shot speed (v)
 * v = d/t
 * v = d / sqrt(2(d+h)/g)
 */
Shot::Shot(const Vector2f & origin, double direction, double target_dist)
{
    m_direction = Vector2f(cos(direction), sin(direction));
    m_origin = origin;
    m_speed = target_dist /
        sqrt(2 * (target_dist + INITIAL_SHOT_HEIGHT) / GRAVITY);
}

Vector3f Shot::get_position()
{
    float time = m_clock.getElapsedTime().asSeconds();
    float horiz_dist = m_speed * time;
    float z = INITIAL_SHOT_HEIGHT + m_speed * time - GRAVITY * time * time / 2.0;
    Vector2f xy = m_origin + m_direction * horiz_dist;
    return Vector3f(xy.x, xy.y, z);
}
