
#ifndef REFPTR_H
#define REFPTR_H REFPTR_H

/* Author: Josh Holtrop
 * Purpose: Provide a reference-counting pointer-like first order
 *   C++ object that will free the object it is pointing to when
 *   all references to it have been destroyed.
 * This implementation does not solve the circular reference problem.
 * I was not concerned with that when developing this class.
 */
#include <stdlib.h>             /* NULL */

template <typename T>
class refptr
{
    public:
        refptr<T>();
        refptr<T>(T * ptr);
        refptr<T>(const refptr<T> & orig);
        refptr<T> & operator=(const refptr<T> & orig);
        refptr<T> & operator=(T * ptr);
        ~refptr<T>();
        T & operator*() const { return *m_ptr; }
        T * operator->() const { return m_ptr; }
        bool isNull() const { return m_ptr == NULL; }

    private:
        void cloneFrom(const refptr<T> & orig);
        void destroy();

        T * m_ptr;
        int * m_refCount;
};

template <typename T> refptr<T>::refptr()
{
    m_ptr = NULL;
    m_refCount = NULL;
}

template <typename T> refptr<T>::refptr(T * ptr)
{
    m_ptr = ptr;
    m_refCount = new int;
    *m_refCount = 1;
}

template <typename T> refptr<T>::refptr(const refptr<T> & orig)
{
    cloneFrom(orig);
}

template <typename T> refptr<T> & refptr<T>::operator=(const refptr<T> & orig)
{
    destroy();
    cloneFrom(orig);
    return *this;
}

template <typename T> refptr<T> & refptr<T>::operator=(T * ptr)
{
    destroy();
    m_ptr = ptr;
    m_refCount = new int;
    *m_refCount = 1;
    return *this;
}

template <typename T> void refptr<T>::cloneFrom(const refptr<T> & orig)
{
    this->m_ptr = orig.m_ptr;
    this->m_refCount = orig.m_refCount;
    if (m_refCount != NULL)
        (*m_refCount)++;
}

template <typename T> refptr<T>::~refptr()
{
    destroy();
}

template <typename T> void refptr<T>::destroy()
{
    if (m_refCount != NULL)
    {
        if (*m_refCount <= 1)
        {
            delete m_ptr;
            delete m_refCount;
        }
        else
        {
            (*m_refCount)--;
        }
    }
}

#endif
