#pragma once
#include "platform.h"
#include <vector>
#include <string>
#include <cstdint>

// Leaky Integrate-and-Fire (LIF) neuron model
// Same model type used by Eon Systems for whole-brain emulation
enum class NeuronType {
    Sensory,      // Receives input from environment
    Inter,        // Interneuron - processing
    Motor,        // Outputs to muscles/actuators
    Modulatory    // Neuromodulatory (dopamine, serotonin analogs)
};

enum class NeurotransmitterType {
    Excitatory,   // Glutamate analog - ACh in C. elegans
    Inhibitory,   // GABA analog
    Modulatory    // Dopamine/Serotonin/Octopamine analog
};

struct NeuronParams {
    float tau_m       = 20.0f;    // Membrane time constant (ms)
    float v_rest      = -65.0f;   // Resting potential (mV)
    float v_threshold = -50.0f;   // Spike threshold (mV)
    float v_reset     = -70.0f;   // Reset potential after spike (mV)
    float v_peak      = 30.0f;    // Spike peak for visualization
    float tau_ref     = 2.0f;     // Refractory period (ms)
    float i_bias      = 0.0f;     // Intrinsic bias current
    float leak_rate   = 0.05f;    // Leak conductance
};

class Neuron {
public:
    Neuron(int id, const std::string& name, NeuronType type,
           NeurotransmitterType nt = NeurotransmitterType::Excitatory);

    void step(float dt);
    void injectCurrent(float current);
    void reset();

    bool hasFired() const { return m_fired; }
    float voltage() const { return m_voltage; }
    float activity() const { return m_activity; } // Smoothed firing rate
    int id() const { return m_id; }
    const std::string& name() const { return m_name; }
    NeuronType type() const { return m_type; }
    NeurotransmitterType ntType() const { return m_ntType; }

    // Position in 3D brain space for visualization
    float posX = 0, posY = 0, posZ = 0;

    NeuronParams params;

private:
    int m_id;
    std::string m_name;
    NeuronType m_type;
    NeurotransmitterType m_ntType;

    float m_voltage = -65.0f;
    float m_inputCurrent = 0.0f;
    float m_refractoryTimer = 0.0f;
    bool m_fired = false;
    float m_activity = 0.0f;       // Exponential moving average of firing
    float m_activityDecay = 0.985f; // Slow decay - keeps activity visible longer
};
