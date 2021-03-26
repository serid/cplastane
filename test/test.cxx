#include "test.hxx"

using namespace std;

namespace test {
    // Returns a tuple of (number of successfuly completed test, number of tests)
    auto run_test_group(const TestGroup &test_group) -> TestGroupResult {
        u64 success_counter = 0;

        for (const TestBase *x:test_group) {
            bool success = x->run();
            success_counter += success;
        }

        return make_tuple(success_counter, test_group.size());
    }

    auto log_run_test_group(const TestGroup &test_group) -> void {
        TestGroupResult run_test_group_result = run_test_group(test_group);
        u64 success_counter = get<0>(run_test_group_result);
        u64 test_group_size = get<1>(run_test_group_result);

        cout << "\n";
        cout << "Testing complete [" << success_counter << " / " << test_group_size << "].\n";
    }
}