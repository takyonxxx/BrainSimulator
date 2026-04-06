#include "platform.h"
#include "neuron.h"
#include <cmath>
#include <algorithm>

Neuron::Neuron(int id, const std::string& name, NeuronType type,
               NeurotransmitterType nt)
    : m_id(id), m_name(name), m_type(type), m_ntType(nt)
{
    m_voltage = params.v_rest;

    // Different neuron types have slightly different dynamics
    switch (type) {
    case NeuronType::Sensory:
        params.tau_m = 10.0f;
        params.v_threshold = -55.0f;   // Easy to fire (close to rest)
        params.tau_ref = 1.5f;
        break;
    case NeuronType::Inter:
        params.tau_m = 8.0f;           // Fast dynamics
        params.v_threshold = -55.0f;
        params.tau_ref = 1.0f;
        break;
    case NeuronType::Motor:
        params.tau_m = 12.0f;
        params.v_threshold = -56.0f;   // Very easy to fire
        params.tau_ref = 2.0f;
        break;
    case NeuronType::Modulatory:
        params.tau_m = 15.0f;
        params.v_threshold = -54.0f;
        params.tau_ref = 3.0f;
        break;
    }
}

void Neuron::step(float dt)
{
    m_fired = false;

    // Refractory period
    if (m_refractoryTimer > 0.0f) {
        m_refractoryTimer -= dt;
        m_inputCurrent = 0.0f;
        m_activity *= m_activityDecay;
        return;
    }

    // Standard LIF equation: tau_m * dV/dt = -(V - V_rest) + I_total
    // The -(V - V_rest) term IS the leak - no separate leak needed!
    float I_total = m_inputCurrent + params.i_bias;
    float dv = (-(m_voltage - params.v_rest) + I_total) / params.tau_m;

    // Sub-step Euler integration for stability (dt can be 1-4ms)
    int subSteps = (dt > 1.5f) ? 4 : 1;
    float subDt = dt / (float)subSteps;
    for (int s = 0; s < subSteps; s++) {
        m_voltage += dv * subDt;
        // Recompute dv with new voltage
        dv = (-(m_voltage - params.v_rest) + I_total) / params.tau_m;
    }

    // Spike detection
    if (m_voltage >= params.v_threshold) {
        m_fired = true;
        m_voltage = params.v_reset;
        m_refractoryTimer = params.tau_ref;
        m_activity = (std::min)(1.0f, m_activity + 0.4f);
    } else {
        m_activity *= m_activityDecay;
    }

    // Clamp voltage
    m_voltage = (std::clamp)(m_voltage, -80.0f, params.v_peak);

    // Reset input current for next step
    m_inputCurrent = 0.0f;
}

void Neuron::injectCurrent(float current)
{
    m_inputCurrent += current;
}

void Neuron::reset()
{
    m_voltage = params.v_rest;
    m_inputCurrent = 0.0f;
    m_refractoryTimer = 0.0f;
    m_fired = false;
    m_activity = 0.0f;
}
