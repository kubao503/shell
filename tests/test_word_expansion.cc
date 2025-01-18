#include "../interpreter/interpreter.hh"
#include <gtest/gtest.h>

class WordExpansionTest : public testing::Test {

protected:
    static void Init() {
        setenv("a", "x", true);
    }
    void assertExpansionEval(std::string expr, std::string value) {
        interpreter.performWordExpansions(expr);
        ASSERT_EQ(expr, value);
    }
    Interpreter interpreter;
};

TEST_F(WordExpansionTest, basic_dollar_expr) {
    Init();
    assertExpansionEval("$a", "x");
}

TEST_F(WordExpansionTest, double_dollar_expr) {
    Init();
    assertExpansionEval("$a $a", "x x");
    assertExpansionEval("$a$a", "xx");
}

TEST_F(WordExpansionTest, basic_quote) {
    Init();
    assertExpansionEval("\'a\'", "a");
    assertExpansionEval("\'$a\'", "$a");
}

TEST_F(WordExpansionTest, advanced_quote) {
    Init();
    assertExpansionEval("$a\'$a\'", "x$a");
    assertExpansionEval("\'$a\'$a", "$ax");
    assertExpansionEval("$a\'$a\'$a", "x$ax");
}
