#ifndef MATRIXHPP
#define MATRIXHPP

#include <vector>
#include <functional>
#include <cmath>

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
    std::vector<std::vector<TScalar>> mat;

public:
    Matrix() : mat(std::vector<std::vector<TScalar>>(NRows, std::vector<TScalar>(NCols, 0))){};

    Matrix(TScalar defaultValue) : mat(std::vector<std::vector<TScalar>>(NRows, std::vector<TScalar>(NCols, defaultValue))){};

    Matrix(std::function<TScalar(std::size_t row, std::size_t col)> fun) : mat(std::vector<std::vector<TScalar>>(NRows, std::vector<TScalar>(NCols)))
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
        Matrix<TScalar, NCols, NRows> transpose(0);
        std::vector<std::vector<TScalar>> &transposeMat = transpose.mat;
        for (std::size_t i = 0; i < NRows; ++i)
        {
            for (std::size_t j = 0; j < NCols; ++j)
            {
                transposeMat[j][i] = mat[i][j];
            }
        }
        return transpose;
    };

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
    };

    /**
     * @brief Set an entry of the matrix; there is no bounds checking.
     * 
     * @param i Row index: 0 <= i < NRows
     * @param j Col index: 0 <= j < NCols
     * @param value The value to write
     */
    void set(std::size_t i, std::size_t j, TScalar value)
    {
        mat[i][j] = value;
    };

    /**
     * @brief Add a scalar times another matrix to the current matrix.
     * 
     * @param other Another matrix of the same shape.
     * @param scalar The scalar to multiply.
     */
    void add(const Matrix<TScalar, NRows, NCols> &other, TScalar scalar)
    {
        const std::vector<std::vector<TScalar>> &otherMat = other.mat;
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t j = 0; j != NCols; ++j)
            {
                mat[i][j] += scalar * otherMat[i][j];
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
        const std::vector<std::vector<TScalar>> &otherMat = other.mat;
        std::vector<std::vector<TScalar>> &resultMat = result.mat;
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t k = 0; k != NCols; ++k)
            {
                for (std::size_t j = 0; j != NColsProduct; ++j)
                {
                    resultMat[i][j] += mat[i][k] * otherMat[k][j];
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
        const std::vector<std::vector<TScalar>> &otherMat = other.mat;
        std::vector<std::vector<TScalar>> &resultMat = result.mat;
        for (std::size_t i = 0; i != NRowsProduct; ++i)
        {
            for (std::size_t k = 0; k != NRows; ++k)
            {
                for (std::size_t j = 0; j != NCols; ++j)
                {
                    resultMat[i][j] += otherMat[i][k] * mat[k][j];
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
        Matrix<TScalar, NRowsSubmatrix, NColsSubmatrix> result(0);
        std::vector<std::vector<TScalar>> &resultMat = result.mat;
        for (std::size_t i = 0; i < NRowsSubmatrix; ++i)
        {
            for (std::size_t j = 0; j < NColsSubmatrix; ++j)
            {
                resultMat[i][j] = mat[firstRow + i][firstCol + j];
            }
        }
        return result;
    };

    /**
     * @brief Return a copy of a column of the current matrix. There is no bounds checking.
     * 
     * @param colIdx The column index.
     * @return Matrix<TScalar, NRows, 1> The column.
     */
    Matrix<TScalar, NRows, 1> column(std::size_t colIdx) const
    {
        return submatrix<NRows, 1>(0, colIdx);
    };

    /**
     * @brief Return a submatrix consisting of columns of the current matrix. There is no bounds checking.
     * 
     * @tparam NColsSubmatrix The number of columns to return.
     * @param firstCol The first column of the submatrix.
     * @return Matrix<TScalar, NRows, NColsSubmatrix> The submatrix.
     */
    template <std::size_t NColsSubmatrix>
    Matrix<TScalar, NRows, NColsSubmatrix> columns(std::size_t firstCol)
    {
        return submatrix<NRows, NColsSubmatrix>(0, firstCol);
    };

    /**
     * @brief Overwrite columns of the target matrix with columns of the current matrix. There is no bounds checking.
     * 
     * @tparam NColsSubmatrix The number of columns of the target.
     * @param firstCol The first column index of the current matrix.
     * @param target The target matrix.
     */
    template <std::size_t NColsSubmatrix>
    void columnsInto(std::size_t firstCol, Matrix<TScalar, NRows, NColsSubmatrix> &target) const
    {
        std::vector<std::vector<TScalar>> &targetMat = target.mat;
        for (std::size_t i = 0; i < NRows; ++i)
        {
            for (std::size_t j = 0; j < NColsSubmatrix; ++j)
            {
                targetMat[i][j] = mat[i][firstCol + j];
            }
        }
    };

    /**
     * @brief Overwrite rows of the target matrix with rows of the current matrix. There is no bounds checking.
     * 
     * @tparam NRowsSubmatrix The number of rows of the target.
     * @param firstRow The first row index of the current matrix.
     * @param target The target matrix.
     */
    template <std::size_t NRowsSubmatrix>
    void rowsInto(std::size_t firstRow, Matrix<TScalar, NRowsSubmatrix, NCols> &target) const
    {
        std::vector<std::vector<TScalar>> &targetMat = target.mat;
        for (std::size_t i = 0; i < NRowsSubmatrix; ++i)
        {
            for (std::size_t j = 0; j < NCols; ++j)
            {
                targetMat[i][j] = mat[i + firstRow][j];
            }
        }
    };

    /**
     * @brief Return a copy of a row of the current matrix. There is no bounds checking.
     * 
     * @param colIdx The row index.
     * @return Matrix<TScalar, 1, NCols> The row.
     */
    Matrix<TScalar, 1, NCols> row(std::size_t rowIdx) const
    {
        return submatrix<1, NCols>(rowIdx, 0);
    };

    /**
     * @brief Return a submatrix consisting of rows of the current matrix. There is no bounds checking.
     * 
     * @tparam NRowsSubmatrix The number of rows to return.
     * @param firstCol The first row of the submatrix.
     * @return Matrix<TScalar, NRowsSubmatrix, NCols> The submatrix.
     */
    template <std::size_t NRowsSubmatrix>
    Matrix<TScalar, NRowsSubmatrix, NCols> rows(std::size_t firstRow)
    {
        return submatrix<NRowsSubmatrix, NCols>(firstRow, 0);
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
        const std::vector<std::vector<TScalar>> &otherMat = other.mat;
        for (std::size_t i = 0; i != NRowsSubmatrix; ++i)
        {
            for (std::size_t j = 0; j != NColsSubmatrix; ++j)
            {
                mat[firstRow + i][firstCol + j] = otherMat[i][j];
            }
        }
    };

    /**
     * @brief Add to a submatrix of the current matrix a constant multiple of another matrix. There is no bounds checking.
     * 
     * @tparam NRowsSubmatrix The number of rows of the submatrix.
     * @tparam NColsSubmatrix The number of columns of the submatrix.
     * @param other A matrix containing the new values to add to the submatrix of the current matrix.
     * @param firstRow The index of the first row of the submatrix: we must have (firstRow + NRowsSubmatrix) less than NRows.
     * @param firstCol The index of the first column of the submatrix: we must have (firstCol + NColsSubmatrix) less than NCols.
     * @param scalar A scalar multiple.
     */
    template <std::size_t NRowsSubmatrix, std::size_t NColsSubmatrix>
    void addToSubmatrix(const Matrix<TScalar, NRowsSubmatrix, NColsSubmatrix> &other, std::size_t firstRow, std::size_t firstCol, TScalar scalar)
    {
        const std::vector<std::vector<TScalar>> &otherMat = other.mat;
        for (std::size_t i = 0; i != NRowsSubmatrix; ++i)
        {
            for (std::size_t j = 0; j != NColsSubmatrix; ++j)
            {
                mat[firstRow + i][firstCol + j] += scalar * otherMat[i][j];
            }
        }
    };

    /**
     * @brief Return a quantity proportional to the Frobenius norm (element-wise 2-norm) of the current matrix.
     * 
     * @return TScalar The computed norm.
     */
    TScalar frobNorm() const
    {
        TScalar sum = 0;
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t j = 0; j != NCols; ++j)
            {
                TScalar elem = mat[i][j];
                sum += elem * elem / (NRows * NCols);
            }
        }

        return std::sqrt(std::abs(sum));
    };

    /**
     * @brief Compute the biased sample variance of the entries of the current matrix
     * 
     * @return The sample variance
     */
    TScalar sample_dev() const
    {
        TScalar mean = 0;
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t j = 0; j != NCols; ++j)
            {
                mean += mat[i][j] / (NRows * NCols);
            }
        }

        TScalar variance = 0;
        for (std::size_t i = 0; i != NRows; ++i)
        {
            for (std::size_t j = 0; j != NCols; ++j)
            {
                TScalar diff = mat[i][j] - mean;
                variance += diff * diff / (NRows * NCols);
            }
        }

        return std::sqrt(std::abs(variance));
    };
};

#endif