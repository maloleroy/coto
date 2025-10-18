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

TEST(GateAppliersTest, phase)
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
}

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
    ampl::Amplitude v[] = {1, 2, 3, 4};
    ampl::ConcreteState base(2, v);

    auto d = Diagram::from_state_vector(base);

    gateappliers::apply_s(d, 0, 1);

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
