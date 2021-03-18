#include "tests.hxx"

#include <iostream>

#include "../../test/test.hxx"
#include "../parsec.hxx"

namespace parsec {
    namespace tests {
        auto print_string(string s) -> void {
            std::cout << s;
        }

        template<typename A, typename B>
        auto print_tuple(std::tuple<A, B> tuple) -> void {
            std::cout << '(' << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ')';
        }

        template<typename A, typename B>
        auto print_option_tuple(const Option<std::tuple<A, B>> &opt) -> void {
            if (opt.has_value())
                print_tuple(*opt);
        }

        template<typename T>
        using owning_parser_result = Option<std::tuple<string, T>>;

        auto test() -> void {
            test::TestGroup tests = {
                    new test::Test<std::monostate, string>(
                            "skip_while_char",
                            std::monostate(),
                            "bbbccc",
                            [](std::monostate) -> string {
                                string s("aaabbbccc");
                                std::string_view tail(s);
                                infallible_parser_result <std::monostate> res = skip_while_char(tail,
                                                                                                [](char c) -> bool {
                                                                                                    return c == 'a';
                                                                                                });
                                tail = std::get<0>(res);
                                return string(tail);
                            },
                            print_string
                    ),
                    new test::Test<std::monostate, std::tuple<string, string>>(
                            "scan_while_char",
                            std::monostate(),
                            std::make_tuple("bbbccc", "aaa"),
                            [](std::monostate) -> std::tuple<string, string> {
                                string s("aaabbbccc");
                                std::string_view tail(s);
                                infallible_parser_result <string> res = scan_while_char(tail,
                                                                                        [](char c) -> bool {
                                                                                            return c == 'a';
                                                                                        });
                                tail = std::get<0>(res);
                                string scanned = std::get<1>(res);

                                return std::make_tuple(string(tail), scanned);
                            },
                            print_tuple<string, string>
                    ),
                    new test::BoolTest(
                            "scan_char",
                            []() -> bool {
                                string s("aaabbbccc");
                                std::string_view tail(s);
                                parser_result<char> res = scan_char(tail);
                                tail = std::string(std::get<0>(*res));
                                char res_char = std::get<1>(*res);
                                return tail == "aabbbccc" && res_char == 'a';
                            }
                    ),
                    new test::BoolTest(
                            "scan_char-fail",
                            []() -> bool {
                                string s("");
                                std::string_view tail(s);
                                parser_result<char> res = scan_char(tail);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "i64",
                            []() -> bool {
                                string s("-100aaa");
                                std::string_view tail(s);
                                if (parser_result < i64 > res = parse_i64(tail)) {
                                    tail = std::get<0>(*res);
                                    int scanned = std::get<1>(*res);
                                    return tail == "aaa" && scanned == -100;
                                }
                                return false;
                            }
                    ),
                    new test::BoolTest(
                            "i64-fail",
                            []() -> bool {
                                string s("aaa");
                                std::string_view tail(s);
                                parser_result <i64> res = parse_i64(tail);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_char",
                            []() -> bool {
                                string s("aabb");
                                std::string_view tail(s);
                                if (parser_result < int > res = consume_prefix_char(tail, 'a', 23)) {
                                    tail = std::get<0>(*res);
                                    int success = std::get<1>(*res);
                                    return tail == "abb" && success == 23;
                                }
                                return false;
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_char-fail",
                            []() -> bool {
                                string s("bb");
                                std::string_view tail(s);
                                parser_result<int> res = consume_prefix_char(tail, 'a', 23);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_char-fail-empty",
                            []() -> bool {
                                string s("bb");
                                std::string_view tail(s);
                                parser_result<int> res = consume_prefix_char(tail, 'a', 23);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_str",
                            []() -> bool {
                                string s("123bb");
                                std::string_view tail(s);
                                if (parser_result < int > res = consume_prefix_str(tail, "123", 23)) {
                                    tail = std::get<0>(*res);
                                    int success = std::get<1>(*res);
                                    return tail == "bb" && success == 23;
                                }
                                return false;
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_str-fail",
                            []() -> bool {
                                string s("456bb");
                                std::string_view tail(s);
                                parser_result<int> res = consume_prefix_str(tail, "123", 23);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_str-fail-empty",
                            []() -> bool {
                                string s("");
                                std::string_view tail(s);
                                parser_result<int> res = consume_prefix_str(tail, "123", 23);
                                return !res.has_value();
                            }
                    ),
            };

            test::log_run_test_group(tests);
        }
    }
}