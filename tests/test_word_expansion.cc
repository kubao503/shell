#include "../interpreter/interpreter.hh"
#include <gtest/gtest.h>

class WordExpansionTest : public testing::Test {

protected:
    WordExpansionTest() {
        setenv("a", "x", true);
    }
    void assertExpansionEval(const std::string &expr, const std::string &expected) {
        const auto transformedExpr = interpreter.performWordExpansions(expr);
        EXPECT_EQ(transformedExpr, expected);
    }
    Interpreter interpreter;
};

TEST_F(WordExpansionTest, basic_dollar_expr) {
    assertExpansionEval("$a", "x");
}

TEST_F(WordExpansionTest, double_dollar_expr) {
    assertExpansionEval("$a $a", "x x");
    assertExpansionEval("$a$a", "xx");
}

TEST_F(WordExpansionTest, basic_quote) {
    assertExpansionEval(R"('a')", "a");
    assertExpansionEval(R"('$a')", "$a");
}

TEST_F(WordExpansionTest, advanced_quote) {
    assertExpansionEval(R"($a'$a')", "x$a");
    assertExpansionEval(R"('$a'$a)", "$ax");
    assertExpansionEval(R"($a'$a'$a)", "x$ax");
}

TEST_F(WordExpansionTest, unmatched_quote) {
    EXPECT_THROW(interpreter.performWordExpansions(R"('$a)"), std::runtime_error);
}
