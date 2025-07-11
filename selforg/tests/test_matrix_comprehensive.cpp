/**
 * Comprehensive unit tests for Matrix class
 * Demonstrates proper testing practices for LPZRobots
 */

#include <gtest/gtest.h>
#include <selforg/matrix.h>
#include <cmath>
#include <limits>

using namespace matrix;

class MatrixTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests
        m2x2 = Matrix(2, 2);
        m3x3 = Matrix(3, 3);
        m2x3 = Matrix(2, 3);
        
        // Initialize with known values
        double data2x2[] = {1.0, 2.0, 3.0, 4.0};
        m2x2.set(2, 2, data2x2);
        
        // Identity matrix
        m3x3.toId();
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
    
    // Helper function to check matrix equality with tolerance
    bool matricesEqual(const Matrix& a, const Matrix& b, double tolerance = 1e-10) {
        if (a.getM() != b.getM() || a.getN() != b.getN()) {
            return false;
        }
        
        for (int i = 0; i < a.getM(); ++i) {
            for (int j = 0; j < a.getN(); ++j) {
                if (std::abs(a.val(i, j) - b.val(i, j)) > tolerance) {
                    return false;
                }
            }
        }
        return true;
    }
    
    Matrix m2x2, m3x3, m2x3;
};

// Test basic construction and dimensions
TEST_F(MatrixTest, ConstructionAndDimensions) {
    Matrix m1(5, 7);
    EXPECT_EQ(m1.getM(), 5);
    EXPECT_EQ(m1.getN(), 7);
    
    // Default construction
    Matrix m2;
    EXPECT_EQ(m2.getM(), 0);
    EXPECT_EQ(m2.getN(), 0);
}

