#pragma once

#include <iostream>
#include <functional>
#include <array>

#include "../int.hxx"
#include "../strvec.hxx"

#include <variant>

using namespace std;

namespace test {
    class TestBase {
    public:
        virtual auto run() const & -> bool = 0;

        virtual ~TestBase() {};
    };

    template<typename Input, typename Output>
    class Test : public TestBase {
    public:
        typedef function<Output(const Input &)> process_f;
        typedef function<void(const Output &)> output_printer_f;

    private:
        string name;
        Input input;
        Output expected_output;
        process_f process;
        output_printer_f output_printer;

    public:
        Test(string &&name, Input &&input, Output &&expected_output, process_f process, output_printer_f output_printer)
                : name(move(name)), input(move(input)), expected_output(move(expected_output)),
                  process(process), output_printer(output_printer) {}

        virtual auto run() const & -> bool override {
            cout << ">> Test \"" << this->name << "\".\n";

            Output actual_output = this->process(this->input);
            bool do_outputs_compare = actual_output == this->expected_output;
            if (do_outputs_compare) {
                cout << "[+] Output check successful.\n";
                cout << "Output: ";
                this->output_printer(actual_output);
                cout << ".\n";
            } else {
                cout << "[/] Output check failed.\n";
                cout << "Expected output: ";
                this->output_printer(this->expected_output);
                cout << ".\n";

                cout << "Actual output: ";
                this->output_printer(actual_output);
                cout << ".\n";
            }
            return do_outputs_compare;
        }
    };

    class BoolTest : public TestBase {
        static auto print_bool(bool b) -> void {
            cout << b;
        }

        typedef Test<monostate, bool> InnerTest;
        InnerTest embed;

    public:
        virtual auto run() const & -> bool override {
            return embed.run();
        }

        BoolTest(string &&name, function<bool()> process) : embed(move(name), monostate(), true,
                                                                       [=](monostate) -> bool { return process(); },
                                                                       print_bool) {}
    };

    using TestGroup = vector<const TestBase *>;

    using TestGroupResult = tuple<u64, u64>;

    // Returns a tuple of (number of successfuly completed test, number of tests)
    auto run_test_group(const TestGroup &test_group) -> TestGroupResult;

    auto log_run_test_group(const TestGroup &test_group) -> void;

    template<size_t N>
    auto log_combine_test_groups_results(array<TestGroupResult, N> test_group_results) -> void {
        u64 combined_success_counter = 0;
        u64 combined_test_group_size = 0;

        for (const TestGroupResult &result : test_group_results) {
            combined_success_counter += get<0>(result);
            combined_test_group_size += get<1>(result);
        }

        cout << "\n";
        cout << "Testing complete (combined result) [" << combined_success_counter << " / "
                  << combined_test_group_size << "].\n";
    }
}