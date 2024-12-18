#include <gtest/gtest.h>

#include "lexer.hh"
#include "parser.hh"
#include <sstream>
#include <memory>

class TestOutput : public ParserOutput
{
public:
    std::vector<Wrapper<Statement>> statements_;
    void interpret(std::vector<Wrapper<Statement>> statements)
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

    const auto *simpleCommand = dynamic_cast<const SimpleCommand *>(output_.statements_.at(0).get());
    ASSERT_TRUE(simpleCommand);

    EXPECT_EQ(simpleCommand->name, "prog");
    ASSERT_EQ(simpleCommand->args.size(), 1);
    EXPECT_EQ(simpleCommand->args.at(0), "arg");
    ASSERT_EQ(simpleCommand->redirections.size(), 2);
    EXPECT_EQ(simpleCommand->redirections.at(0).file, "in");
    EXPECT_EQ(simpleCommand->redirections.at(0).type, RedirectionType::IN);
    EXPECT_EQ(simpleCommand->redirections.at(1).file, "out");
    EXPECT_EQ(simpleCommand->redirections.at(1).type, RedirectionType::OUT);
}

TEST_F(ParserTest, syntax_error)
{
    Init("prog < ;");
    ASSERT_TRUE((*parser_)());
    ASSERT_EQ(output_.statements_.size(), 0);
}

TEST(AstTest, assignment_ctor)
{
    auto assignment = Assignment("var=value");
    EXPECT_EQ(assignment.var, "var");
    EXPECT_EQ(assignment.value, "value");
}

TEST(AstTest, assignment_ctor_empty_value)
{
    auto assignment = Assignment("var=");
    EXPECT_EQ(assignment.var, "var");
    EXPECT_EQ(assignment.value, "");
}

TEST(AstTest, assignment_ctor_empty_variable)
{
    auto assignment = Assignment("var=");
    EXPECT_EQ(assignment.var, "var");
    EXPECT_EQ(assignment.value, "");
}

TEST(AstTest, assignment_ctor_no_equal_sign)
{
    EXPECT_THROW(Assignment("varvalue"), std::runtime_error);
}

TEST(AstTest, assignment_ctor_empty)
{
    EXPECT_THROW(Assignment(""), std::runtime_error);
}
