#include "catch.hpp"

#include "rush/lexer/token.hpp"
#include "rush/lexer/lexer.hpp"

// namespace rtok = rush::tokens;

// void check_valid_lex(char const* text, std::vector<lexical_token> tokens) {
// 	auto lxa = rush::lex(text);
// 	REQUIRE( std::equal(
// 		begin(lxa), end(lxa),
// 		begin(tokens), end(tokens))
// 	);
// }

TEST_CASE( "tokenize identifiers", "[unit][lexer]" ) {
	SECTION( "string with an underscore" ) {
		// identifier underscore head
		// check_valid_lex("_ _a _1 _a1", {
		// 	rtok::identifier("_"),
		// 	rtok::identifier("_a"),
		// 	rtok::identifier("_1"),
		// 	rtok::identifier("_a1")
		// });
	}

	SECTION( "string with a letter" ) {
		// check_valid_lex("a b1 c_ d2_ e_3", {
		// 	rtok::identifier("a"),
		// 	rtok::identifier("b1"),
		// 	rtok::identifier("c_"),
		// 	rtok::identifier("d2_"),
		// 	rtok::identifier("e_3"),
		// 	rtok::identifier("f4_ab12__"),
		// });
	}

	// identifier like-joined
	// check_valid_lex("abc123___ abc___123 ___abc123 ___123abc", {
	// 	rtok::identifier("abc123___"),
	// 	rtok::identifier("abc___123"),
	// 	rtok::identifier("___abc123"),
	// 	rtok::identifier("___123abc"),
	// });

	// identifier interspersed
	// check_valid_lex("a1_b2_c3_ a_1b_2c_3 _a1_b2_c3 _1a_2b_3c", {
	// 	rtok::identifier("a_1b_2c_3"),
	// 	rtok::identifier("_a1_b2_c3"),
	// 	rtok::identifier("_1a_2b_3c"),
	// });
}

TEST_CASE( "tokenize keywords", "[unit][lexer]" ) {
	// check_valid_lex("var let const if for while", {
	// 	rtok::keyword_var(),
	// 	rtok::keyword_let(),
	// 	rtok::keyword_const(),
	// 	rtok::keyword_if(),
	// 	rtok::keyword_for(),
	// 	rtok::keyword_while(),
	// });
}

TEST_CASE( "tokenize punctuation", "[unit][lexer]" ) {

}