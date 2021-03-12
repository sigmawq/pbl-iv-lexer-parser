//
// Created by swql on 2/12/21.
//

#ifndef LEXER_TEST_PARSE_OPERATIONS_H
#define LEXER_TEST_PARSE_OPERATIONS_H

#include "grammar.h"

struct tokenizer_data{
    jump_table keywords;
    jump_table identifiers;
    jump_table integers;
    jump_table floats;
    jump_table comments;
    jump_table whitespace;

    std::vector<std::reference_wrapper<jump_table>> get_in_array_form(){
        return { whitespace, comments, floats, integers, keywords, identifiers };
    }
};

tokenizer_data prepare_tokenizer(){
    tokenizer_data tokenizer_data;

    tokenizer_data.keywords.add_keyword("for", KEYWORD);
    tokenizer_data.keywords.add_keyword("while", KEYWORD);
    tokenizer_data.keywords.add_keyword("if", KEYWORD);
    tokenizer_data.keywords.add_keyword("else", KEYWORD);
    tokenizer_data.keywords.add_keyword("lambda", KEYWORD);
    tokenizer_data.keywords.add_keyword("=", OPERATOR);
    tokenizer_data.keywords.add_keyword("<", OPERATOR);
    tokenizer_data.keywords.add_keyword("<=", OPERATOR);
    tokenizer_data.keywords.add_keyword(">=", OPERATOR);
    tokenizer_data.keywords.add_keyword("==", OPERATOR);
    tokenizer_data.keywords.add_keyword("!=", OPERATOR);
    tokenizer_data.keywords.add_keyword("+", OPERATOR);
    tokenizer_data.keywords.add_keyword("-", OPERATOR);
    tokenizer_data.keywords.add_keyword("*", OPERATOR);
    tokenizer_data.keywords.add_keyword("/", OPERATOR);
    tokenizer_data.keywords.add_keyword("^", OPERATOR);
    tokenizer_data.keywords.add_keyword("(", SPECIAL_CHARACTER);
    tokenizer_data.keywords.add_keyword(")", SPECIAL_CHARACTER);
    tokenizer_data.keywords.add_keyword("foreach", KEYWORD);

    // Table for identifiers
    rule a1 { RULE_TYPE::ANY_OF_SEQUENCE_AND, RULE_QUALIFIER::ONE_TIME, { 'a', 'z', 'A', 'Z' } };
    rule a2 { RULE_TYPE::ANY_OF_SEQUENCE_AND, RULE_QUALIFIER::ZERO_OR_MORE,
              { 'a', 'z', 'A', 'Z', '1', '9' } };
    tokenizer_data.identifiers.add_sequence_of_rules(std::vector{a1, a2}, TOKEN_TYPE::IDENTIFIER);

    // Table for integers
    rule b1 { RULE_TYPE::ANY_OF_SEQUENCE, RULE_QUALIFIER::ONE_TIME, {'1', '9'} };
    rule b2 { RULE_TYPE::ANY_OF_SEQUENCE, RULE_QUALIFIER::ZERO_OR_MORE , {'0', '9'} };
    rule zero_rule { RULE_TYPE::ONE_CHAR, RULE_QUALIFIER::ONE_TIME , {'0'} };

    tokenizer_data.integers.add_sequence_of_rules(std::vector{b1, b2}, TOKEN_TYPE::CONSTANT_INTEGER);
    tokenizer_data.integers.add_sequence_of_rules(std::vector{zero_rule}, TOKEN_TYPE::CONSTANT_INTEGER);

    // Table for floats
    rule c2 { RULE_TYPE::ANY_OF_SEQUENCE, RULE_QUALIFIER::ZERO_OR_MORE , {'0', '9'} };
    rule c3 { RULE_TYPE::ONE_CHAR, RULE_QUALIFIER::ONE_TIME , {'.'} };
    rule c4 { RULE_TYPE::ANY_OF_SEQUENCE, RULE_QUALIFIER::ZERO_OR_MORE , {'0', '9'} };

    tokenizer_data.floats.add_sequence_of_rules(std::vector{c2, c3, c4}, TOKEN_TYPE::CONSTANT_FP);

    // Table for comments
    rule d1 { RULE_TYPE::ONE_CHAR, RULE_QUALIFIER::ONE_TIME , {'/'} };
    rule d2 { RULE_TYPE::ANY_OF_SEQUENCE_AND, RULE_QUALIFIER::ZERO_OR_MORE , {0, 0x0A - 1, 0x0A + 1, INT8_MAX - 1} };
    rule d3 { RULE_TYPE::ONE_CHAR, RULE_QUALIFIER::ONE_TIME , {0x0A} };

    tokenizer_data.comments.add_sequence_of_rules(std::vector{d1, d1, d2, d3}, TOKEN_TYPE::COMMENT_LINE);

    // Table for whitespaces
    rule ws { RULE_TYPE::ONE_CHAR, RULE_QUALIFIER::ZERO_OR_MORE , {' '} };;

    tokenizer_data.whitespace.add_sequence_of_rules(std::vector{ws}, TOKEN_TYPE::PURE_WS);

    return tokenizer_data;
}

