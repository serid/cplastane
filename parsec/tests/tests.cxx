#include "tests.hxx"

#include <iostream>

#include "../../test/test.hxx"
#include "../parsec.hxx"

using namespace std;

namespace parsec {
    namespace tests {
        auto print_string(string s) -> void {
            cout << s;
        }

        template<typename A, typename B>
        auto print_tuple(tuple<A, B> tuple) -> void {
            cout << '(' << get<0>(tuple) << ", " << get<1>(tuple) << ')';
        }

        template<typename A, typename B>
        auto print_option_tuple(const Option<tuple<A, B>> &opt) -> void {
            if (opt.has_value())
                print_tuple(*opt);
        }

        template<typename T>
        using owning_parser_result = Option<tuple<string, T>>;

        auto test() -> void {
            test::TestGroup tests = {
                    new test::Test<monostate, string>(
                            "skip_while_char",
                            monostate(),
                            "bbbccc",
                            [](monostate) -> string {
                                string s("aaabbbccc");
                                string_view tail(s);
                                infallible_parser_result <monostate> res = skip_while_char(tail,
                                                                                                [](char c) -> bool {
                                                                                                    return c == 'a';
                                                                                                });
                                tail = get<0>(res);
                                return string(tail);
                            },
                            print_string
                    ),
                    new test::Test<monostate, tuple<string, string>>(
                            "scan_while_char",
                            monostate(),
                            make_tuple("bbbccc", "aaa"),
                            [](monostate) -> tuple<string, string> {
                                string s("aaabbbccc");
                                string_view tail(s);
                                infallible_parser_result <string> res = scan_while_char(tail,
                                                                                        [](char c) -> bool {
                                                                                            return c == 'a';
                                                                                        });
                                tail = get<0>(res);
                                string scanned = get<1>(res);

                                return make_tuple(string(tail), scanned);
                            },
                            print_tuple<string, string>
                    ),
                    new test::BoolTest(
                            "scan_char",
                            []() -> bool {
                                string s("aaabbbccc");
                                string_view tail(s);
                                parser_result<char> res = scan_char(tail);
                                tail = string(get<0>(*res));
                                char res_char = get<1>(*res);
                                return tail == "aabbbccc" && res_char == 'a';
                            }
                    ),
                    new test::BoolTest(
                            "scan_char-fail",
                            []() -> bool {
                                string s("");
                                string_view tail(s);
                                parser_result<char> res = scan_char(tail);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "i64",
                            []() -> bool {
                                string s("-100aaa");
                                string_view tail(s);
                                if (parser_result < i64 > res = parse_i64(tail)) {
                                    tail = get<0>(*res);
                                    int scanned = get<1>(*res);
                                    return tail == "aaa" && scanned == -100;
                                }
                                return false;
                            }
                    ),
                    new test::BoolTest(
                            "i64-fail",
                            []() -> bool {
                                string s("aaa");
                                string_view tail(s);
                                parser_result <i64> res = parse_i64(tail);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_char",
                            []() -> bool {
                                string s("aabb");
                                string_view tail(s);
                                if (parser_result < int > res = consume_prefix_char(tail, 'a', 23)) {
                                    tail = get<0>(*res);
                                    int success = get<1>(*res);
                                    return tail == "abb" && success == 23;
                                }
                                return false;
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_char-fail",
                            []() -> bool {
                                string s("bb");
                                string_view tail(s);
                                parser_result<int> res = consume_prefix_char(tail, 'a', 23);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_char-fail-empty",
                            []() -> bool {
                                string s("bb");
                                string_view tail(s);
                                parser_result<int> res = consume_prefix_char(tail, 'a', 23);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_str",
                            []() -> bool {
                                string s("123bb");
                                string_view tail(s);
                                if (parser_result < int > res = consume_prefix_str(tail, "123", 23)) {
                                    tail = get<0>(*res);
                                    int success = get<1>(*res);
                                    return tail == "bb" && success == 23;
                                }
                                return false;
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_str-fail",
                            []() -> bool {
                                string s("456bb");
                                string_view tail(s);
                                parser_result<int> res = consume_prefix_str(tail, "123", 23);
                                return !res.has_value();
                            }
                    ),
                    new test::BoolTest(
                            "consume_prefix_str-fail-empty",
                            []() -> bool {
                                string s("");
                                string_view tail(s);
                                parser_result<int> res = consume_prefix_str(tail, "123", 23);
                                return !res.has_value();
                            }
                    ),
            };

            test::log_run_test_group(tests);
        }
    }
}