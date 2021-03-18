#include <iostream>

#include "tests/assembly.hxx"
#include "parsec/tests/tests.hxx"

int main() {
//    syntax::parse::test_parser();
    tests::test_assembly();
    parsec::tests::test();
}
