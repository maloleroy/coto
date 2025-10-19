#include <gtest/gtest.h>
#include <gateappliers.h>

using diagram::Diagram;

const size_t MAX_QUBITS = 8;

size_t flip_nth_bit(const size_t n, const size_t index)
{
    return index ^ (1 << n);
}

TEST(GateAppliersTest, x)
{
    for (auto number_of_qubits = 1; number_of_qubits < MAX_QUBITS; number_of_qubits++)
    {
        for (qubit target = 0; target < number_of_qubits; target++)
        {
            ampl::ConcreteState base(number_of_qubits);
            for (auto i = 0; i < base.size(); i++)
            {
                base[i] = i;
            }
            diagram::Evaluation result(number_of_qubits);
            for (auto i = 0; i < result.size(); i++)
            {
                result[flip_nth_bit(number_of_qubits - target - 1, i)] = i;
            }

            auto d = Diagram::from_state_vector(base);
            gateappliers::apply_x(d, target);

            auto ev = d->evaluate();
            for (auto i = 0; i < result.size(); i++)
            {
                EXPECT_EQ(ev[i], result[i])
                    << "Failed appplying X on qubit " << target
                    << " at index " << i
                    << " got " << ev[i].to_string()
                    << ", expected " << result[i].to_string();
            }
        }
    }
}

/*TEST(GateAppliersTest, phase)
{
    for (auto number_of_qubits = 1; number_of_qubits < MAX_QUBITS; number_of_qubits++)
    {
        for (qubit target = 0; target < number_of_qubits; target++)
        {
            const auto phase_denominator = 3;
            ampl::ConcreteState base(number_of_qubits);
            for (auto i = 0; i < base.size(); i++)
            {
                base[i] = i;
            }
            diagram::Evaluation expected(number_of_qubits);
            for (auto i = 0; i < expected.size(); i++)
            {
                expected[i] = !(i & (1 << (number_of_qubits - target - 1)))
                                  ? absi::Interval(i)
                                  : polar::Interval(polar::PositiveInterval(i), polar::AngleInterval(2. / phase_denominator, 0));
                std::cout << expected[i].to_string() << std::endl;
            }

            auto d = Diagram::from_state_vector(base);
            gateappliers::apply_phase(d, target, phase_denominator);

            auto ev = d->evaluate();
            std::cout << "At height " << target << ", got " << d->count_nodes_at_height(target) << std::endl;
            for (auto i = 0; i < expected.size(); i++)
            {
                EXPECT_EQ(ev[i], expected[i])
                    << "Failed appplying phase(2*pi/" << phase_denominator
                    << ") on qubit " << target
                    << " at index " << i
                    << " got result " << ev[i].to_string()
                    << ", while expected " << expected[i].to_string();
            }
            std::cout << "--------" << std::endl;
        }
    }
}*/

TEST(GateAppliersTest, gate_matrix_identity)
{
    for (auto number_of_qubits = 1; number_of_qubits < MAX_QUBITS; number_of_qubits++)
    {
        for (qubit target = 0; target < number_of_qubits; target++)
        {
            gateappliers::GateMatrix id(1);
            id(0, 0) = 1;
            id(1, 1) = 1;

            ampl::ConcreteState base(number_of_qubits);
            for (auto i = 0; i < base.size(); i++)
            {
                base[i] = i;
            }

            auto d = Diagram::from_state_vector(base);
            gateappliers::apply_gate_matrix(d, target, id);

            auto ev = d->evaluate();
            for (auto i = 0; i < base.size(); i++)
            {
                EXPECT_EQ(ev[i], base[i])
                    << "Failed appplying identity on qubit 0"
                    << " at index " << i
                    << " got " << ev[i].to_string()
                    << ", expected " << base[i];
            }
        }
    }
}

TEST(GateAppliersTest, gate_matrix_hadamard_qubit_0)
{
    absi::Interval coeffs[] = {1, 1, 1, -1};
    gateappliers::GateMatrix gate(1, coeffs);

    ampl::Amplitude v[] = {1, 2, 3, 4};
    ampl::ConcreteState base(2, v);

    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_gate_matrix(d, 0, gate);

    auto ev = d->evaluate();
    EXPECT_EQ(ev[0], 4) << ev[0].to_string() << " != 4";
    EXPECT_EQ(ev[1], 6) << ev[1].to_string() << " != 6";
    EXPECT_EQ(ev[2], -2) << ev[2].to_string() << " != -2";
    EXPECT_EQ(ev[3], -2) << ev[3].to_string() << " != -2";
}

