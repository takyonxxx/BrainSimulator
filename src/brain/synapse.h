#pragma once
#include "platform.h"

// Synapse with short-term plasticity and optional STDP
struct Synapse {
    int preNeuronId;
    int postNeuronId;
    float weight;           // Synaptic weight (positive=excitatory, negative=inhibitory)
    float delay;            // Axonal delay in ms
    float baseWeight;       // Original weight for homeostasis

    // Short-term dynamics
    float facilitation = 1.0f;   // Short-term facilitation factor
    float depression   = 1.0f;   // Short-term depression factor
    float tau_f = 200.0f;        // Facilitation time constant
    float tau_d = 500.0f;        // Depression time constant
    float tau_r = 800.0f;        // Recovery time constant

    // Transmission
    float transmitterLevel = 1.0f;

    void step(float dt, bool preFired) {
        // Short-term plasticity (Tsodyks-Markram model simplified)
        if (preFired) {
            float released = transmitterLevel * depression * facilitation;
            transmitterLevel -= released * 0.1f;
            facilitation += 0.1f;  // Use-dependent facilitation
            depression *= 0.9f;    // Use-dependent depression
        }

        // Recovery
        transmitterLevel += (1.0f - transmitterLevel) * (dt / tau_r);
        facilitation += (1.0f - facilitation) * (dt / tau_f);
        depression += (1.0f - depression) * (dt / tau_d);
    }

    float effectiveWeight() const {
        return weight * transmitterLevel * facilitation * depression;
    }
};
