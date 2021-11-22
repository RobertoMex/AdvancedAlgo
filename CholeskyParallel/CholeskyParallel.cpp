#define _USE_MATH_DEFINES

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <vector>
#include <random>

#include "../MessageQueue/MessageQueue.hpp"
#include "../Matrix/Matrix.hpp"

/**
 * @brief Compute one block in the parallel Cholesky. The block shape is N by NBlock; NBlock must divide N.
 * 
 * The decomposition is
 * 
 * A = L * L^T
 * 
 * The results are enqueued successively, from left to right, and each message allows a column of L to be
 * constructed. Thus the calling thread is responsible for constructing L; this reconstruction is a lower-order
 * cost and thus does not need its own parallelism.
 * 
 * @tparam TScalar The scalar type (should usually be float or double -- int will not work)
 * @tparam N Matrix size
 * @tparam NBlock Number of columns in this block. Must divide N.
 * @param blockIndex The zero-based index of this block (left to right).
 * @param mat Should be initialized with the corresponding block of the matrix A.
 * @param messageQueue The queue for communication across threads.
 */
template <typename TScalar, std::size_t N, std::size_t NBlock>
void cholBlockIter(std::size_t blockIndex, Matrix<TScalar, N, NBlock> mat, MessageQueue<Matrix<TScalar, N, 1>> &messageQueue)
{
    Client<Matrix<TScalar, N, 1>> client = messageQueue.getClient();
    std::size_t firstIdx = NBlock * blockIndex;

    for (std::size_t i = 0; i != blockIndex * NBlock; ++i)
    {
        while (!messageQueue.hasNext(client))
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        Matrix<TScalar, N, 1> column = messageQueue.next(client);
        const TScalar diagElem = column.get(i, 0);
        column.set(i, 0, 0);
        Matrix<TScalar, N, NBlock> outerProduct;
        Matrix<TScalar, NBlock, 1> subcolumn;
        column.rowsInto(firstIdx, subcolumn);
        subcolumn.transpose().multiplyLeft(column, outerProduct);
        mat.add(outerProduct, -1.0 / diagElem);
        Matrix<TScalar, 1, NBlock> zeroRow;
        mat.overwriteSubmatrix(zeroRow, i, 0);
    }

    for (std::size_t i = 0; i != NBlock; ++i)
    {
        Matrix<TScalar, N, 1> column = mat.column(i);
        for (std::size_t j = 0; j != i + firstIdx; ++j)
        {
            column.set(j, 0, 0);
        }
        messageQueue.enqueue(column, client);
        const TScalar diagElem = column.get(i + firstIdx, 0);
        column.set(i + firstIdx, 0, 0);
        Matrix<TScalar, N, NBlock> outerProduct;
        Matrix<TScalar, NBlock, 1> subcolumn;
        column.rowsInto(firstIdx, subcolumn);
        subcolumn.transpose().multiplyLeft(column, outerProduct);
        mat.add(outerProduct, -1.0 / diagElem);
        Matrix<TScalar, 1, NBlock> zeroRow;
        Matrix<TScalar, N, 1> zeroCol;
        mat.overwriteSubmatrix(zeroRow, i + firstIdx, 0);
        mat.overwriteSubmatrix(zeroCol, 0, i);
        mat.set(i + firstIdx, i, 1);
    }
}

template <typename TScalar, std::size_t N, std::size_t NBlock>
void computeCholeskyParallel(const Matrix<TScalar, N, N> &mat, Matrix<TScalar, N, N> &result)
{
    MessageQueue<Matrix<TScalar, N, 1>> messageQueue;
    Client<Matrix<TScalar, N, 1>> client = messageQueue.getClient();
    const std::size_t p = N / NBlock;

    auto timerStart = std::chrono::steady_clock::now();

    std::vector<std::thread> threads{};
    for (std::size_t i = 0; i != p; ++i)
    {
        std::size_t firstCol = i * NBlock;
        Matrix<TScalar, N, NBlock> submatrix;
        mat.columnsInto(firstCol, submatrix);
        std::thread thread = std::thread(cholBlockIter<TScalar, N, NBlock>, i, submatrix, std::ref(messageQueue));
        threads.push_back(std::move(thread));
    }

    for (std::size_t i = 0; i != p; ++i)
    {
        threads[i].join();
    }

    for (std::size_t i = 0; i != N; ++i)
    {
        if (!messageQueue.hasNext(client))
        {
            std::cout << "FATAL ERROR: parallel chol alg aborted unexpectedly" << std::endl;
            return;
        }

        Matrix<TScalar, N, 1> column = messageQueue.next(client);
        const TScalar diagElem = std::sqrt(std::abs(column.get(i, 0)));
        column.set(i, 0, diagElem);
        for (std::size_t j = i + 1; j != N; ++j)
        {
            TScalar val = column.get(j, 0) / diagElem;
            column.set(j, 0, val);
        }

        result.overwriteSubmatrix(column, 0, i);
    }

    auto timerStop = std::chrono::steady_clock::now();
    std::chrono::duration<double> milliseconds = timerStop - timerStart;
    int count = 1000 * milliseconds.count();

    Matrix<TScalar, N, N> computed;
    result.multiplyRight(result.transpose(), computed);
    computed.add(mat, -1.0);
    TScalar frobResidual = computed.frobNorm();
    TScalar frobMat = mat.frobNorm();
    TScalar frobFractional = 100.0 * frobResidual / frobMat;

    std::cout << "Parallel Cholesky, N = " << N << ", p = " << p << std::endl;
    std::cout << "Milliseconds: " << count << std::endl;
    std::cout << "Percent residual (Frobenius): " << frobFractional << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
}

int main()
{
    constexpr std::size_t N = 2000;
    constexpr std::size_t NBlock = 200;
    std::mt19937 rng;
    rng.seed(11828);
    std::normal_distribution<float> normal_dist(0.0, 10.0);
    Matrix<float, N, N> id([](std::size_t rowIdx, std::size_t colIdx)
                           { return rowIdx == colIdx ? 1 : 0; });
    Matrix<float, N, N> mat0([rng, normal_dist](std::size_t rowIdx, std::size_t colIdx) mutable
                             { return (float)(normal_dist(rng)); });
    Matrix<float, N, N> transpose = mat0.transpose();
    Matrix<float, N, N> mat;
    mat0.transpose().multiplyRight(mat0, mat);
    mat.multiplyScalar(1.0 / N);
    mat.add(id, 1.0);
    std::cout << "Matrix entry-wise sample standard deviation: " << mat.sample_dev() << std::endl;
    std::cout << "---------------------------" << std::endl;
    Matrix<float, N, N> result;
    computeCholeskyParallel<float, N, NBlock>(mat, result);
    return 0;
}