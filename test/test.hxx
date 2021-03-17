#pragma once

#include <functional>
#include <array>

#include "../int.hxx"
#include "../strvec.hxx"

namespace test {
    class TestBase {
    public:
        virtual auto run() const & -> bool = 0;
    };

    template<typename Input, typename Output>
    class Test : public TestBase {
        typedef std::function<Output(const Input &)> process_f;
        typedef std::function<bool(const Output &, const Output &)> comparator_f;
        typedef std::function<void(const Output &)> output_printer_f;

        string name;
        Input input;
        Output expected_output;
        process_f process;
        comparator_f comparator;
        output_printer_f output_printer;

    public:
        Test(string &&name, Input &&input, Output &&expected_output, process_f process, comparator_f comparator,
             output_printer_f output_printer) : name(name), input(input), expected_output(expected_output),
                                                process(process), comparator(comparator),
                                                output_printer(output_printer) {}

        virtual auto run() const & -> bool override {
            std::cout << ">> Test \"" << this->name << "\".\n";

            Output actual_output = this->process(this->input);
            bool do_outputs_compare = this->comparator(actual_output, this->expected_output);
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

    using TestGroup = vector<const TestBase *>;

    using TestGroupResult = std::tuple<u64, u64>;

    // Returns a tuple of (number of successfuly completed test, number of tests)
    auto run_test_group(const TestGroup &test_group) -> TestGroupResult {
        u64 success_counter = 0;

        for (const TestBase *x:test_group) {
            bool success = x->run();
            success_counter += success;
        }

        return std::make_tuple(success_counter, test_group.size());
    }

    auto log_run_test_group(const TestGroup &test_group) -> void {
        TestGroupResult run_test_group_result = run_test_group(test_group);
        u64 success_counter = std::get<0>(run_test_group_result);
        u64 test_group_size = std::get<1>(run_test_group_result);

        std::cout << "\n";
        std::cout << "Testing complete [" << success_counter << " / " << test_group_size << "].\n";
    }

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