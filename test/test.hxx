#pragma once

#include <functional>
#include <array>

#include "../int.hxx"
#include "../strvec.hxx"

namespace test {
    template<typename Input, typename Output>
    struct test_t {
        typedef std::function<Output(const Input &)> process_f;
        typedef std::function<bool(const Output &, const Output &)> comparator_f;
        typedef std::function<void(const Output &)> output_printer_f;

        string name;
        Input input;
        Output expected_output;
        process_f process;
        comparator_f comparator;
        output_printer_f output_printer;

        test_t(string &&name, Input &&input, Output &&expected_output, process_f process, comparator_f comparator,
               output_printer_f output_printer) : name(name), input(input), expected_output(expected_output),
                                                  process(process), comparator(comparator),
                                                  output_printer(output_printer) {}
    };

    template<typename Input, typename Output>
    using test_group = vector<test_t<Input, Output>>;

    template<typename Input, typename Output>
    auto run_test(const test_t<Input, Output> &x_test) -> bool {
        std::cout << ">> Test \"" << x_test.name << "\".\n";

        Output actual_output = x_test.process(x_test.input);
        bool do_outputs_compare = x_test.comparator(actual_output, x_test.expected_output);
        if (do_outputs_compare) {
            std::cout << "[+] Output check successful.\n";
            std::cout << "Output: ";
            x_test.output_printer(actual_output);
            std::cout << ".\n";
        } else {
            std::cout << "[/] Output check failed.\n";
            std::cout << "Expected output: ";
            x_test.output_printer(x_test.expected_output);
            std::cout << ".\n";

            std::cout << "Actual output: ";
            x_test.output_printer(actual_output);
            std::cout << ".\n";
        }
        return do_outputs_compare;
    }

    using test_group_result = std::tuple<u64, u64>;

    // Returns a tuple of (number of successfuly completed test, number of tests)
    template<typename Input, typename Output>
    auto run_test_group(const test_group<Input, Output> &test_group) -> test_group_result {
        u64 success_counter = 0;

        for (const test_t<Input, Output> &x:test_group) {
            bool success = run_test(x);
            success_counter += success;
        }

        return std::make_tuple(success_counter, test_group.size());
    }

    template<typename Input, typename Output>
    auto log_run_test_group(const test_group<Input, Output> &test_group) -> void {
        test_group_result run_test_group_result = run_test_group(test_group);
        u64 success_counter = std::get<0>(run_test_group_result);
        u64 test_group_size = std::get<1>(run_test_group_result);

        std::cout << "\n";
        std::cout << "Testing complete [" << success_counter << " / " << test_group_size << "].\n";
    }

    template<size_t N>
    auto log_combine_test_groups_results(std::array<test_group_result, N> test_group_results) -> void {
        u64 combined_success_counter = 0;
        u64 combined_test_group_size = 0;

        for (const test_group_result &result : test_group_results) {
            combined_success_counter += std::get<0>(result);
            combined_test_group_size += std::get<1>(result);
        }

        std::cout << "\n";
        std::cout << "Testing complete (combined result) [" << combined_success_counter << " / "
                  << combined_test_group_size << "].\n";
    }
}