// Produdes token stream from source string.
// Comments and whitespaces are ignored
std::optional<std::vector<token>> tokenize(tokenizer_data &tokenizer_data, std::string const& source){
    token_reader token_reader { source, tokenizer_data.get_in_array_form() };
    std::vector<token> token_stream;

    pbl_utility::debug_print_nl("Tokens parsed: ");
    while (!token_reader.string_eos_reached()){
        auto result = token_reader.read_next_token();
        if (!result.has_value()){
            auto pos = token_reader.get_position();
            std::cout << "Unrecognized token at " << pos.first << ", " << pos.second << std::endl;
            return {};
        }
        else{
            if (result.value().type == TOKEN_TYPE::PURE_WS || result.value().type == TOKEN_TYPE::COMMENT_LINE) continue;
            token_stream.push_back(result.value());
            pbl_utility::debug_print_nl(" ", result.value().attribute, " (", std::to_string(result.value().type), ')');
        }
    }
    return {token_stream};
}

std::vector<bound_token> bind_token_to_universe(std::vector<grammar_unit> &universe, std::vector<token> &tok_stream){
    std::vector<bound_token> bound_token_stream;

    std::vector<grammar_unit>::iterator result;
    for (const auto &el : tok_stream) {
        if (el.type == TOKEN_TYPE::IDENTIFIER){
            result = std::find_if(universe.begin(), universe.end(), [&](const grammar_unit &gu) {
                return (gu.is_identifier);
            });
        }
        else if (el.type == TOKEN_TYPE::CONSTANT_FP || el.type == TOKEN_TYPE::CONSTANT_INTEGER){
            result = std::find_if(universe.begin(), universe.end(), [&](const grammar_unit &gu) {
                return (gu.is_number);
            });
        }
        else{
            result = std::find_if(universe.begin(), universe.end(), [&](const grammar_unit &gu) {
                return (gu.string_representation == el.attribute);
            });
        }

        if (result == universe.end()) {
            std::string err;
            pbl_utility::str_compose(err, "Token ", el.attribute, " of type ", el.type, " has not been "
                                                                                        "found in "
                                                                                        "universe set");
            throw std::runtime_error(err);
        }
        else{
            bound_token_stream.push_back({el, &*result});
        }
    }

    return bound_token_stream;
}

struct parse_data{
    std::vector<grammar_unit> universe;
    std::optional<parse_table> pt;
    const grammar_unit *start_symbol;
    std::unordered_map<const grammar_unit *, production_group> productions;
    std::unordered_map<const grammar_unit *, first_set> first_set_val;
    std::unordered_map<const grammar_unit *, follow_set> follow_set_val;
    std::vector<predict_set_record> predict_set_val;
};

void prepare_parse(parse_data &pd){
    std::vector<grammar_unit> &universe = pd.universe;

    // Non terminals
    universe.push_back({false, "STMT"});
    universe.push_back({false, "EXPR"});
    universe.push_back({false, "TERM"});
    universe.push_back({false, "EXPR_P"});
    universe.push_back({false, "TERM_P"});
    universe.push_back({false, "FACTOR"});

    // Terminals
    universe.push_back({true, "+"});
    universe.push_back({true, "-"});
    universe.push_back({true, "*"});
    universe.push_back({true, "/"});

    universe.push_back({true, "<NUMBER>"});
    universe.back().mark_as_number();

    universe.push_back({true, "<IDENTIFIER>"});
    universe.back().mark_as_identifier();


    std::vector<std::pair<std::string, std::vector<std::string>>> productions_raw{
            {"STMT",         {" "}},
            {"STMT",         {"EXPR"}},

            // Arithmetic expr group

            {"EXPR",         {"TERM", "EXPR_P"}},

            {"EXPR_P",         {"+", "TERM", "EXPR_P"}},
            {"EXPR_P",         {" "}},

            {"TERM",          {"FACTOR", "TERM_P"}},

            {"TERM_P",         {"*", "FACTOR", "TERM_P"}},
            {"TERM_P",         {" "}},

            {"FACTOR",         {"<IDENTIFIER>"}},
            {"FACTOR",         {"<NUMBER>"}}
    };

    pd.productions = grammar::parse_productions(universe, productions_raw);

    // Just alias, as for universe above
    auto &productions = pd.productions;

    for (auto &thing : productions){
        thing.second.D_out(thing.first->string_representation);
    }

    using namespace pbl_utility;

    const grammar_unit *start_symbol = grammar::find_gu_by_name(universe, "STMT");


    pd.first_set_val = grammar::get_first_set(universe, productions, start_symbol);
    grammar::D_first_set_out(pd.first_set_val);
    std::cout << std::endl;

    debug_print_nl("Follow set: ");
    pd.follow_set_val = grammar::get_follow_set(universe, productions, start_symbol, pd.first_set_val);
    grammar::D_follow_set_out(pd.follow_set_val);

    debug_print_nl("Predict set: ");
    pd.predict_set_val = grammar::get_predict_set(universe, productions, start_symbol, pd.first_set_val);
    grammar::D_predict_out(pd.predict_set_val);

    pd.pt.emplace(pd.universe, pd.predict_set_val, pd.follow_set_val);
    pd.start_symbol = start_symbol;
}

// Builds parse tree from source string
parse_tree parse_source(tokenizer_data &td, parse_data &pd, std::string &source){
    auto tokens = tokenize(td, source);

    // Error checking
    if (!tokens.has_value()) {
        throw std::runtime_error("Parser stops due to tokenizer failure"); }

    auto bound_token_stream = bind_token_to_universe(pd.universe, tokens.value());
    auto tree = parse_string(bound_token_stream, pd.start_symbol, pd.pt.value());

    return tree;
}

void convert_parse_to_ast(){
    // TODO
}

#endif //LEXER_TEST_PARSE_OPERATIONS_H
