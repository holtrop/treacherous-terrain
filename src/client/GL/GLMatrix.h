
#ifndef GLMATRIX_H
#define GLMATRIX_H

#include GL_INCLUDE_FILE
#include <stack>

/**
 * Store a 4x4 matrix of floats with associated operations.
 *
 * Matrices are stored in column-major order.
 */
class GLMatrix
{
    public:
        GLMatrix();
        GLMatrix(const GLMatrix & orig);
        void load_identity();
        void translate(GLfloat x, GLfloat y, GLfloat z);
        void scale(GLfloat x, GLfloat y, GLfloat z);
        void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
        void multiply(const GLMatrix & other);
        void frustum(GLfloat left, GLfloat right,
                GLfloat bottom, GLfloat top,
                GLfloat z_near, GLfloat z_far);
        void look_at(GLfloat eye_x, GLfloat eye_y, GLfloat eye_z,
                GLfloat center_x, GLfloat center_y, GLfloat center_z,
                GLfloat up_x, GLfloat up_y, GLfloat up_z);
        void perspective(GLfloat fovy, GLfloat aspect,
                GLfloat z_near, GLfloat z_far);
        void ortho(GLfloat left, GLfloat right,
                GLfloat bottom, GLfloat top,
                GLfloat z_near, GLfloat z_far);
        void to_uniform(GLint uniform) const;

        void push();
        void pop();

        friend GLMatrix operator*(const GLMatrix &, const GLMatrix &);

        static const GLMatrix Identity;

    protected:
        typedef GLfloat Mat4[4][4];
        Mat4 m_mat;
        struct Mat4_s { Mat4 mat; };
        std::stack<Mat4_s> m_stack;
};

#endif
