#include "powmatrix.h"

template <typename T>
PowMatrix<T>::PowMatrix(const size_t n)
{
    if (n + 1 >= sizeof(size_t) * 8)
    {
        throw std::overflow_error("Exponent too large"); // Prevent overflow
    }
    height_ = n;
    size_ = static_cast<size_t>(1) << n; // Compute 2^n
    square_size_ = size_ * size_;
    data_ = std::make_unique<T[]>(square_size_);
}

template <typename T>
PowMatrix<T>::PowMatrix(size_t n, const T *value)
{
    if (n + 1 >= sizeof(size_t) * 8)
    {
        throw std::overflow_error("Exponent too large"); // Prevent overflow
    }
    height_ = n;
    size_ = static_cast<size_t>(1) << n; // Compute 2^n
    square_size_ = size_ * size_;
    data_ = std::make_unique<T[]>(square_size_);
    for (size_t i = 0; i < square_size_; i++)
    {
        data_[i] = value[i];
    }
}

template <typename T>
T &PowMatrix<T>::operator()(size_t iindex, size_t jindex)
{
    if (iindex >= size_ || jindex >= size_)
    {
        throw std::out_of_range("Index out of range");
    }
    return data_[iindex * size_ + jindex];
}

template <typename T>
const T &PowMatrix<T>::operator()(size_t iindex, size_t jindex) const
{
    if (iindex >= size_ || jindex >= size_)
    {
        throw std::out_of_range("Index out of range");
    }
    return data_[iindex * size_ + jindex];
}

template <typename T>
T PowMatrix<T>::value() const
{
    if (height_ != 0)
    {
        throw std::runtime_error("Matrix is not a scalar");
    }
    return data_[0];
}

template <typename T>
PowMatrix<T> PowMatrix<T>::top_left() const
{
    auto part = PowMatrix(height() - 1);
    for (size_t i = 0; i < part.size(); i++)
    {
        for (size_t j = 0; j < part.size(); j++)
        {
            part(i, j) = (*this)(i, j);
        }
    }
    return part;
}

template <typename T>
PowMatrix<T> PowMatrix<T>::top_right() const
{
    auto part = PowMatrix(height() - 1);
    for (size_t i = 0; i < part.size(); i++)
    {
        for (size_t j = 0; j < part.size(); j++)
        {
            part(i, j) = (*this)(i, j + part.size());
        }
    }
    return part;
}

template <typename T>
PowMatrix<T> PowMatrix<T>::bottom_left() const
{
    auto part = PowMatrix(height() - 1);
    for (size_t i = 0; i < part.size(); i++)
    {
        for (size_t j = 0; j < part.size(); j++)
        {
            part(i, j) = (*this)(i + part.size(), j);
        }
    }
    return part;
}

template <typename T>
PowMatrix<T> PowMatrix<T>::bottom_right() const
{
    auto part = PowMatrix(height() - 1);
    for (size_t i = 0; i < part.size(); i++)
    {
        for (size_t j = 0; j < part.size(); j++)
        {
            part(i, j) = (*this)(i + part.size(), j + part.size());
        }
    }
    return part;
}

#include "amplitude.h"
#include "absi.h"

template class PowMatrix<ampl::Amplitude>;
template class PowMatrix<absi::Interval>;
