
#ifndef GLMATRIX_H
#define GLMATRIX_H

#include <GL/glew.h>

/**
 * Store a 4x4 matrix of GLfloats with associated operations.
 *
 * Matrices are stored in column-major order.
 */
class GLMatrix
{
    public:
        GLMatrix();
        void set_identity();
        void translate(GLfloat x, GLfloat y, GLfloat z);
        void scale(GLfloat x, GLfloat y, GLfloat z);
        void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
        void multiply(const GLMatrix & other);
        void frustum(GLfloat left, GLfloat right,
                GLfloat bottom, GLfloat top,
                GLfloat near, GLfloat far);
        void perspective(GLfloat fovy, GLfloat aspect,
                GLfloat near, GLfloat far);
        void ortho(GLfloat left, GLfloat right,
                GLfloat bottom, GLfloat top,
                GLfloat near, GLfloat far);

        friend GLMatrix operator*(const GLMatrix &, const GLMatrix &);

        static const GLMatrix Identity;

    protected:
        GLfloat m_mat[4][4];
};

#endif
