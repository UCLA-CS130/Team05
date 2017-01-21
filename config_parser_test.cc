#include "gtest/gtest.h"
#include "config_parser.h"



class NginxConfigParserStringTest : public ::testing::Test {
protected:
    bool ParseString(const std::string config_string) {
        std::stringstream config_stream(config_string);
 	return parser_.Parse(&config_stream, &out_config_);
    }

    NginxConfigParser parser_;
    NginxConfig out_config_;
 };


TEST(NginxConfigParserTest, SimpleConfigFile) {
    NginxConfigParser parser;
    NginxConfig out_config;
    
    EXPECT_TRUE(parser.Parse("example_config", &out_config)) << "Example config file can be parsed";
    EXPECT_FALSE(parser.Parse("non_existent_config", &out_config)) << "Nonexistent config file cannot be parsed";
}


TEST(NginxConfigStatementTest, ToString) {
    NginxConfigStatement statement;
    
    statement.tokens_.push_back("foo");
    statement.tokens_.push_back("bar");
    
    EXPECT_EQ("foo bar;\n", statement.ToString(0));
}


TEST_F(NginxConfigParserStringTest, SimpleStatementConfig) {	
    EXPECT_TRUE(ParseString("foo bar;"));
    EXPECT_EQ(1, out_config_.statements_.size()) << "Config has only one statement.";
    EXPECT_EQ("foo", out_config_.statements_[0]->tokens_[0]) << "foo is the first token.";	
}


TEST_F(NginxConfigParserStringTest, SimpleInvalidStatementsConfig) {
    EXPECT_FALSE(ParseString("foo bar")) << "Config missing semicolon.";
    EXPECT_FALSE(ParseString("foo bar;;")) << "Config with repeated semicolons.";
    EXPECT_FALSE(ParseString("foo bar {}")) << "Config missing child block statement.";
    EXPECT_FALSE(ParseString("foo 'bar;")) << "Config missing closed single quote.";
    EXPECT_FALSE(ParseString("foo \"bar;")) << "Config missing closed double quote.";
    EXPECT_FALSE(ParseString("foo 'bar\";")) << "Config with improper matching quote.";
    EXPECT_FALSE(ParseString("")) << "Empty string config.";
    EXPECT_FALSE(ParseString(" ")) << "White space config.";
    EXPECT_FALSE(ParseString("{")) << "Config with only opening curly brace.";
    EXPECT_FALSE(ParseString("}")) << "Config with only ending curly brace.";
    EXPECT_FALSE(ParseString("{}")) << "Config with matching curly braces but no child_block statements.";
}


TEST_F(NginxConfigParserStringTest, MultipleStatementsConfig) {
    EXPECT_TRUE(ParseString("foo bar {foo bar; } foobar;"));
    EXPECT_EQ(2, out_config_.statements_.size()) << "Config with two statements: one with child block and one normal.";
    EXPECT_EQ("foo bar {\n  foo bar;\n}\n", out_config_.statements_[0]->ToString(0));
}


TEST_F(NginxConfigParserStringTest, InnerStatementsConfig) {
    EXPECT_TRUE(ParseString("foo bar {foo barr; bar foo;} foobar;"));
    EXPECT_EQ(2, out_config_.statements_.size());
    EXPECT_EQ(2, out_config_.statements_[0]->child_block_->statements_.size()) << "Child block has two statements.";
    EXPECT_EQ("bar foo;\n", out_config_.statements_[0]->child_block_->statements_[1]->ToString(0));
    EXPECT_EQ("foo barr;\n", out_config_.statements_[0]->child_block_->statements_[0]->ToString(0));
}


TEST_F(NginxConfigParserStringTest, CurlyConfigs) {
    EXPECT_TRUE(ParseString("foo bar {foo bar; foo bar;}"));
    EXPECT_EQ(1, out_config_.statements_.size());
}


TEST_F(NginxConfigParserStringTest, EmbedCurlyConfigs) {
    EXPECT_TRUE(ParseString("foo bar { foo bar {fooo bar;} }"));
    EXPECT_EQ(1, out_config_.statements_.size());
    EXPECT_EQ(1, out_config_.statements_[0]->child_block_->statements_.size());
    EXPECT_EQ(1, out_config_.statements_[0]->child_block_->statements_[0]->child_block_->statements_.size());
}


TEST_F(NginxConfigParserStringTest, UnbalancedCurlyConfigs) {
    EXPECT_FALSE(ParseString("foo bar {foo bar; ")) << "Missing closing curly brace.";
    EXPECT_FALSE(ParseString("foo bar {foo bar {foo bar; }"));
    EXPECT_FALSE(ParseString("foo bar foo bar; } } }"));
}
