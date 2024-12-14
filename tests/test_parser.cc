#include <gtest/gtest.h>

#include "lexer.hh"
#include "parser.hh"
#include <sstream>
#include <memory>

class TestOutput : public ParserOutput
{
public:
    std::vector<Statement *> statements_;
    void interpret(std::vector<Statement *> statements)
    {
        statements_.insert(statements_.end(), statements.begin(), statements.end());
    }
};

class ParserTest : public testing::Test
{
protected:
    void Init(const std::string &input)
    {
        stream_ = std::istringstream(input);
        lexer_ = std::make_unique<Lexer>(&stream_);
        parser_ = std::make_unique<yy::parser>(*lexer_, output_);
    }
    std::istringstream stream_;
    std::unique_ptr<Lexer> lexer_;
    TestOutput output_;
    std::unique_ptr<yy::parser> parser_;
};

TEST_F(ParserTest, parse_success)
{
    Init("prog arg <in >out\n");
    ASSERT_FALSE((*parser_)());
    ASSERT_EQ(output_.statements_.size(), 1);
}

TEST_F(ParserTest, syntax_error)
{
    Init("prog < ;");
    ASSERT_TRUE((*parser_)());
    ASSERT_EQ(output_.statements_.size(), 0);
}