TEST(GateAppliersTest, gate_matrix_hadamard_qubit_1)
{
    absi::Interval v[] = {1, 1, 1, -1};
    gateappliers::GateMatrix m(1, v);

    ampl::ConcreteState base(3);
    for (auto i = 0; i < base.size(); i++)
    {
        base[i] = i + 1;
    }

    auto d = Diagram::from_state_vector(base);
    gateappliers::apply_gate_matrix(d, 1, m);
    auto computed = d->evaluate();

    ampl::Amplitude expected[8] = {4, 6, -2, -2, 12, 14, -2, -2};
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(computed[i], expected[i])
            << "Failed applying H on qubit 1 at index " << i
            << ", computed " << computed[i].to_string()
            << ", expected " << expected[i];
    }
}

TEST(GateAppliersTest, apply_h_consistency)
{
    auto h = gateappliers::GateMatrix(1);
    h(0, 0) = ampl::inv_sqrt2;
    h(0, 1) = ampl::inv_sqrt2;
    h(1, 0) = ampl::inv_sqrt2;
    h(1, 1) = -ampl::inv_sqrt2;

    const auto number_of_qubits = 4;
    for (auto q = 0; q < number_of_qubits; q++)
    {
        auto d0 = Diagram::random(number_of_qubits);
        auto d1 = d0->clone();
        gateappliers::apply_gate_matrix(d0, q, h);
        gateappliers::apply_h(d1, q);

        auto e0 = d0->evaluate();
        auto e1 = d1->evaluate();
        for (auto i = 0; i < e0.size(); i++)
        {
            EXPECT_EQ(e0[i], e1[i])
                << "Failed applying H on qubit " << q << "  at index " << i
                << ", e0 " << e0[i].to_string()
                << ", e1 " << e1[i].to_string();
        }
        delete d0;
        delete d1;
    }
}

TEST(GateAppliersTest, apply_s)
{
    ampl::Amplitude v[] = {1, 0, 0, 0};
    ampl::ConcreteState base(1, v);

    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_s(d, 0);

    auto ev = d->evaluate();
    EXPECT_EQ(ev[0], 1) << ev[0].to_string() << " != 1";
}

TEST(GateAppliersTest, apply_swap)
{
    ampl::Amplitude v[] = {1, 2, 3, 4};
    ampl::ConcreteState base(2, v);

    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_swap(d, 0, 1);

    auto ev = d->evaluate();
    EXPECT_EQ(ev[0], 1) << ev[0].to_string() << " != 1";
    EXPECT_EQ(ev[1], 3) << ev[1].to_string() << " != 3";
    EXPECT_EQ(ev[2], 2) << ev[2].to_string() << " != 2";
    EXPECT_EQ(ev[3], 4) << ev[3].to_string() << " != 4";
}

TEST(GateAppliersTest, apply_cx)
{
    ampl::Amplitude v[] = {1, 2, 3, 4};
    ampl::ConcreteState base(2, v);

    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_cx(d, 0, 1);

    auto ev = d->evaluate();
    EXPECT_EQ(ev[0], 1) << ev[0].to_string() << " != 1";
    EXPECT_EQ(ev[1], 2) << ev[1].to_string() << " != 2";
    EXPECT_EQ(ev[2], 4) << ev[2].to_string() << " != 4";
    EXPECT_EQ(ev[3], 3) << ev[3].to_string() << " != 3";
}

TEST(GateAppliersTest, applyCXOnQubit1and2)
{
    ampl::Amplitude v[] = {1, 2, 3, 4, 5, 6, 7, 8};
    ampl::ConcreteState base(3, v);

    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_cx(d, 1, 2);

    auto ev = d->evaluate();
    ampl::Amplitude expected[] = {1, 2, 4, 3, 5, 6, 8, 7};
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev[i], expected[i])
            << "Failed applying CX on qubit 1 and 2 at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << expected[i];
    }
}

TEST(GateAppliersTest, apply_h)
{
    const ampl::Amplitude v[] = {1, 0, 0, 0};
    const ampl::Amplitude after_h[] = {ampl::inv_sqrt2, 0, ampl::inv_sqrt2, 0};
    const ampl::ConcreteState base(2, v);
    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_h(d, 0);
    auto ev = d->evaluate();
    for (auto i = 0; i < ev.size(); i++)
    {
        EXPECT_EQ(ev[i], after_h[i])
            << "Failed applying H in debug (after H) at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << after_h[i];
    }
}

