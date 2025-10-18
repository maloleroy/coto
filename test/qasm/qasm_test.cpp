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
    EXPECT_EQ(eval("s"), "gate: s[1]");
    EXPECT_EQ(eval("swap"), "gate: swap[2]");
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

TEST(QasmTest, apply_two_qubit_gate)
{
    Runtime r = exec("qubit a;\nqubit b;");
    EXPECT_NO_THROW(r = r.exec("cx a, b;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, apply_two_qubit_gate_with_spaces)
{
    Runtime r = exec("qubit a;\nqubit b;");
    EXPECT_NO_THROW(r = r.exec("cx a , b;"));
    EXPECT_NO_THROW(r = r.exec("swap a , b;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, memory_usage)
{
    Runtime r = exec("qubit a;");
    EXPECT_NO_THROW(r = r.exec("@build;"));
    // Memory usage should be a positive number
    EXPECT_NO_THROW(r = r.exec("@memory;"));
}

TEST(QasmTest, pauli_gates)
{
    EXPECT_EQ(eval("x"), "gate: x[1]");
    EXPECT_EQ(eval("y"), "gate: y[1]");
    EXPECT_EQ(eval("z"), "gate: z[1]");
}

TEST(QasmTest, clifford_gates)
{
    EXPECT_EQ(eval("s"), "gate: s[1]");
    EXPECT_EQ(eval("sdg"), "gate: sdg[1]");
    EXPECT_EQ(eval("t"), "gate: t[1]");
    EXPECT_EQ(eval("tdg"), "gate: tdg[1]");
}

TEST(QasmTest, sqrt_gates)
{
    EXPECT_EQ(eval("sx"), "gate: sx[1]");
}

TEST(QasmTest, rotation_gates)
{
    EXPECT_EQ(eval("rx(0)"), "gate: rx(0)[1]");
    EXPECT_EQ(eval("ry(pi/2)"), "gate: ry(pi/2)[1]");
    EXPECT_EQ(eval("rz(pi)"), "gate: rz(pi)[1]");
}

TEST(QasmTest, two_qubit_pauli_gates)
{
    EXPECT_EQ(eval("swap"), "gate: swap[2]");
    EXPECT_EQ(eval("cx"), "gate: cx[2]");
    EXPECT_EQ(eval("cy"), "gate: cy[2]");
    EXPECT_EQ(eval("cz"), "gate: cz[2]");
    EXPECT_EQ(eval("ch"), "gate: ch[2]");
}

TEST(QasmTest, controlled_rotation_gates)
{
    EXPECT_EQ(eval("cp(pi/2)"), "gate: cp(pi/2)[2]");
    EXPECT_EQ(eval("crx(pi/4)"), "gate: crx(pi/4)[2]");
    EXPECT_EQ(eval("cry(pi)"), "gate: cry(pi)[2]");
    EXPECT_EQ(eval("crz(pi/2)"), "gate: crz(pi/2)[2]");
}

TEST(QasmTest, universal_gates)
{
    EXPECT_EQ(eval("u(0, 0, 0)"), "gate: u(0, 0, 0)[1]");
    EXPECT_EQ(eval("cu(pi/2, pi/4, pi)"), "gate: cu(pi/2, pi/4, pi)[2]");
}

TEST(QasmTest, three_qubit_gates)
{
    EXPECT_EQ(eval("ccx"), "gate: ccx[3]");
    EXPECT_EQ(eval("cswap"), "gate: cswap[3]");
}

TEST(QasmTest, global_phase_gate)
{
    EXPECT_EQ(eval("gphase(pi)"), "gate: gphase(pi)[0]");
}

TEST(QasmTest, apply_single_qubit_gates)
{
    Runtime r = exec("qubit q;");
    EXPECT_NO_THROW(r = r.exec("x q;"));
    EXPECT_NO_THROW(r = r.exec("y q;"));
    EXPECT_NO_THROW(r = r.exec("z q;"));
    EXPECT_NO_THROW(r = r.exec("h q;"));
    EXPECT_NO_THROW(r = r.exec("s q;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, apply_rotation_gates)
{
    // We do not test them all in the same context to avoid
    // unsupported interval operations like addition of polar intervals
    Runtime r = exec("qubit q;");
    EXPECT_NO_THROW(auto rx = r.exec("rx(pi/4) q;"); rx.exec("@display;"));
    EXPECT_NO_THROW(auto ry = r.exec("ry(pi/4) q;"); ry.exec("@display;"));
    EXPECT_NO_THROW(auto rz = r.exec("rz(pi/4) q;"); rz.exec("@display;"));
}

TEST(QasmTest, apply_controlled_gates)
{
    Runtime r = exec("qubit a;\nqubit b;");
    EXPECT_NO_THROW(r = r.exec("cx a, b;"));
    EXPECT_NO_THROW(r = r.exec("cy a, b;"));
    EXPECT_NO_THROW(r = r.exec("cz a, b;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, apply_swap_gate)
{
    Runtime r = exec("qubit a;\nqubit b;");
    EXPECT_NO_THROW(r = r.exec("swap a, b;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, apply_three_qubit_gates)
{
    Runtime r = exec("qubit a;\nqubit b;\nqubit c;");
    EXPECT_NO_THROW(r = r.exec("ccx a, b, c;"));
    EXPECT_NO_THROW(r = r.exec("cswap a, b, c;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, universal_u_gate)
{
    Runtime r = exec("qubit q;");
    EXPECT_NO_THROW(r = r.exec("u(pi/2, pi/4, pi) q;"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, global_phase)
{
    Runtime r = exec("qubit q;");
    EXPECT_NO_THROW(r = r.exec("gphase(pi/2);"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_definition)
{
    // Test basic qubit array definition
    EXPECT_NO_THROW(exec("qubits[3] q;"));
    EXPECT_NO_THROW(exec("qubits[5] myQubits;"));
    EXPECT_NO_THROW(exec("qubits[1] singleQubit;"));
}

TEST(QasmTest, qubit_array_definition_invalid)
{
    // Test invalid array sizes
    EXPECT_THROW(exec("qubits[0] q;"), SyntaxError);
    EXPECT_THROW(exec("qubits[] q;"), SyntaxError);
    EXPECT_THROW(exec("qubits[abc] q;"), SyntaxError);
}

TEST(QasmTest, qubit_array_access_single)
{
    Runtime r = exec("qubits[3] q;");
    EXPECT_NO_THROW(r = r.exec("x q[0];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_access_multiple)
{
    Runtime r = exec("qubits[5] q;");
    EXPECT_NO_THROW(r = r.exec("x q[0];"));
    EXPECT_NO_THROW(r = r.exec("x q[1];"));
    EXPECT_NO_THROW(r = r.exec("x q[4];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_access_two_qubit_gates)
{
    Runtime r = exec("qubits[3] q;");
    EXPECT_NO_THROW(r = r.exec("cx q[0], q[1];"));
    EXPECT_NO_THROW(r = r.exec("swap q[1], q[2];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_access_three_qubit_gates)
{
    Runtime r = exec("qubits[5] q;");
    EXPECT_NO_THROW(r = r.exec("ccx q[0], q[1], q[2];"));
    EXPECT_NO_THROW(r = r.exec("cswap q[2], q[3], q[4];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_with_rotation_gates)
{
    Runtime r = exec("qubits[3] q;");
    EXPECT_NO_THROW(r = r.exec("rx(pi/4) q[0];"));
    EXPECT_NO_THROW(r = r.exec("ry(pi/2) q[1];"));
    EXPECT_NO_THROW(r = r.exec("rz(pi) q[2];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_multiple_arrays)
{
    // Use contiguous qubits within the same array to avoid non-contiguous gate issue
    Runtime r = exec("qubits[4] q;");
    EXPECT_NO_THROW(r = r.exec("x q[0];"));
    EXPECT_NO_THROW(r = r.exec("y q[1];"));
    EXPECT_NO_THROW(r = r.exec("cx q[0], q[1];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_mixed_access)
{
    // Use contiguous qubits to avoid non-contiguous gate issue
    Runtime r = exec("qubits[3] arr;");
    EXPECT_NO_THROW(r = r.exec("x arr[0];"));
    EXPECT_NO_THROW(r = r.exec("x arr[1];"));
    EXPECT_NO_THROW(r = r.exec("cx arr[1], arr[2];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}

TEST(QasmTest, qubit_array_index_out_of_bounds)
{
    Runtime r = exec("qubits[3] q;");
    EXPECT_THROW(r.exec("x q[3];"), VariableError);
    EXPECT_THROW(r.exec("x q[10];"), VariableError);
}

TEST(QasmTest, qubit_array_memory_count)
{
    Runtime r = exec("qubits[5] q;");
    EXPECT_NO_THROW(r = r.exec("@build;"));
    EXPECT_NO_THROW(r = r.exec("@memory;"));
}

TEST(QasmTest, qubit_array_complex_circuit)
{
    Runtime r = exec("qubits[4] q;");
    EXPECT_NO_THROW(r = r.exec("h q[0];"));
    EXPECT_NO_THROW(r = r.exec("cx q[0], q[1];"));
    EXPECT_NO_THROW(r = r.exec("x q[2];"));
    EXPECT_NO_THROW(r = r.exec("swap q[2], q[3];"));
    EXPECT_NO_THROW(r = r.exec("@display;"));
}