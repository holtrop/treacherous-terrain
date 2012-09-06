
#include <math.h>
#include <string.h>
#include "GLMatrix.h"

const GLMatrix GLMatrix::Identity;

GLMatrix::GLMatrix()
{
    set_identity();
}

void GLMatrix::set_identity()
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
        GLfloat near, GLfloat far)
{
    GLMatrix mult;
    GLfloat rl = right - left;
    GLfloat tb = top - bottom;
    GLfloat fn = far - near;
    mult.m_mat[0][0] = 2 * near / rl;
    mult.m_mat[2][0] = (right + left) / rl;
    mult.m_mat[1][1] = 2 * near / tb;
    mult.m_mat[2][1] = (top + bottom) / tb;
    mult.m_mat[2][2] = - (far + near) / fn;
    mult.m_mat[3][2] = - 2 * far * near / fn;
    mult.m_mat[2][3] = -1.0;
    mult.m_mat[3][3] = 0.0f;
    multiply(mult);
}

void GLMatrix::perspective(GLfloat fovy, GLfloat aspect,
        GLfloat near, GLfloat far)
{
    GLMatrix mult;
    GLfloat f = 1.0 / tan(M_PI * fovy / 360.0);
    GLfloat nf = near - far;
    mult.m_mat[0][0] = f / aspect;
    mult.m_mat[1][1] = f;
    mult.m_mat[2][2] = (far + near) / nf;
    mult.m_mat[3][2] = (2 * far * near) / nf;
    mult.m_mat[2][3] = -1.0;
    mult.m_mat[3][3] = 0.0f;
    multiply(mult);
}

void GLMatrix::ortho(GLfloat left, GLfloat right,
        GLfloat bottom, GLfloat top,
        GLfloat near, GLfloat far)
{
    GLMatrix mult;
    GLfloat rl = right - left;
    GLfloat tb = top - bottom;
    GLfloat fn = far - near;
    mult.m_mat[0][0] = 2 / rl;
    mult.m_mat[3][0] = - (right + left) / rl;
    mult.m_mat[1][1] = 2 / tb;
    mult.m_mat[3][1] = - (top + bottom) / tb;
    mult.m_mat[2][2] = -2 / fn;
    mult.m_mat[3][2] = - (far + near) / fn;
    multiply(mult);
}
