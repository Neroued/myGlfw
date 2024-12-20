#include <iostream>
#include <NSMatrix.h>
#include <FEMatrix.h>
#include <Mesh.h>
#include <fem.h>
#include <NavierStokesSolver.h>
#include <unordered_map>
#include <vec3.h>
#include <utility>

static double test_f(const Vec3 &pos, double omega_0 = 1.0, double sigma = 1.0)
{
    using namespace std;

    double x = pos[0];
    double y = pos[1];
    double z = pos[2];

    double r_squared = z * z;
    double theta = std::atan2(std::sqrt(x * x + y * y), z);
    double omega;

    // omega = 100 * z * std::exp(-50 * z * z) * (1 + 0.5 * cos(20 * theta));

    // 基于二维高斯分布生成涡量
    omega = omega_0 * std::exp(-r_squared / (2.0 * sigma * sigma)) * (1.0 + 0.5 * std::cos(20.0 * x * y) * z);

    return omega;
}

static size_t computeMeshBandwith(Mesh &m)
{
    CSRMatrix tmp(m);
    size_t sum = 0;
    for (size_t row = 0; row < tmp.rows; ++row)
    {
        size_t offset = tmp.row_offset[row];
        int len = tmp.row_offset[row + 1] - row;

        for (int i = 0; i < len - 1; ++i)
        {
            sum += tmp.elm_idx[i + 1] - tmp.elm_idx[i];
        }
    }
    return sum;
}

void printStrangeElm(CSRMatrix &S1)
{
    for (size_t row = 0; row < S1.rows; ++row)
    {
        size_t offset = S1.row_offset[row];
        int len = S1.row_offset[row + 1] - offset;
        for (int i = 0; i < len; ++i)
        {
            if (S1.elements[offset + i] == 0)
            {
                std::cout << "row: " << row << " index: " << S1.elm_idx[offset + i] << std::endl;
            }
        }
    }
}

struct PairHash
{
    std::size_t operator()(const std::pair<uint32_t, uint32_t> &p) const
    {
        return std::hash<uint32_t>{}(p.first) ^ (std::hash<uint32_t>{}(p.second) << 1);
    }
};

struct PairEqual
{
    bool operator()(const std::pair<uint32_t, uint32_t> &p1, const std::pair<uint32_t, uint32_t> &p2) const
    {
        return p1.first == p2.first && p1.second == p2.second;
    }
};

int main()
{
    // Mesh mesh(2, SPHERE);
    // NSMatrix M1(mesh);
    // buildMassMatrix(M1);
    // M1.print();
    // FEMatrix M2(mesh, FEMatrix::P1_Mass);
    // buildMassMatrix(M2);
    // M2.print();

    // NSMatrix S1(mesh);
    // buildStiffnessMatrix(S1);
    // S1.print();
    // FEMatrix S2(mesh, FEMatrix::P1_Stiffness);
    // buildStiffnessMatrix(S2);
    // S2.print();

    // printStrangeElm(S1);

    // 测试生成不同生成网格方法的优劣，计算每行中相邻元素距离之和，使用最小的
    // Mesh test_mesh(50, CUBE);
    // std::cout << "Bandwith: " << computeMeshBandwith(test_mesh) << std::endl;

    // int subdiv = 2;

    // NavierStokesSolver solver(subdiv, SPHERE);
    // for (size_t i = 0; i < solver.Omega.size; ++i)
    // {
    //     solver.Omega[i] = test_f(solver.mesh.vertices[i], 1.0, 1.5);
    // }
    // solver.setZeroMean(solver.Omega);

    // for (size_t t = 0; t < 20; ++t)
    // {
    //     std::cout << solver.Omega[solver.Omega.size - t - 1] << " ";
    // }
    // std::cout << std::endl;

    // int n = 5;
    // double dt = 0.005;
    // double nu = 1e-3;
    // while (n-- > 0)
    // {
    //     solver.timeStep(dt, nu);
    //     std::cout << "Omega: ";
    //     for (size_t t = 0; t < 20; ++t)
    //     {
    //         std::cout << solver.Omega[solver.Omega.size - t - 1] << " ";
    //     }
    //     std::cout << std::endl;
    //     std::cout << "Psi: ";
    //     for (size_t t = 0; t < 20; ++t)
    //     {
    //         std::cout << solver.Psi[solver.Psi.size - t - 1] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    int subdiv = 1;
    Mesh mesh(subdiv, CUBE);

    std::unordered_map<std::pair<uint32_t, uint32_t>, bool, PairHash, PairEqual> hash;

    for (size_t t = 0; t < mesh.triangle_count(); ++t)
    {
        uint32_t a = mesh.indices[3 * t + 0];
        uint32_t b = mesh.indices[3 * t + 1];
        uint32_t c = mesh.indices[3 * t + 2];

        std::cout << "triangle: (" << a << ", " << b << ", " << c << ")" << std::endl;
    }

    for (size_t t = 0; t < mesh.triangle_count(); ++t)
    {
        uint32_t a = mesh.indices[3 * t + 0];
        uint32_t b = mesh.indices[3 * t + 1];
        uint32_t c = mesh.indices[3 * t + 2];

        std::pair<uint32_t, uint32_t> e1 = {a, b};
        std::pair<uint32_t, uint32_t> e2 = {b, c};
        std::pair<uint32_t, uint32_t> e3 = {c, a};

        std::vector<std::pair<uint32_t, uint32_t>> edges = {e1, e2, e3};

        for (auto e : edges)
        {
            if (!hash[e])
            {
                hash[e] = true;
            }
            else
            {   
                // std::cout << "edge: (" << e.second << ", " << e.first << "): " << hash[{e.second, e.first}] << std::endl;
                std::cout << "t = " << t << " edge: (" << e.first << ", " << e.second << ")" << std::endl;
                std::cout << "triangle: (" << a << ", " << b << ", " << c << ")" << std::endl << std::endl;
            }
        }
    }
}