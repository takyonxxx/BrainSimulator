#pragma once
#include "platform.h"
#include "neuron.h"
#include "synapse.h"
#include <vector>
#include <string>

// C. elegans-inspired connectome data
// Real C. elegans has 302 neurons with ~7000 chemical synapses and ~900 gap junctions
// This implements the core identified circuits with real neuron names and connectivity patterns
// References: WormAtlas, OpenWorm project, Cook et al. 2019, White et al. 1986

struct NeuronDef {
    std::string name;
    NeuronType type;
    NeurotransmitterType nt;
    float x, y, z;  // Approximate 3D position in normalized brain space
};

struct SynapseDef {
    int from;
    int to;
    float weight;
    NeurotransmitterType type;
};

class ConnectomeData {
public:
    // Build full connectome with 302 neurons
    static void buildConnectome(std::vector<NeuronDef>& neurons,
                                std::vector<SynapseDef>& synapses);

private:
    // Circuit builders
    static void buildSensoryNeurons(std::vector<NeuronDef>& neurons);
    static void buildInterneurons(std::vector<NeuronDef>& neurons);
    static void buildMotorNeurons(std::vector<NeuronDef>& neurons);
    static void buildModulatoryNeurons(std::vector<NeuronDef>& neurons);

    // Connectivity patterns from real C. elegans data
    static void buildChemotaxisCircuit(std::vector<SynapseDef>& synapses);
    static void buildLocomotionCircuit(std::vector<SynapseDef>& synapses);
    static void buildTouchCircuit(std::vector<SynapseDef>& synapses);
    static void buildFeedingCircuit(std::vector<SynapseDef>& synapses);
    static void buildAvoidanceCircuit(std::vector<SynapseDef>& synapses);
    static void buildModulatoryCircuit(std::vector<SynapseDef>& synapses);
    static void buildCPGCircuit(std::vector<SynapseDef>& synapses);
};
