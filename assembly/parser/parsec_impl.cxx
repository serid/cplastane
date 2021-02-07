#include "../../parsec/parsec_template.hxx"

#include "parse.hxx"

namespace parsec {
    template auto
    consume_prefix_char(std::string_view s, char prefix, std::monostate on_success) -> parser_result<std::monostate>;

    template auto consume_prefix_str(std::string_view s, std::string_view prefix,
                                     assembly::mnemo_t::arg_t::reg_t on_success) -> parser_result<assembly::mnemo_t::arg_t::reg_t>;

    template auto choice_combinator(std::string_view s,
                                    vector<parser_type<assembly::mnemo_t::arg_t::reg_t, std::monostate>> funs) -> parser_result<assembly::mnemo_t::arg_t::reg_t>;
}