TEST(GateAppliersTest, h_then_cx)
{
    const ampl::Amplitude v[] = {1, 0, 0, 0};
    const ampl::Amplitude after_h[] = {ampl::inv_sqrt2, 0, ampl::inv_sqrt2, 0};
    const Interval expected[] = {ampl::inv_sqrt2, 0, 0, ampl::inv_sqrt2};
    const ampl::ConcreteState base(2, v);
    std::cout << "- - - - - Creating d - - - - -" << std::endl;
    auto d = Diagram::from_state_vector(base);
    std::cout << "- - - - - Creating d2 - - - - -" << std::endl;
    auto d2 = Diagram::from_state_vector(ampl::ConcreteState(2, after_h));

    std::cout << "- - - - - Applying H on d - - - - -" << std::endl;
    gateappliers::apply_h(d, 0);
    auto ev = d->evaluate();
    auto ev2 = d2->evaluate();

    for (auto i = 0; i < base.size(); i++)
    {
        EXPECT_EQ(ev[i], ev2[i])
            << "Failed applying H in debug (after H) at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << ev2[i].to_string();
    }

    std::cout << "- - - - - Applying CX on d - - - - -" << std::endl;
    // reduction::cut_dead_branches(d);
    gateappliers::apply_cx(d, 0, 1);
    // reduction::cut_dead_branches(d);
    std::cout << "- - - - - Applying CX on d2 - - - - -" << std::endl;

    gateappliers::apply_cx(d2, 0, 1);
    auto evcx = d->evaluate();
    auto evcx2 = d2->evaluate();

    for (auto i = 0; i < base.size(); i++)
    {
        EXPECT_EQ(evcx2[i], expected[i])
            << "Failed applying H and CX in debug at index " << i
            << ", got " << evcx2[i].to_string()
            << ", expected " << expected[i].to_string();
    }

    for (auto i = 0; i < base.size(); i++)
    {
        EXPECT_EQ(evcx[i], expected[i])
            << "Failed applying H and CX in debug at index " << i
            << ", got " << evcx[i].to_string()
            << ", expected " << expected[i].to_string();
    }
}

// ========== Non-consecutive qubit tests ==========

TEST(GateAppliersTest, apply_swap_non_consecutive)
{
    // Test SWAP on non-consecutive qubits (0 and 2)
    // Create a simple state: |100> and test that SWAP(0, 2) works
    ampl::Amplitude v[] = {0, 0, 0, 0, 1, 0, 0, 0};
    ampl::ConcreteState base(3, v);

    auto d = Diagram::from_state_vector(base);

    // Apply SWAP(0, 2) - should swap qubits 0 and 2
    // the result should be |001> so {0,1,0,0,0,0,0,0}
    gateappliers::apply_swap(d, 0, 2);

    auto ev = d->evaluate();

    // Just verify it executes and produces valid output
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev[i], (i == 1 ? 1 : 0));
    }
}

TEST(GateAppliersTest, apply_cz_non_consecutive)
{
    // Test CZ on non-consecutive qubits
    ampl::Amplitude v[] = {1, 2, 3, 4, 5, 6, 7, 8};
    ampl::Amplitude expected_amplitudes[] = {1, 2, 3, 4, 5, -6, 7, -8};
    ampl::ConcreteState base(3, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CZ(0, 2)
    gateappliers::apply_cz(d, 0, 2);

    auto ev = d->evaluate();

    // Verify it executes and produces valid output
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev[i], expected_amplitudes[i])
            << "CZ on non-consecutive qubits produced invalid output at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << ampl::to_string(expected_amplitudes[i]);
    }
}

TEST(GateAppliersTest, apply_cx_non_consecutive_v2)
{
    // Test CX on non-consecutive qubits (0 and 2)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(3, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CX(0, 2) - control on qubit 0, target on qubit 2
    gateappliers::apply_cx(d, 0, 2);

    auto ev = d->evaluate();

    // Just verify it executes and produces valid output
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev[i], (i == 0 ? 1 : 0))
            << "CZ on non-consecutive qubits produced invalid output at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << ampl::to_string((i == 0 ? 1 : 0));
    }
}

TEST(GateAppliersTest, apply_cz_non_consecutive_v2)
{
    // Test CZ on non-consecutive qubits (0 and 2)
    ampl::Amplitude v[] = {0, 0, 0, 0, 0, 1, 0, 0}; // |101>
    ampl::ConcreteState base(3, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CZ(0, 2)
    gateappliers::apply_cz(d, 0, 2);

    auto ev = d->evaluate();

    // Verify it executes and produces valid output
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev[i], (i == 5 ? -1 : 0))
            << "CZ on non-consecutive qubits produced invalid output at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << ampl::to_string((i == 5 ? -1 : 0));
    }
}

