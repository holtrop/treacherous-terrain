
#include <math.h>
#include <string.h>
#include "GLMatrix.h"

const GLMatrix GLMatrix::Identity;

static void normalize(GLfloat * vec)
{
    GLfloat len = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

static void cross(GLfloat * out, GLfloat * in1, GLfloat * in2)
{
    out[0] = in1[1] * in2[2] - in1[2] * in2[1];
    out[1] = in1[2] * in2[0] - in1[0] * in2[2];
    out[2] = in1[0] * in2[1] - in1[1] * in2[0];
}

GLMatrix::GLMatrix()
{
    load_identity();
}

GLMatrix::GLMatrix(const GLMatrix & orig)
{
    memcpy(&m_mat, &orig.m_mat, sizeof(m_mat));
}

void GLMatrix::load_identity()
{
    memset(m_mat, 0, sizeof(m_mat));
    for (int i = 0; i < 4; i++)
        m_mat[i][i] = 1.0f;
}

GLMatrix operator*(const GLMatrix & left, const GLMatrix & right)
{
    GLMatrix result = left;
    result.multiply(right);
    return result;
}

void GLMatrix::multiply(const GLMatrix & other)
{
    GLMatrix tmp = *this;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            GLfloat v = 0.0;
            for (int p = 0; p < 4; p++)
            {
                v += tmp.m_mat[p][i] * other.m_mat[j][p];
            }
            m_mat[j][i] = v;
        }
    }
}

void GLMatrix::translate(GLfloat x, GLfloat y, GLfloat z)
{
    for (int i = 0; i < 4; i++)
    {
        m_mat[3][i] += m_mat[0][i] * x + m_mat[1][i] * y + m_mat[2][i] * z;
    }
}

void GLMatrix::scale(GLfloat x, GLfloat y, GLfloat z)
{
    for (int i = 0; i < 4; i++)
    {
        m_mat[0][i] *= x;
        m_mat[1][i] *= y;
        m_mat[2][i] *= z;
    }
}

void GLMatrix::rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat p = x*x + y*y + z*z;
    if (p != 1.0f)
    {
        GLfloat scale = sqrt(p);
        x /= scale;
        y /= scale;
        z /= scale;
    }
    GLfloat c = cos(M_PI * angle / 180.0);
    GLfloat s = sin(M_PI * angle / 180.0);
    GLMatrix mult;
    GLfloat oc = 1 - c;
    mult.m_mat[0][0] = x * x * oc + c;
    mult.m_mat[1][0] = x * y * oc - z * s;
    mult.m_mat[2][0] = x * z * oc + y * s;
    mult.m_mat[3][0] = 0.0;
    mult.m_mat[0][1] = y * x * oc + z * s;
    mult.m_mat[1][1] = y * y * oc + c;
    mult.m_mat[2][1] = y * z * oc - x * s;
    mult.m_mat[3][1] = 0.0;
    mult.m_mat[0][2] = x * z * oc - y * s;
    mult.m_mat[1][2] = y * z * oc + x * s;
    mult.m_mat[2][2] = z * z * oc + c;
    mult.m_mat[3][2] = 0.0;
    mult.m_mat[0][3] = 0.0;
    mult.m_mat[1][3] = 0.0;
    mult.m_mat[2][3] = 0.0;
    mult.m_mat[3][3] = 1.0;
    *this = *this * mult;
}

void GLMatrix::frustum(GLfloat left, GLfloat right,
        GLfloat bottom, GLfloat top,
        GLfloat z_near, GLfloat z_far)
{
    GLMatrix mult;
    GLfloat rl = right - left;
    GLfloat tb = top - bottom;
    GLfloat fn = z_far - z_near;
    mult.m_mat[0][0] = 2 * z_near / rl;
    mult.m_mat[2][0] = (right + left) / rl;
    mult.m_mat[1][1] = 2 * z_near / tb;
    mult.m_mat[2][1] = (top + bottom) / tb;
    mult.m_mat[2][2] = - (z_far + z_near) / fn;
    mult.m_mat[3][2] = - 2 * z_far * z_near / fn;
    mult.m_mat[2][3] = -1.0;
    mult.m_mat[3][3] = 0.0f;
    multiply(mult);
}

void GLMatrix::look_at(GLfloat eye_x, GLfloat eye_y, GLfloat eye_z,
        GLfloat center_x, GLfloat center_y, GLfloat center_z,
        GLfloat up_x, GLfloat up_y, GLfloat up_z)
{
    GLfloat forward[3], side[3], up[3];
    forward[0] = center_x - eye_x;
    forward[1] = center_y - eye_y;
    forward[2] = center_z - eye_z;
    normalize(forward);
    up[0] = up_x;
    up[1] = up_y;
    up[2] = up_z;
    cross(side, forward, up);
    normalize(side);
    cross(up, side, forward);
    GLMatrix mult;
    mult.m_mat[0][0] = side[0];
    mult.m_mat[1][0] = side[1];
    mult.m_mat[2][0] = side[2];
    mult.m_mat[0][1] = up[0];
    mult.m_mat[1][1] = up[1];
    mult.m_mat[2][1] = up[2];
    mult.m_mat[0][2] = -forward[0];
    mult.m_mat[1][2] = -forward[1];
    mult.m_mat[2][2] = -forward[2];
    multiply(mult);
    translate(-eye_x, -eye_y, -eye_z);
}

void GLMatrix::perspective(GLfloat fovy, GLfloat aspect,
        GLfloat z_near, GLfloat z_far)
{
    GLMatrix mult;
    GLfloat f = 1.0 / tan(M_PI * fovy / 360.0);
    GLfloat nf = z_near - z_far;
    mult.m_mat[0][0] = f / aspect;
    mult.m_mat[1][1] = f;
    mult.m_mat[2][2] = (z_far + z_near) / nf;
    mult.m_mat[3][2] = (2 * z_far * z_near) / nf;
    mult.m_mat[2][3] = -1.0;
    mult.m_mat[3][3] = 0.0f;
    multiply(mult);
}

void GLMatrix::ortho(GLfloat left, GLfloat right,
        GLfloat bottom, GLfloat top,
        GLfloat z_near, GLfloat z_far)
{
    GLMatrix mult;
    GLfloat rl = right - left;
    GLfloat tb = top - bottom;
    GLfloat fn = z_far - z_near;
    mult.m_mat[0][0] = 2 / rl;
    mult.m_mat[3][0] = - (right + left) / rl;
    mult.m_mat[1][1] = 2 / tb;
    mult.m_mat[3][1] = - (top + bottom) / tb;
    mult.m_mat[2][2] = -2 / fn;
    mult.m_mat[3][2] = - (z_far + z_near) / fn;
    multiply(mult);
}

void GLMatrix::to_uniform(GLint uniform)
{
    glUniformMatrix4fv(uniform, 1, GL_FALSE, &m_mat[0][0]);
}

void GLMatrix::push()
{
    m_stack.push(Mat4_s());
    memcpy(&m_stack.top().mat, &m_mat, sizeof(m_mat));
}

void GLMatrix::pop()
{
    if (m_stack.size() > 0)
    {
        memcpy(&m_mat, &m_stack.top().mat, sizeof(m_mat));
        m_stack.pop();
    }
}
