#include <doctest.h>
#include <selforg/matrix/matrix.h>
#include <selforg/utils/stl_adds.h>

using namespace matrix;

TEST_CASE("Matrix basic functionality") {
    Matrix m(2, 3);
    
    SUBCASE("Matrix dimensions") {
        CHECK(m.getM() == 2);
        CHECK(m.getN() == 3);
    }
    
    SUBCASE("Matrix initialization") {
        m.toZero();
        for(unsigned int i = 0; i < m.getM(); i++) {
            for(unsigned int j = 0; j < m.getN(); j++) {
                CHECK(m.val(i, j) == 0.0);
            }
        }
    }
    
    SUBCASE("Matrix identity") {
        Matrix id(3, 3);
        id.toId();
        CHECK(id.getM() == 3);
        CHECK(id.getN() == 3);
        for(unsigned int i = 0; i < 3; i++) {
            for(unsigned int j = 0; j < 3; j++) {
                if(i == j) {
                    CHECK(id.val(i, j) == 1.0);
                } else {
                    CHECK(id.val(i, j) == 0.0);
                }
            }
        }
    }
}

TEST_CASE("STL utilities") {
    SUBCASE("String conversions") {
        std::string result = std::itos(42);
        CHECK(result == "42");
        
        std::string dresult = std::ftos(3.14159);
        CHECK(dresult.length() > 0); // Just check it's not empty
    }
}