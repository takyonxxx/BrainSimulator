#pragma once
#include "platform.h"
#include "neuron.h"
#include "synapse.h"
#include "connectome_data.h"
#include <vector>
#include <unordered_map>
#include <string>

class Brain {
public:
    Brain();

    void initialize();
    void step(float dt);
    void reset();

    // Sensory input interface
    void setChemosensoryInput(float leftConc, float rightConc);  // Chemical gradient
    void setMechanosensoryInput(float anterior, float posterior, float noseTouch);
    void setProprioceptiveInput(const std::vector<float>& jointAngles);
    void setThermosensoryInput(float temperature);

    // Motor output interface
    float getForwardDrive() const;
    float getBackwardDrive() const;
    float getTurnSignal() const;         // Positive=right, negative=left
    float getHeadBendSignal() const;
    float getFeedingSignal() const;
    std::vector<float> getMotorOutputs() const;  // All motor neuron activities

    // For visualization
    const std::vector<Neuron>& neurons() const { return m_neurons; }
    const std::vector<Synapse>& synapses() const { return m_synapses; }

    // Stats
    int totalSpikes() const { return m_totalSpikes; }
    float meanFiringRate() const;
    int neuronCount() const { return (int)m_neurons.size(); }
    int synapseCount() const { return (int)m_synapses.size(); }

    // Neuromodulation state
    float dopamineLevel() const { return m_dopamine; }
    float serotoninLevel() const { return m_serotonin; }
    float octopamineLevel() const { return m_octopamine; }

private:
    std::vector<Neuron> m_neurons;
    std::vector<Synapse> m_synapses;
    std::unordered_map<std::string, int> m_nameToId;

    // Neuromodulator levels (global)
    float m_dopamine = 0.5f;
    float m_serotonin = 0.5f;
    float m_octopamine = 0.3f;

    int m_totalSpikes = 0;
    int m_stepCount = 0;

    void propagateSynapses();
    void updateNeuromodulators(float dt);
    int findNeuron(const std::string& name) const;

    // Motor output computation helpers
    float computePopulationActivity(int startId, int endId) const;
};