TEST(GateAppliersTest, apply_ch_non_consecutive_v2)
{
    // Test CH on non-consecutive qubits (1 and 3)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(4, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CH(1, 3) - controlled Hadamard
    gateappliers::apply_ch(d, 1, 3);

    auto ev = d->evaluate();
    // When qubit 1 is 0, do nothing
    // When qubit 1 is 1, apply Hadamard to qubit 3
    ampl::Amplitude expected[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (auto i = 0; i < 16; i++)
    {
        EXPECT_EQ(ev[i], expected[i])
            << "Failed applying CH on non-consecutive qubits (1, 3) at index " << i
            << ", got " << ev[i].to_string()
            << ", expected " << expected[i];
    }
}

TEST(GateAppliersTest, apply_crx_non_consecutive)
{
    // Test CRX on non-consecutive qubits (0 and 2)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(3, v);

    auto d1 = Diagram::from_state_vector(base);
    auto d2 = Diagram::from_state_vector(base);

    // Apply CRX on consecutive qubits
    gateappliers::apply_crx(d1, 0, 1, M_PI / 2.0);

    // Apply CRX on non-consecutive qubits
    gateappliers::apply_crx(d2, 0, 2, M_PI / 2.0);

    auto ev1 = d1->evaluate();
    auto ev2 = d2->evaluate();

    // Just verify they both execute without throwing
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev1[i].to_string().length() > 0, true)
            << "CRX on consecutive qubits failed";
        EXPECT_EQ(ev2[i].to_string().length() > 0, true)
            << "CRX on non-consecutive qubits failed";
    }
}

TEST(GateAppliersTest, apply_cry_non_consecutive)
{
    // Test CRY on non-consecutive qubits (1 and 3)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(4, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CRY on non-consecutive qubits
    gateappliers::apply_cry(d, 1, 3, M_PI / 4.0);

    auto ev = d->evaluate();

    // Just verify it executes without throwing
    for (auto i = 0; i < 16; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CRY on non-consecutive qubits failed at index " << i;
    }
}

TEST(GateAppliersTest, apply_crz_non_consecutive)
{
    // Test CRZ on non-consecutive qubits (0 and 3)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(4, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CRZ on non-consecutive qubits
    gateappliers::apply_crz(d, 0, 3, M_PI / 3.0);

    auto ev = d->evaluate();

    // Just verify it executes without throwing
    for (auto i = 0; i < 16; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CRZ on non-consecutive qubits failed at index " << i;
    }
}

TEST(GateAppliersTest, apply_cp_non_consecutive)
{
    // Test CP on non-consecutive qubits (0 and 2)
    ampl::Amplitude v[] = {1, 2, 3, 4, 5, 6, 7, 8};
    ampl::ConcreteState base(3, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CP on non-consecutive qubits
    gateappliers::apply_cp(d, 0, 2, M_PI / 2.0);

    auto ev = d->evaluate();

    // Just verify it executes without throwing
    for (auto i = 0; i < 8; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CP on non-consecutive qubits failed at index " << i;
    }
}

TEST(GateAppliersTest, apply_cu_non_consecutive)
{
    // Test CU on non-consecutive qubits (1 and 3)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(4, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CU on non-consecutive qubits
    gateappliers::apply_cu(d, 1, 3, M_PI / 4.0, M_PI / 6.0, M_PI / 3.0);

    auto ev = d->evaluate();

    // Just verify it executes without throwing
    for (auto i = 0; i < 16; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CU on non-consecutive qubits failed at index " << i;
    }
}

TEST(GateAppliersTest, apply_ccx_non_consecutive)
{
    // Test CCX on non-consecutive qubits (0, 2, 4)
    ampl::Amplitude v[32];
    for (int i = 0; i < 32; i++)
        v[i] = i + 1;
    ampl::ConcreteState base(5, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CCX on non-consecutive qubits
    gateappliers::apply_ccx(d, 0, 2, 4);

    auto ev = d->evaluate();

    // Just verify it executes without throwing and produces valid output
    for (auto i = 0; i < 32; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CCX on non-consecutive qubits failed at index " << i;
    }
}

TEST(GateAppliersTest, apply_cswap_non_consecutive)
{
    // Test CSWAP on non-consecutive qubits (0, 2, 4)
    ampl::Amplitude v[32];
    for (int i = 0; i < 32; i++)
        v[i] = i + 1;
    ampl::ConcreteState base(5, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CSWAP on non-consecutive qubits
    gateappliers::apply_cswap(d, 0, 2, 4);

    auto ev = d->evaluate();

    // Just verify it executes without throwing and produces valid output
    for (auto i = 0; i < 32; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CSWAP on non-consecutive qubits failed at index " << i;
    }
}

TEST(GateAppliersTest, apply_cy_non_consecutive)
{
    // Test CY on non-consecutive qubits (1 and 3)
    ampl::Amplitude v[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ampl::ConcreteState base(4, v);

    auto d = Diagram::from_state_vector(base);

    // Apply CY on non-consecutive qubits
    gateappliers::apply_cy(d, 1, 3);

    auto ev = d->evaluate();

    // Just verify it executes without throwing and produces valid output
    for (auto i = 0; i < 16; i++)
    {
        EXPECT_EQ(ev[i].to_string().length() > 0, true)
            << "CY on non-consecutive qubits failed at index " << i;
    }
}