// Test element access and modification
TEST_F(MatrixTest, ElementAccess) {
    EXPECT_DOUBLE_EQ(m2x2.val(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(m2x2.val(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(m2x2.val(1, 0), 3.0);
    EXPECT_DOUBLE_EQ(m2x2.val(1, 1), 4.0);
    
    // Modify element
    m2x2.val(1, 1) = 5.0;
    EXPECT_DOUBLE_EQ(m2x2.val(1, 1), 5.0);
    
    // Test bounds checking in debug mode
#ifdef DEBUG
    EXPECT_DEATH(m2x2.val(2, 0), ".*");  // Out of bounds
#endif
}

// Test matrix addition
TEST_F(MatrixTest, Addition) {
    Matrix a(2, 2);
    a.set(2, 2, new double[4]{1, 2, 3, 4});
    
    Matrix b(2, 2);
    b.set(2, 2, new double[4]{5, 6, 7, 8});
    
    Matrix c = a + b;
    
    EXPECT_DOUBLE_EQ(c.val(0, 0), 6.0);
    EXPECT_DOUBLE_EQ(c.val(0, 1), 8.0);
    EXPECT_DOUBLE_EQ(c.val(1, 0), 10.0);
    EXPECT_DOUBLE_EQ(c.val(1, 1), 12.0);
    
    // Test dimension mismatch
    Matrix d(3, 2);
    EXPECT_THROW(a + d, std::runtime_error);
}

// Test matrix multiplication
TEST_F(MatrixTest, Multiplication) {
    Matrix a(2, 3);
    double dataA[] = {1, 2, 3, 4, 5, 6};
    a.set(2, 3, dataA);
    
    Matrix b(3, 2);
    double dataB[] = {7, 8, 9, 10, 11, 12};
    b.set(3, 2, dataB);
    
    Matrix c = a * b;
    
    EXPECT_EQ(c.getM(), 2);
    EXPECT_EQ(c.getN(), 2);
    
    // Expected result:
    // [1*7 + 2*9 + 3*11, 1*8 + 2*10 + 3*12] = [58, 64]
    // [4*7 + 5*9 + 6*11, 4*8 + 5*10 + 6*12] = [139, 154]
    EXPECT_DOUBLE_EQ(c.val(0, 0), 58.0);
    EXPECT_DOUBLE_EQ(c.val(0, 1), 64.0);
    EXPECT_DOUBLE_EQ(c.val(1, 0), 139.0);
    EXPECT_DOUBLE_EQ(c.val(1, 1), 154.0);
    
    // Test dimension mismatch
    Matrix d(2, 2);
    EXPECT_THROW(a * d, std::runtime_error);
}

// Test transpose
TEST_F(MatrixTest, Transpose) {
    Matrix a(2, 3);
    double data[] = {1, 2, 3, 4, 5, 6};
    a.set(2, 3, data);
    
    Matrix aT = a.T();
    
    EXPECT_EQ(aT.getM(), 3);
    EXPECT_EQ(aT.getN(), 2);
    
    EXPECT_DOUBLE_EQ(aT.val(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(aT.val(1, 0), 2.0);
    EXPECT_DOUBLE_EQ(aT.val(2, 0), 3.0);
    EXPECT_DOUBLE_EQ(aT.val(0, 1), 4.0);
    EXPECT_DOUBLE_EQ(aT.val(1, 1), 5.0);
    EXPECT_DOUBLE_EQ(aT.val(2, 1), 6.0);
}

// Test identity matrix
TEST_F(MatrixTest, IdentityMatrix) {
    EXPECT_DOUBLE_EQ(m3x3.val(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(m3x3.val(1, 1), 1.0);
    EXPECT_DOUBLE_EQ(m3x3.val(2, 2), 1.0);
    EXPECT_DOUBLE_EQ(m3x3.val(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(m3x3.val(1, 0), 0.0);
    
    // Test multiplication with identity
    Matrix a(3, 3);
    a.set(3, 3, new double[9]{1, 2, 3, 4, 5, 6, 7, 8, 9});
    
    Matrix result = a * m3x3;
    EXPECT_TRUE(matricesEqual(result, a));
}

// Test matrix inversion
TEST_F(MatrixTest, Inversion) {
    // Test 2x2 inversion
    Matrix inv = m2x2.pseudoInverse();
    Matrix shouldBeIdentity = m2x2 * inv;
    
    // Check if close to identity (within numerical tolerance)
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            double expected = (i == j) ? 1.0 : 0.0;
            EXPECT_NEAR(shouldBeIdentity.val(i, j), expected, 1e-10);
        }
    }
    
    // Test singular matrix
    Matrix singular(2, 2);
    double singularData[] = {1, 2, 2, 4};  // Rows are linearly dependent
    singular.set(2, 2, singularData);
    
    Matrix singularInv = singular.pseudoInverse();
    // Pseudo-inverse should exist even for singular matrices
    EXPECT_FALSE(singularInv.hasNaN());
}

// Test eigenvalue computation
TEST_F(MatrixTest, Eigenvalues) {
    // Test symmetric matrix eigenvalues
    Matrix symmetric(2, 2);
    double symData[] = {4, 1, 1, 3};
    symmetric.set(2, 2, symData);
    
    Matrix eigenvals = symmetric.eigenValues();
    
    // For this matrix, eigenvalues should be 5 and 2
    double lambda1 = std::max(eigenvals.val(0, 0), eigenvals.val(1, 0));
    double lambda2 = std::min(eigenvals.val(0, 0), eigenvals.val(1, 0));
    
    EXPECT_NEAR(lambda1, 5.0, 1e-10);
    EXPECT_NEAR(lambda2, 2.0, 1e-10);
}

// Test numerical stability checks
TEST_F(MatrixTest, NumericalStability) {
    Matrix unstable(2, 2);
    
    // Test NaN detection
    unstable.val(0, 0) = std::numeric_limits<double>::quiet_NaN();
    EXPECT_TRUE(unstable.hasNaN());
    EXPECT_FALSE(unstable.isNormal());
    
    // Test infinity detection
    unstable.val(0, 0) = std::numeric_limits<double>::infinity();
    EXPECT_FALSE(unstable.hasNaN());
    EXPECT_FALSE(unstable.isNormal());
    
    // Test normal matrix
    unstable.val(0, 0) = 1.0;
    EXPECT_TRUE(unstable.isNormal());
}

// Test matrix norms
TEST_F(MatrixTest, MatrixNorms) {
    Matrix a(2, 2);
    double data[] = {3, 0, 0, 4};
    a.set(2, 2, data);
    
    // Frobenius norm should be sqrt(3^2 + 4^2) = 5
    double norm = a.norm();
    EXPECT_NEAR(norm, 5.0, 1e-10);
    
    // Test max norm
    double maxNorm = a.max();
    EXPECT_DOUBLE_EQ(maxNorm, 4.0);
}

// Test map function
TEST_F(MatrixTest, MapFunction) {
    Matrix a(2, 2);
    double data[] = {1, 4, 9, 16};
    a.set(2, 2, data);
    
    // Apply sqrt to all elements
    Matrix b = a.map(sqrt);
    
    EXPECT_DOUBLE_EQ(b.val(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(b.val(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(b.val(1, 0), 3.0);
    EXPECT_DOUBLE_EQ(b.val(1, 1), 4.0);
    
    // Test tanh mapping (common in neural networks)
    Matrix c = a.map(tanh);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_DOUBLE_EQ(c.val(i, j), tanh(a.val(i, j)));
        }
    }
}

// Test buffer conversion (important for robot control)
TEST_F(MatrixTest, BufferConversion) {
    Matrix a(3, 1);
    double data[] = {1.5, 2.5, 3.5};
    a.set(3, 1, data);
    
    // Convert to buffer
    double buffer[3];
    a.convertToBuffer(buffer, 3);
    
    EXPECT_DOUBLE_EQ(buffer[0], 1.5);
    EXPECT_DOUBLE_EQ(buffer[1], 2.5);
    EXPECT_DOUBLE_EQ(buffer[2], 3.5);
}

// Test performance-critical operations
TEST_F(MatrixTest, PerformanceOperations) {
    // Test in-place operations
    Matrix a(100, 100);
    a.toId();
    
    // Scalar multiplication
    a *= 2.0;
    EXPECT_DOUBLE_EQ(a.val(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(a.val(50, 50), 2.0);
    EXPECT_DOUBLE_EQ(a.val(0, 1), 0.0);
    
    // In-place addition
    Matrix b(100, 100);
    b.set(1.0);  // All elements to 1.0
    
    a += b;
    EXPECT_DOUBLE_EQ(a.val(0, 0), 3.0);
    EXPECT_DOUBLE_EQ(a.val(50, 50), 3.0);
    EXPECT_DOUBLE_EQ(a.val(0, 1), 1.0);
}

// Test edge cases
TEST_F(MatrixTest, EdgeCases) {
    // Empty matrix
    Matrix empty;
    EXPECT_EQ(empty.getM(), 0);
    EXPECT_EQ(empty.getN(), 0);
    
    // Single element matrix
    Matrix single(1, 1);
    single.val(0, 0) = 42.0;
    
    Matrix singleInv = single.pseudoInverse();
    EXPECT_NEAR(singleInv.val(0, 0), 1.0/42.0, 1e-10);
    
    // Row vector
    Matrix row(1, 5);
    EXPECT_EQ(row.getM(), 1);
    EXPECT_EQ(row.getN(), 5);
    
    // Column vector
    Matrix col(5, 1);
    EXPECT_EQ(col.getM(), 5);
    EXPECT_EQ(col.getN(), 1);
}

// Test controller-specific operations
TEST_F(MatrixTest, ControllerOperations) {
    // Simulate sensor to motor mapping
    Matrix sensors(3, 1);
    double sensorData[] = {0.5, -0.3, 0.8};
    sensors.set(3, 1, sensorData);
    
    // Controller matrix
    Matrix C(2, 3);
    double controllerData[] = {1.0, 0.5, 0.0, 
                              0.0, 1.0, 0.5};
    C.set(2, 3, controllerData);
    
    // Motor output
    Matrix motors = C * sensors;
    
    EXPECT_EQ(motors.getM(), 2);
    EXPECT_EQ(motors.getN(), 1);
    
    // Check motor values
    EXPECT_DOUBLE_EQ(motors.val(0, 0), 0.5 * 1.0 + (-0.3) * 0.5 + 0.8 * 0.0);
    EXPECT_DOUBLE_EQ(motors.val(1, 0), 0.5 * 0.0 + (-0.3) * 1.0 + 0.8 * 0.5);
    
    // Apply activation function (tanh)
    Matrix activated = motors.map(tanh);
    for (int i = 0; i < 2; ++i) {
        EXPECT_DOUBLE_EQ(activated.val(i, 0), tanh(motors.val(i, 0)));
    }
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}