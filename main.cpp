#include <iostream>

#include "tests/assembly.hxx"
#include "parsec/tests/tests.hxx"
#include "assembly/parse/parse.hxx"
#include "jit/jit.hxx"

auto eval(strive s) -> i64 {
    auto mnemos = assembly::parse::parse(s).value().data;
    auto bytes = assembly::assemble(mnemos);
    return jit::eval_mc(bytes.data(), bytes.size());
}

int main() {
//    syntax::parse::test_parser();
//    tests::test_assembly();
    parsec::tests::test();

    assembly::parse::test();
}
