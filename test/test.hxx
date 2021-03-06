#pragma once

#include <iostream>
#include <functional>
#include <array>

#include "../int.hxx"
#include "../strvec.hxx"

#include <utility>
#include <variant>

namespace test {
    class TestBase {
    public:
        [[nodiscard]] virtual auto run() const & -> bool = 0;

        virtual ~TestBase() = default;
    };

    template<typename Input, typename Output>
    class Test : public TestBase {
    public:
        typedef std::function<Output(const Input &)> process_f;
        typedef std::function<void(const Output &)> output_printer_f;

    private:
        string name;
        Input input;
        Output expected_output;
        process_f process;
        output_printer_f output_printer;

    public:
        Test(string &&name, Input &&input, Output &&expected_output, process_f process, output_printer_f output_printer)
                : name(move(name)), input(move(input)), expected_output(std::move(expected_output)),
                  process(move(process)), output_printer(move(output_printer)) {}

        [[nodiscard]] auto run() const & -> bool override {
            std::cout << ">> Test \"" << this->name << "\".\n";

            Output actual_output = this->process(this->input);
            bool do_outputs_compare = actual_output == this->expected_output;
            if (do_outputs_compare) {
                std::cout << "[+] Output check successful.\n";
                std::cout << "Output: ";
                this->output_printer(actual_output);
                std::cout << ".\n";
            } else {
                std::cout << "[/] Output check failed.\n";
                std::cout << "Expected output: ";
                this->output_printer(this->expected_output);
                std::cout << ".\n";

                std::cout << "Actual output: ";
                this->output_printer(actual_output);
                std::cout << ".\n";
            }
            return do_outputs_compare;
        }
    };

    class BoolTest : public TestBase {
        static auto print_bool(bool b) -> void {
            std::cout << b;
        }

        typedef Test<std::monostate, bool> InnerTest;
        InnerTest embed;

    public:
        [[nodiscard]] auto run() const & -> bool override {
            return embed.run();
        }

        BoolTest(string &&name, const std::function<bool()> &process) : embed(move(name), std::monostate(), true,
                                                                              [=](std::monostate) -> bool { return process(); },
                                                                              print_bool) {}
    };

    using TestGroup = vector<const TestBase *>;

    using TestGroupResult = std::tuple<u64, u64>;

    // Returns a tuple of (number of successfuly completed test, number of tests)
    auto run_test_group(const TestGroup &test_group) -> TestGroupResult;

    auto log_run_test_group(const TestGroup &test_group) -> void;

    template<size_t N>
    auto log_combine_test_groups_results(std::array<TestGroupResult, N> test_group_results) -> void {
        u64 combined_success_counter = 0;
        u64 combined_test_group_size = 0;

        for (const TestGroupResult &result : test_group_results) {
            combined_success_counter += std::get<0>(result);
            combined_test_group_size += std::get<1>(result);
        }

        std::cout << "\n";
        std::cout << "Testing complete (combined result) [" << combined_success_counter << " / "
                  << combined_test_group_size << "].\n";
    }
}