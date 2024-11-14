#pragma once

#include <Matrix.h>
#include <TArray.h>
#include <iostream>
#include <stdexcept>
#include <vector>

typedef struct Cooef
{
    int i;
    int j;
    double val;
} Cooef;

class COOMatrix : public Matrix
// 按元素存储的稀疏矩阵
{
public:
    int nnz; //  非零元素的数量
    struct Cooef *cooefs;

    COOMatrix() : Matrix(0, 0), nnz(0), cooefs(nullptr) {}

    COOMatrix(int r, int c) : Matrix(r, c), nnz(0), cooefs(nullptr) {}

    COOMatrix(int r, int c, int nonZeroElements)
        : Matrix(r, c), nnz(nonZeroElements), cooefs(new Cooef[nonZeroElements]) {}

    ~COOMatrix() { delete[] cooefs; }

    // 禁用拷贝构造和赋值运算符
    COOMatrix(const COOMatrix &) = delete;
    COOMatrix &operator=(const COOMatrix &) = delete;

    void print();
    void MVP(const Vec &x, Vec &y) const; // Ax = y
};

void COOMatrix::MVP(const Vec &x, Vec &y) const
{
    // 初始化out
    y.setAll(0);
    if (cols != x.size || cols != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    // 稀疏矩阵-向量乘法
    struct Cooef *tmp;
    for (int idx = 0; idx < nnz; ++idx)
    {
        tmp = &cooefs[idx];
        y[tmp->i] += tmp->val * x[tmp->j];
    }
}


Vec MVP(const COOMatrix &M, const Vec &v)
{
    Vec out(M.rows, 0);

    if (M.cols != v.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    // 稀疏矩阵-向量乘法
    struct Cooef *tmp;
    for (int idx = 0; idx < M.nnz; ++idx)
    {
        tmp = &M.cooefs[idx];
        out[tmp->i] += tmp->val * v[tmp->j];
    }
    return out;
}

void MVP(const COOMatrix &M, const Vec &v, Vec &out)
{
    // 初始化out
    out.setAll(0);
    if (M.cols != v.size || M.cols != out.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    // 稀疏矩阵-向量乘法
    struct Cooef *tmp;
    for (int idx = 0; idx < M.nnz; ++idx)
    {
        tmp = &M.cooefs[idx];
        out[tmp->i] += tmp->val * v[tmp->j];
    }
}

void COOMatrix::print()
{
    for (int i = 0; i < nnz; ++i)
    {
        std::cout << "i: " << cooefs[i].i << " j: " << cooefs[i].j << " val: " << cooefs[i].val << std::endl;
    }
}