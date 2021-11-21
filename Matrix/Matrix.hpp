#ifndef MATRIXHPP
#define MATRIXHPP

#include <array>
#include <functional>

/**
 * @brief A numerical matrix class with compile-time shape specified.
 * 
 * Supports addition, subtraction, multiplication, and submatrix extraction.
 * 
 * All indices are zero-based.
 * 
 * @tparam TScalar The scalar type (must support +, -, *)
 * @tparam NRows The number of rows
 * @tparam NCols The number of columns
 */
template <typename TScalar, std::size_t NRows, std::size_t NCols>
class Matrix
{
    template <typename TScalarOther, std::size_t NRowsOther, std::size_t NColsOther>
    friend class Matrix;

private:
    std::array<std::array<TScalar, NCols>, NRows> mat{};

public:
    Matrix(TScalar defaultValue)
    {
        for (auto &row : mat)
        {
            for (auto &entry : row)
            {
                entry = defaultValue;
            }
        }
    };

    Matrix(std::function<TScalar(std::size_t row, std::size_t col)> fun)
    {
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t j = 0; j != NCols; ++j)
            {
                mat[i][j] = fun(i, j);
            }
        }
    };

    Matrix<TScalar, NCols, NRows> transpose() const
    {
        return Matrix<TScalar, NCols, NRows>([this](std::size_t i, std::size_t j)
                                             { return mat[j][i]; });
    }

    void print() const
    {
        for (auto &row : mat)
        {
            for (auto &entry : row)
            {
                std::cout << entry << " ";
            }
            std::cout << std::endl;
        }
    };

    /**
     * @brief Get an entry of the matrix; there is no bounds checking.
     * 
     * @param i Row index: 0 <= i < NRows
     * @param j Col index: 0 <= j < NCols
     */
    TScalar get(std::size_t i, std::size_t j) const
    {
        return mat[i][j];
    }

    /**
     * @brief Add a scalar times another matrix to the current matrix.
     * 
     * @param other Another matrix of the same shape.
     * @param scalar The scalar to multiply.
     */
    void add(const Matrix<TScalar, NRows, NCols> &other, TScalar scalar)
    {
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t j = 0; j != NCols; ++j)
            {
                mat[i][j] += scalar * other.mat[i][j];
            }
        }
    };

    /**
     * @brief Multiply the current matrix by a given scalar.
     * 
     * @param scalar The scalar by which to multiply.
     */
    void multiplyScalar(TScalar scalar)
    {
        for (auto &row : mat)
        {
            for (auto &entry : row)
            {
                entry *= scalar;
            }
        }
    };

    /**
     * @brief Multiply the current matrix from the right and add the product to the result matrix; the current matrix is unchanged.
     *
     * @tparam NColsProduct The number of columns of the product matrix.
     * @param other The other matrix
     * @param result The result matrix
     */
    template <std::size_t NColsProduct>
    void multiplyRight(const Matrix<TScalar, NCols, NColsProduct> &other, Matrix<TScalar, NRows, NColsProduct> &result) const
    {
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t k = 0; k != NCols; ++k)
            {
                for (std::size_t j = 0; j != NColsProduct; ++j)
                {
                    result.mat[i][j] += mat[i][k] * other.mat[k][j];
                }
            }
        }
    };

    /**
     * @brief Multiply the current matrix from the left and add the product to the result matrix; the current matrix is unchanged.
     *
     * @tparam NRowsProduct The number of rows of the product matrix.
     * @param other The other matrix
     * @param result The result matrix
     */
    template <std::size_t NRowsProduct>
    void multiplyLeft(const Matrix<TScalar, NRowsProduct, NRows> &other, Matrix<TScalar, NRowsProduct, NCols> &result) const
    {
        for (std::size_t i = 0; i != NRowsProduct; ++i)
        {
            for (std::size_t k = 0; k != NRows; ++k)
            {
                for (std::size_t j = 0; j != NCols; ++j)
                {
                    result.mat[i][j] += other.mat[i][k] * mat[k][j];
                }
            }
        }
    };

    /**
     * @brief Return a copy of a submatrix of the current matrix. There is no bounds checking.
     * 
     * @tparam NRowsSubmatrix The number of rows of the submatrix.
     * @tparam NColsSubmatrix The number of columns of the submatrix.
     * @param firstRow The index of the first row of the submatrix: we must have (firstRow + NRowsSubmatrix) less than NRows.
     * @param firstCol The index of the first column of the submatrix: we must have (firstCol + NColsSubmatrix) less than NCols.
     * @return Matrix<TScalar, NRowsSubmatrix, NColsSubmatrix> A copy of the submatrix.
     */
    template <std::size_t NRowsSubmatrix, std::size_t NColsSubmatrix>
    Matrix<TScalar, NRowsSubmatrix, NColsSubmatrix> submatrix(std::size_t firstRow, std::size_t firstCol) const
    {
        return Matrix<TScalar, NRowsSubmatrix, NColsSubmatrix>([this, firstRow, firstCol](std::size_t i, std::size_t j)
                                                               { return mat[firstRow + i][firstCol + j]; });
    };

    /**
     * @brief Overwrite a submatrix of the current matrix with another matrix. There is no bounds checking.
     * 
     * @tparam NRowsSubmatrix The number of rows of the submatrix.
     * @tparam NColsSubmatrix The number of columns of the submatrix.
     * @param other A matrix containing the new values to write to the submatrix of the current matrix.
     * @param firstRow The index of the first row of the submatrix: we must have (firstRow + NRowsSubmatrix) less than NRows.
     * @param firstCol The index of the first column of the submatrix: we must have (firstCol + NColsSubmatrix) less than NCols.
     */
    template <std::size_t NRowsSubmatrix, std::size_t NColsSubmatrix>
    void overwriteSubmatrix(const Matrix<TScalar, NRowsSubmatrix, NColsSubmatrix> &other, std::size_t firstRow, std::size_t firstCol)
    {
        for (std::size_t i = 0; i != NRowsSubmatrix; ++i)
        {
            for (std::size_t j = 0; j != NColsSubmatrix; ++j)
            {
                mat[firstRow + i][firstCol + j] = other.mat[i][j];
            }
        }
    };
};

#endif