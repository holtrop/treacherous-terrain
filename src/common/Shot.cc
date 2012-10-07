
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
#define GRAVITY 100

#define SHOT_ANGLE 30

/* We model the shot's position using a parametric equation based on time.
 * x = vct
 * y = h + vst - gt²/2
 *   = (-g/2)t² + vst + h
 * where
 * s = sin(shot angle)
 * c = cos(shot angle)
 * v = shot speed
 * t = time
 * g = gravity
 * h = INITIAL_SHOT_HEIGHT
 *
 * Given a target distance of d, we want to figure out a speed that makes
 * (x, y) = (d, 0) a valid point on the trajectory.
 * Then:
 * d = vct
 * 0 = (-g/2)t² + vst + h
 * So:
 * v = d/ct
 * 0 = (-g/2)t² + (d/ct)st + h
 * 0 = (-g/2)t² + ds/c + h
 * According to the quadratic formula (x = (-b ± sqrt(b² - 4ac))/2a),
 * t = ±sqrt(-4(-g/2)(ds/c+h)) / 2(-g/2)
 * -tg = ±sqrt(2g(ds/c+h))
 * t²g² = 2g(ds/c+h)
 * t² = 2(ds/c+h)/g
 * t = sqrt(2(ds/c+h)/g)
 * Now that we know the time at which this point occurs, we can solve for
 * the shot speed (v)
 * v = d/ct
 * v = d / c / sqrt(2(ds/c+h)/g)
 */
Shot::Shot(const Vector2f & origin, double direction, double target_dist)
{
    m_direction = Vector2f(cos(direction), sin(direction));
    m_origin = origin;
    m_cos_a = cos(SHOT_ANGLE * M_PI / 180.0);
    m_sin_a = sin(SHOT_ANGLE * M_PI / 180.0);
    m_speed = target_dist / m_cos_a /
        sqrt(2 * (target_dist * m_sin_a / m_cos_a + INITIAL_SHOT_HEIGHT) /
                GRAVITY);
}

Vector3f Shot::get_position()
{
    float time = m_clock.getElapsedTime().asSeconds();
    float horiz_dist = m_speed * m_cos_a * time;
    float z = INITIAL_SHOT_HEIGHT + m_speed * m_sin_a * time -
        GRAVITY * time * time / 2.0;
    Vector2f xy = m_origin + m_direction * horiz_dist;
    return Vector3f(xy.x, xy.y, z);
}
