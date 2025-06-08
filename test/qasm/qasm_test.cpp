#include <gtest/gtest.h>
#include <qasm.h>
#include <qasm/error.h>

using namespace qasm;

TEST(QasmTest, empty)
{
    EXPECT_NO_THROW(exec(""));
    EXPECT_THROW(exec(";"), SyntaxError);
    EXPECT_THROW(exec("  ;"), SyntaxError);
    EXPECT_THROW(exec("  \n;"), SyntaxError);
    EXPECT_THROW(exec("  \t;"), SyntaxError);
}

TEST(QasmTest, definition)
{
    EXPECT_THROW(exec("bit 6a;"), SyntaxError);
    EXPECT_THROW(exec("bit a~8;"), SyntaxError);
    EXPECT_NO_THROW(exec("bit my_clean_Bit887121;"));

    EXPECT_NO_THROW(exec("int n;").eval("n"));

    EXPECT_THROW(exec("int x;"), VariableError);
}

TEST(QasmTest, assignment)
{
    EXPECT_THROW(exec("a!3 q=2;"), SyntaxError);
    EXPECT_THROW(exec("int a!3=2;"), SyntaxError);
    EXPECT_THROW(exec("3a=2;"), SyntaxError);

    EXPECT_EQ(exec("int n=2;").eval("n"), "int: 2");

    EXPECT_EQ(exec("bit a=1;").eval("a"), "bit: 1");

    EXPECT_THROW(exec("qubit q=0;"), VariableError);
}

TEST(QasmTest, version)
{
    EXPECT_THROW(exec("OPENQASM 2.0;"), VersionError);
    EXPECT_NO_THROW(exec("OPENQASM 3.0;"));
    EXPECT_NO_THROW(exec("OPENQASM 3;"));
}

TEST(QasmTest, include)
{
    EXPECT_NO_THROW(exec("include \"file.qasm\""));
}

TEST(QasmTest, qubits)
{
    Runtime r = exec("qubit a;");
    r.exec("qubit b;");
    EXPECT_EQ(r.eval("a"), "qubit: 0");
    EXPECT_EQ(r.eval("b"), "qubit: 1");
}

TEST(QasmTest, common_gates)
{
    EXPECT_EQ(eval("x"), "gate: x[1]");
    EXPECT_EQ(eval("h"), "gate: h[1]");
    EXPECT_EQ(eval("cx"), "gate: cx[2]");
    EXPECT_EQ(eval("s"), "gate: s[2]");
}

TEST(QasmTest, phase_gate)
{
    EXPECT_EQ(eval("p(pi/4)"), "gate: p(pi/4)[1]");
    Runtime r = exec("qubit q;");
    EXPECT_NO_THROW(r.exec("p(pi/4) q;"));
    EXPECT_NO_THROW(r.exec("p(2pi/3) q;"));
    EXPECT_NO_THROW(r.exec("@display;"));
}

TEST(QasmTest, apply_gate)
{
    Runtime r = exec("qubit a;");
    EXPECT_NO_THROW(r = r.exec("x a;"));
    EXPECT_EQ(r.eval("a"), "qubit: 0");
    EXPECT_NO_THROW(exec("@display;"));
}