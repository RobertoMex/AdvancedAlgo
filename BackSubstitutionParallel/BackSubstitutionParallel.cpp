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
 * @brief Compute one block in the parallel back substitution. The block shape is N by NBlock; NBlock must divide N. Supports any number of right-hand sides.
 * 
 * @tparam TScalar The scalar type (should usually be float or double -- int will not work)
 * @tparam N Matrix size
 * @tparam NBlock Number of columns in this block. Must divide N.
 * @tparam K Number of right hand sides
 * @param blockIndex The zero-based index of this block (top to bottom).
 * @param mat Should be initialized with the corresponding block of the upper-triangular matrix A.
 * @param rhs The block of the right hand side.
 * @param messageQueue The queue for communication across threads.
 * @param populateResult Whether to skip messaging and simply populate the result vectors for this block (for sequential solve).
 * @param result The result vectors
 */
template <typename TScalar, std::size_t N, std::size_t NBlock, std::size_t K>
void backSubBlockIter(std::size_t blockIndex, Matrix<TScalar, NBlock, N> mat, Matrix<TScalar, NBlock, K> rhs, MessageQueue<Matrix<TScalar, NBlock, K>> &messageQueue, bool populateResult, Matrix<TScalar, N, K> &result)
{
    Client<Matrix<TScalar, NBlock, K>> client = messageQueue.getClient();
    std::size_t firstIdx = NBlock * blockIndex;
    std::size_t p = N / NBlock;
    Matrix<TScalar, NBlock, K> subcolumn([mat, rhs, firstIdx](std::size_t rowIdx, std::size_t colIdx)
                                         { return rhs.get(rowIdx, colIdx) / mat.get(rowIdx, rowIdx + firstIdx); });

    for (std::size_t k = 0; k != p - 1 - blockIndex; ++k)
    {
        std::size_t incomingBlockIndex = p - 1 - k;
        std::size_t incomingFirstIdx = NBlock * incomingBlockIndex;

        while (!messageQueue.hasNext(client))
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        Matrix<TScalar, NBlock, K> values = messageQueue.next(client);

        for (std::size_t n = 0; n != K; ++n)
        {
            for (std::size_t i = 0; i != NBlock; ++i)
            {
                TScalar val = values.get(i, n);
                std::size_t valIdx = incomingFirstIdx + i;
                for (std::size_t j = 0; j != NBlock; ++j)
                {
                    TScalar cur = subcolumn.get(j, n);
                    TScalar update = val * mat.get(j, valIdx) / mat.get(j, j + firstIdx);
                    subcolumn.set(j, n, cur - update);
                }
            }
        }
    }

    for (std::size_t n = 0; n != K; ++n)
    {
        for (std::size_t i = 0; i != NBlock; ++i)
        {
            std::size_t idx = NBlock - 1 - i;
            TScalar nextVal = subcolumn.get(idx, n);
            for (std::size_t j = idx + 1; j != NBlock; ++j)
            {
                TScalar update = subcolumn.get(j, n) * mat.get(idx, j + firstIdx) / mat.get(idx, idx + firstIdx);
                nextVal -= update;
            }
            if (populateResult)
            {
                result.set(idx + firstIdx, n, nextVal);
            }
            subcolumn.set(idx, n, nextVal);
        }
    }

    if (!populateResult)
    {
        messageQueue.enqueue(subcolumn, client);
    }
}

template <typename TScalar, std::size_t N, std::size_t K>
void computeBackSubstitutionSequential(const Matrix<TScalar, N, N> &mat, const Matrix<TScalar, N, K> &rhs, Matrix<TScalar, N, K> &result)
{
    auto timerStart = std::chrono::steady_clock::now();
    MessageQueue<Matrix<TScalar, N, K>> messageQueue;
    backSubBlockIter<TScalar, N, N>(0, mat, rhs, messageQueue, true, result);
    auto timerStop = std::chrono::steady_clock::now();
    std::chrono::duration<double> milliseconds = timerStop - timerStart;
    int count = 1000 * milliseconds.count();

    Matrix<TScalar, N, K> computed;
    mat.multiplyRight(result, computed);
    computed.add(rhs, -1.0);
    TScalar frobResidual = computed.frobNorm();
    TScalar frobRhs = rhs.frobNorm();
    TScalar frobFractional = 100.0 * frobResidual / frobRhs;

    std::cout << "Sequential Back Substitution, N = " << N << ", K = " << K << std::endl;
    std::cout << "Milliseconds: " << count << std::endl;
    std::cout << "Percent residual (Frobenius): " << frobFractional << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
}

