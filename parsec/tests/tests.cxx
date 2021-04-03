#include "tests.hxx"

#include <iostream>

#include "../../test/test.hxx"
#include "../parsec.hxx"

using namespace std;

namespace parsec::tests {
    auto print_string(const string &s) -> void {
        cout << s;
    }

    template<typename A, typename B>
    auto print_tuple(const tuple<A, B> &tuple) -> void {
        cout << '(' << get<0>(tuple) << ", " << get<1>(tuple) << ')';
    }

    template<typename A, typename B>
    auto print_option_tuple(const Option<tuple<A, B>> &opt) -> void {
        if (opt.has_value())
            print_tuple(*opt);
    }

    template<typename T>
    using owning_OptionParserResult = Option<tuple<string, T>>;

    auto test() -> void {
        test::TestGroup tests = {
                new test::Test<monostate, string>(
                        "skip_while_char",
                        monostate(),
                        "bbbccc",
                        [](monostate) -> string {
                            string s("aaabbbccc");
                            strive tail(s);
                            ParserResult <monostate> res = skip_while_char(tail,
                                                                           [](char c) -> bool {
                                                                               return c == 'a';
                                                                           });
                            tail = res.tail;
                            return tail.to_string();
                        },
                        print_string
                ),
                new test::Test<monostate, tuple<string, string>>(
                        "scan_while_char",
                        monostate(),
                        make_tuple("bbbccc", "aaa"),
                        [](monostate) -> tuple<string, string> {
                            string s("aaabbbccc");
                            strive tail(s);
                            ParserResult <string> res = scan_while_char(tail,
                                                                        [](char c) -> bool {
                                                                            return c == 'a';
                                                                        });
                            tail = res.tail;
                            string scanned = res.data;

                            return make_tuple(tail.to_string(), scanned);
                        },
                        print_tuple<string, string>
                ),
                new test::BoolTest(
                        "scan_char",
                        []() -> bool {
                            string s("aaabbbccc");
                            strive tail(s);
                            OptionParserResult<char> res = scan_char(tail);
                            tail = res.value().tail.to_string();
                            char res_char = res.value().data;
                            return tail == "aabbbccc" && res_char == 'a';
                        }
                ),
                new test::BoolTest(
                        "scan_char-fail",
                        []() -> bool {
                            string s("");
                            strive tail(s);
                            OptionParserResult<char> res = scan_char(tail);
                            return !res.has_value();
                        }
                ),
                new test::BoolTest(
                        "i64",
                        []() -> bool {
                            string s("-100aaa");
                            strive tail(s);
                            if (OptionParserResult < i64 > res = parse_i64(tail)) {
                                tail = res.value().tail;
                                int scanned = res.value().data;
                                return tail == "aaa" && scanned == -100;
                            }
                            return false;
                        }
                ),
                new test::BoolTest(
                        "i64-fail",
                        []() -> bool {
                            string s("aaa");
                            strive tail(s);
                            OptionParserResult <i64> res = parse_i64(tail);
                            return !res.has_value();
                        }
                ),
                new test::BoolTest(
                        "consume_prefix_char",
                        []() -> bool {
                            string s("aabb");
                            strive tail(s);
                            if (OptionParserResult < int > res = consume_prefix_char(tail, 'a', 23)) {
                                tail = res.value().tail;
                                int success = res.value().data;
                                return tail == "abb" && success == 23;
                            }
                            return false;
                        }
                ),
                new test::BoolTest(
                        "consume_prefix_char-fail",
                        []() -> bool {
                            string s("bb");
                            strive tail(s);
                            OptionParserResult<int> res = consume_prefix_char(tail, 'a', 23);
                            return !res.has_value();
                        }
                ),
                new test::BoolTest(
                        "consume_prefix_char-fail-empty",
                        []() -> bool {
                            string s("bb");
                            strive tail(s);
                            OptionParserResult<int> res = consume_prefix_char(tail, 'a', 23);
                            return !res.has_value();
                        }
                ),
                new test::BoolTest(
                        "consume_prefix_str",
                        []() -> bool {
                            string s("123bb");
                            strive tail(s);
                            if (OptionParserResult < int > res = consume_prefix_str(tail, "123", 23)) {
                                tail = res.value().tail;
                                int success = res.value().data;
                                return tail == "bb" && success == 23;
                            }
                            return false;
                        }
                ),
                new test::BoolTest(
                        "consume_prefix_str-fail",
                        []() -> bool {
                            string s("456bb");
                            strive tail(s);
                            OptionParserResult<int> res = consume_prefix_str(tail, "123", 23);
                            return !res.has_value();
                        }
                ),
                new test::BoolTest(
                        "consume_prefix_str-fail-empty",
                        []() -> bool {
                            string s("");
                            strive tail(s);
                            OptionParserResult<int> res = consume_prefix_str(tail, "123", 23);
                            return !res.has_value();
                        }
                ),
        };

        test::log_run_test_group(tests);
    }
}