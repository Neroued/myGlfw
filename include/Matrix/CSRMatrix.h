#pragma once

#include <Mesh.h>
#include <Matrix.h>
#include <TArray.h>
#include <vector>
#include <iostream>

class CSRMatrix : public Matrix
// 按行存储的稀疏矩阵, 存储每行不为零的元素
{
public:
    Vec elements;
    TArray<size_t> row_offset;
    TArray<size_t> elm_idx;
    Mesh &mesh;

    CSRMatrix(Mesh &m); // 根据Mesh中每个顶点之间的连通性建立
    ~CSRMatrix() = default;

    void MVP(const Vec &x, Vec &y) const;
};

CSRMatrix::CSRMatrix(Mesh &m) : Matrix(m.vertex_count(), m.vertex_count()), row_offset(rows + 1, 1), mesh(m)
{
    /* 统计每个顶点对应的非零元素数量，然后初始化elements, row_offset 和 elm_idx
     * 根据构建网格的特征
     * 顶点首次在三角形中出现时，此时与三个点存在相互作用，值不为零
     * 之后每多出现一次，因为新的三角形必然与前一个出现的三角形共一条边，因此仅多一个非零元素
     * 最后一次出现，不增加新的顶点
     * 因此总数为 1 + 出现次数
     * row_offset前rows个元素为对应row的开始下标，最后多存储一个元素方便计算每一行的长度
     */
    // 统计每个点的出现次数
    for (size_t t = 0; t < m.triangle_count(); ++t)
    {
        row_offset[m.indices[3 * t + 0]] += 1;
        row_offset[m.indices[3 * t + 1]] += 1;
        row_offset[m.indices[3 * t + 2]] += 1;
    }

    // 将数量转换为偏移量
    for (size_t i = 0; i < rows - 1; ++i)
    {
        row_offset[i + 1] += row_offset[i];
    }

    // 将元素向后移动一位
    for (size_t i = rows; i > 0; --i)
    {
        row_offset[i] = row_offset[i - 1];
    }
    row_offset[0] = 0;

    size_t s = row_offset[rows]; // 此时最后一个元素为总和
    elements.resize(s);
    elements.setAll(0.0);

    elm_idx.resize(s);
    elm_idx.setAll(-1); // 设置为-1即size_t最大值，便于之后按大小顺序插入元素

    uint32_t a, b, c;
    for (size_t t = 0; t < m.triangle_count(); ++t)
    {
        a = m.indices[3 * t + 0];
        b = m.indices[3 * t + 1];
        c = m.indices[3 * t + 2];

        std::vector<uint32_t> triangle = {a, b, c};
        /* 对于当前三角形的每一个顶点，对于当前三角形对应的三个行，
         * 判断是否为第一次出现，并以从小到大的顺序插入
         */
        size_t offset;
        int len;
        for (uint32_t current_vtx : triangle)
        {
            for (uint32_t current_row : triangle)
            {
                // 定位当前行
                offset = row_offset[current_row];
                len = row_offset[current_row + 1] - offset;

                // 判断是否为第一次出现，若是则按大小顺序插入
                for (int i = 0; i < len; ++i)
                {
                    if (elm_idx[offset + i] == current_vtx)
                    {
                        break; // 出现过，直接跳过
                    }
                    else if (elm_idx[offset + i] == (size_t)(-1))
                    {
                        elm_idx[offset + i] = current_vtx;
                        break;
                    }
                }
            }
        }
    }

    // 插入后将每行按大小排序
    for (size_t row = 0; row < rows; ++row)
    {
        size_t offset = row_offset[row];
        int len = row_offset[row + 1] - offset;

        // 排序当前行的非零元素下标
        std::sort(elm_idx.begin() + offset, elm_idx.begin() + offset + len);
    }
}

void CSRMatrix::MVP(const Vec &x, Vec &y) const
{
    if (cols != x.size || cols != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }
    y.setAll(0.0);

    size_t offset;
    size_t len;

#pragma omp parallel for private(offset, len)
    for (int r = 0; r < rows; ++r)
    {
        offset = row_offset[r];
        len = row_offset[r + 1] - offset;
        double local_sum = 0.0; // 每个线程维护局部累加器
        for (size_t i = 0; i < len; ++i)
        {
            local_sum += elements[offset + i] * x[elm_idx[offset + i]];
        }
#pragma omp atomic
        y[r] += local_sum;
    }
}

void blas_addMatrix(const CSRMatrix &M, double val, const CSRMatrix &S, CSRMatrix &A)
// 计算A = S + val * M
{
    for (size_t t = 0; t < S.elements.size; ++t)
    {
        A.elements[t] = M.elements[t] * val + S.elements[t];
    }
}