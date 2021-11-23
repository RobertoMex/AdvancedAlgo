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
 * @brief Compute one block in the parallel back substitution. The block shape is N by NBlock; NBlock must divide N.
 * 
 * @tparam TScalar The scalar type (should usually be float or double -- int will not work)
 * @tparam N Matrix size
 * @tparam NBlock Number of columns in this block. Must divide N.
 * @param blockIndex The zero-based index of this block (top to bottom).
 * @param mat Should be initialized with the corresponding block of the upper-triangular matrix A.
 * @param rhs The block of the right hand side.
 * @param messageQueue The queue for communication across threads.
 * @param populateResultVec Whether to skip messaging and simply populate the result vector for this block (for sequential solve).
 * @param resultVec The result vector
 */
template <typename TScalar, std::size_t N, std::size_t NBlock>
void backSubBlockIter(std::size_t blockIndex, Matrix<TScalar, NBlock, N> mat, Matrix<TScalar, NBlock, 1> rhs, MessageQueue<Matrix<TScalar, NBlock, 1>> &messageQueue, bool populateResultVec, Matrix<TScalar, N, 1> &resultVec)
{
    Client<Matrix<TScalar, NBlock, 1>> client = messageQueue.getClient();
    std::size_t firstIdx = NBlock * blockIndex;
    std::size_t p = N / NBlock;
    Matrix<TScalar, NBlock, 1> subcolumn([mat, rhs, firstIdx](std::size_t rowIdx, std::size_t colIdx)
                                         { return rhs.get(rowIdx, 0) / mat.get(rowIdx, rowIdx + firstIdx); });

    for (std::size_t k = 0; k != p - 1 - blockIndex; ++k)
    {
        std::size_t incomingBlockIndex = p - 1 - k;
        std::size_t incomingFirstIdx = NBlock * incomingBlockIndex;

        while (!messageQueue.hasNext(client))
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        Matrix<TScalar, NBlock, 1> values = messageQueue.next(client);

        for (std::size_t i = 0; i != NBlock; ++i)
        {
            TScalar val = values.get(i, 0);
            std::size_t valIdx = incomingFirstIdx + i;
            for (std::size_t j = 0; j != NBlock; ++j)
            {
                TScalar cur = subcolumn.get(j, 0);
                TScalar update = val * mat.get(j, valIdx) / mat.get(j, j + firstIdx);
                subcolumn.set(j, 0, cur - update);
            }
        }
    }

    for (std::size_t i = 0; i != NBlock; ++i)
    {
        std::size_t idx = NBlock - 1 - i;
        TScalar nextVal = subcolumn.get(idx, 0);
        for (std::size_t j = idx + 1; j != NBlock; ++j)
        {
            TScalar update = subcolumn.get(j, 0) * mat.get(idx, j + firstIdx) / mat.get(idx, idx + firstIdx);
            nextVal -= update;
        }
        if (populateResultVec)
        {
            resultVec.set(idx + firstIdx, 0, nextVal);
        }
        subcolumn.set(idx, 0, nextVal);
    }

    if (!populateResultVec)
    {
        messageQueue.enqueue(subcolumn, client);
    }
}

template <typename TScalar, std::size_t N>
void computeBackSubstitutionSequential(const Matrix<TScalar, N, N> &mat, const Matrix<TScalar, N, 1> &rhs, Matrix<TScalar, N, 1> &result)
{
    auto timerStart = std::chrono::steady_clock::now();
    MessageQueue<Matrix<TScalar, N, 1>> messageQueue;
    backSubBlockIter<TScalar, N, N>(0, mat, rhs, messageQueue, true, result);
    auto timerStop = std::chrono::steady_clock::now();
    std::chrono::duration<double> milliseconds = timerStop - timerStart;
    int count = 1000 * milliseconds.count();

    Matrix<TScalar, N, 1> computed;
    mat.multiplyRight(result, computed);
    computed.add(rhs, -1.0);
    TScalar frobResidual = computed.frobNorm();
    TScalar frobRhs = rhs.frobNorm();
    TScalar frobFractional = 100.0 * frobResidual / frobRhs;

    std::cout << "Sequential Back Substitution, N = " << N << std::endl;
    std::cout << "Milliseconds: " << count << std::endl;
    std::cout << "Percent residual (Frobenius): " << frobFractional << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
}

template <typename TScalar, std::size_t N, std::size_t NBlock>
void computeBackSubstitutionParallel(const Matrix<TScalar, N, N> &mat, const Matrix<TScalar, N, 1> &rhs, Matrix<TScalar, N, 1> &result)
{
    MessageQueue<Matrix<TScalar, NBlock, 1>> messageQueue;
    Client<Matrix<TScalar, NBlock, 1>> client = messageQueue.getClient();
    const std::size_t p = N / NBlock;

    auto timerStart = std::chrono::steady_clock::now();

    std::vector<std::thread> threads{};
    for (std::size_t i = 0; i != p; ++i)
    {
        std::size_t firstIdx = i * NBlock;
        Matrix<TScalar, NBlock, N> submatrix;
        mat.rowsInto(firstIdx, submatrix);
        Matrix<TScalar, NBlock, 1> subrhs;
        rhs.rowsInto(firstIdx, subrhs);

        std::thread thread = std::thread(backSubBlockIter<TScalar, N, NBlock>, i, submatrix, subrhs, std::ref(messageQueue), false, std::ref(result));
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

        const Matrix<TScalar, NBlock, 1> values = messageQueue.next(client);
        result.overwriteSubmatrix(values, NBlock * blockIndex, 0);
    }

    auto timerStop = std::chrono::steady_clock::now();
    std::chrono::duration<double> milliseconds = timerStop - timerStart;
    int count = 1000 * milliseconds.count();

    Matrix<TScalar, N, 1> computed;
    mat.multiplyRight(result, computed);
    computed.add(rhs, -1.0);
    TScalar frobResidual = computed.frobNorm();
    TScalar frobRhs = rhs.frobNorm();
    TScalar frobFractional = 100.0 * frobResidual / frobRhs;

    std::cout << "Parallel Back Substitution, N = " << N << ", p = " << p << std::endl;
    std::cout << "Milliseconds: " << count << std::endl;
    std::cout << "Percent residual (Frobenius): " << frobFractional << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
}

template <std::size_t N, std::size_t NBlock>
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
    Matrix<float, N, 1> rhs([rng, normal_dist](std::size_t rowIdx, std::size_t colIdx) mutable
                            { return (float)(normal_dist(rng) * N); });
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "Matrix entry-wise sample standard deviation: " << mat.sample_dev() << std::endl;
    std::cout << "RHS entry-wise sample standard deviation: " << rhs.sample_dev() << std::endl;
    std::cout << "---------------------------" << std::endl;
    Matrix<float, N, 1> result;
    if (useParallel)
    {
        computeBackSubstitutionParallel<float, N, NBlock>(mat, rhs, result);
    }
    else
    {
        computeBackSubstitutionSequential<float, N>(mat, rhs, result);
    }
    std::cout << std::endl;
}

int main()
{
    constexpr std::size_t N = 8192;
    calculateBackSubstitution<N, N>(false);
    calculateBackSubstitution<N, N / 2>(true);
    calculateBackSubstitution<N, N / 4>(true);
    calculateBackSubstitution<N, N / 8>(true);
}