template <typename TScalar, std::size_t N, std::size_t NBlock, std::size_t K>
void computeBackSubstitutionParallel(const Matrix<TScalar, N, N> &mat, const Matrix<TScalar, N, K> &rhs, Matrix<TScalar, N, K> &result)
{
    MessageQueue<Matrix<TScalar, NBlock, K>> messageQueue;
    Client<Matrix<TScalar, NBlock, K>> client = messageQueue.getClient();
    const std::size_t p = N / NBlock;

    auto timerStart = std::chrono::steady_clock::now();

    std::vector<std::thread> threads{};
    for (std::size_t i = 0; i != p; ++i)
    {
        std::size_t firstIdx = i * NBlock;
        Matrix<TScalar, NBlock, N> submatrix;
        mat.rowsInto(firstIdx, submatrix);
        Matrix<TScalar, NBlock, K> subrhs;
        rhs.rowsInto(firstIdx, subrhs);

        std::thread thread = std::thread(backSubBlockIter<TScalar, N, NBlock, K>, i, submatrix, subrhs, std::ref(messageQueue), false, std::ref(result));
        threads.push_back(std::move(thread));
    }

    for (std::size_t i = 0; i != p; ++i)
    {
        threads[i].join();
    }

    for (std::size_t i = 0; i != p; ++i)
    {
        std::size_t blockIndex = p - 1 - i;
        if (!messageQueue.hasNext(client))
        {
            std::cout << "FATAL ERROR: parallel chol alg aborted unexpectedly" << std::endl;
            return;
        }

        const Matrix<TScalar, NBlock, K> values = messageQueue.next(client);
        result.overwriteSubmatrix(values, NBlock * blockIndex, 0);
    }

    auto timerStop = std::chrono::steady_clock::now();
    std::chrono::duration<double> milliseconds = timerStop - timerStart;
    int count = 1000 * milliseconds.count();

    Matrix<TScalar, N, K> computed;
    mat.multiplyRight(result, computed);
    computed.add(rhs, -1.0);
    TScalar frobResidual = computed.frobNorm();
    TScalar frobRhs = rhs.frobNorm();
    TScalar frobFractional = 100.0 * frobResidual / frobRhs;

    std::cout << "Parallel Back Substitution, N = " << N << ", K = " << K << ", p = " << p << std::endl;
    std::cout << "Milliseconds: " << count << std::endl;
    std::cout << "Percent residual (Frobenius): " << frobFractional << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
}

template <std::size_t N, std::size_t NBlock, std::size_t K>
void calculateBackSubstitution(bool useParallel)
{
    std::mt19937 rng;
    rng.seed(11828);
    std::normal_distribution<float> normal_dist(0.0, 1.0 / N);
    Matrix<float, N, N> mat([rng, normal_dist](std::size_t rowIdx, std::size_t colIdx) mutable
                            {
                                if (rowIdx > colIdx)
                                {
                                    return (float)0;
                                }
                                float val = normal_dist(rng);
                                return (rowIdx == colIdx) ? (val + 1) : val;
                            });
    Matrix<float, N, K> rhs([rng, normal_dist](std::size_t rowIdx, std::size_t colIdx) mutable
                            { return (float)(normal_dist(rng) * N); });
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "Matrix entry-wise sample standard deviation: " << mat.sample_dev() << std::endl;
    std::cout << "RHS entry-wise sample standard deviation: " << rhs.sample_dev() << std::endl;
    std::cout << "---------------------------" << std::endl;
    Matrix<float, N, K> result;
    if (useParallel)
    {
        computeBackSubstitutionParallel<float, N, NBlock, K>(mat, rhs, result);
    }
    else
    {
        computeBackSubstitutionSequential<float, N, K>(mat, rhs, result);
    }
    std::cout << std::endl;
}

int main()
{
    calculateBackSubstitution<512, 512, 51>(false);
    calculateBackSubstitution<512, 64, 51>(true);

    calculateBackSubstitution<1024, 1024, 102>(false);
    calculateBackSubstitution<1024, 128, 102>(true);

    calculateBackSubstitution<2048, 2048, 205>(false);
    calculateBackSubstitution<2048, 256, 205>(true);

    calculateBackSubstitution<4096, 4096, 410>(false);
    calculateBackSubstitution<4096, 512, 410>(true);

    calculateBackSubstitution<8192, 8192, 819>(false);
    calculateBackSubstitution<8192, 1024, 819>(true);
}
