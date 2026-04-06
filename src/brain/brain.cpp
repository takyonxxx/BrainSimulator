#include "platform.h"
#include "brain.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

Brain::Brain() {}

void Brain::initialize()
{
    std::vector<NeuronDef> neuronDefs;
    std::vector<SynapseDef> synapseDefs;
    ConnectomeData::buildConnectome(neuronDefs, synapseDefs);

    m_neurons.clear();
    m_neurons.reserve(neuronDefs.size());
    m_nameToId.clear();

    for (int i = 0; i < (int)neuronDefs.size(); i++) {
        auto& nd = neuronDefs[i];
        m_neurons.emplace_back(i, nd.name, nd.type, nd.nt);
        m_neurons.back().posX = nd.x;
        m_neurons.back().posY = nd.y;
        m_neurons.back().posZ = nd.z;
        m_nameToId[nd.name] = i;

        // --- KEY FIX: Set intrinsic bias current for tonic activity ---
        // Command interneurons need spontaneous activity to drive locomotion
        // This mimics the biological tonic depolarization in C. elegans
        if (nd.type == NeuronType::Inter) {
            // AVB (forward command) - default forward locomotion bias
            if (i >= 62 && i <= 63)
                m_neurons.back().params.i_bias = 12.0f;  // Strong forward drive
            // AVA (backward command) - weaker baseline
            else if (i >= 60 && i <= 61)
                m_neurons.back().params.i_bias = 3.0f;
            // Ring interneurons - moderate tonic activity
            else if (i >= 66 && i <= 97)
                m_neurons.back().params.i_bias = 5.0f;
            // Other interneurons
            else
                m_neurons.back().params.i_bias = 2.0f;
        }
        // Motor neurons need some baseline excitability
        else if (nd.type == NeuronType::Motor) {
            // B-class (forward) motor neurons
            if (i >= 201 && i < 219)
                m_neurons.back().params.i_bias = 4.0f;
            // A-class (backward) motor neurons
            else if (i >= 180 && i < 201)
                m_neurons.back().params.i_bias = 1.0f;
            else
                m_neurons.back().params.i_bias = 2.0f;
        }
        // Modulatory neurons - slow tonic firing
        else if (nd.type == NeuronType::Modulatory) {
            m_neurons.back().params.i_bias = 6.0f;
        }
    }

    m_synapses.clear();
    m_synapses.reserve(synapseDefs.size());

    for (auto& sd : synapseDefs) {
        if (sd.from >= 0 && sd.from < (int)m_neurons.size() &&
            sd.to >= 0 && sd.to < (int)m_neurons.size()) {
            Synapse syn;
            syn.preNeuronId = sd.from;
            syn.postNeuronId = sd.to;
            syn.weight = sd.weight;
            syn.baseWeight = sd.weight;
            syn.delay = 1.0f;
            m_synapses.push_back(syn);
        }
    }

    m_totalSpikes = 0;
    m_stepCount = 0;

    // --- Warm up the brain: run a few hundred steps to build up tonic activity ---
    for (int i = 0; i < 200; i++) {
        step(1.0f);
    }
    m_totalSpikes = 0; // Reset counter after warmup
}

void Brain::step(float dt)
{
    m_stepCount++;

    // 1. Add spontaneous noise to ALL neuron types every step
    //    This is critical for generating variable behavior
    for (auto& neuron : m_neurons) {
        float noise = ((float)(rand() % 10000) / 10000.0f - 0.5f);

        switch (neuron.type()) {
        case NeuronType::Sensory:
            neuron.injectCurrent(noise * 3.0f);
            break;
        case NeuronType::Inter:
            neuron.injectCurrent(noise * 5.0f);
            break;
        case NeuronType::Motor:
            neuron.injectCurrent(noise * 2.0f);
            break;
        case NeuronType::Modulatory:
            neuron.injectCurrent(noise * 2.0f);
            break;
        }
    }

    // 2. Propagate synaptic currents
    propagateSynapses();

    // 3. Update all neurons
    for (auto& neuron : m_neurons) {
        neuron.step(dt);
        if (neuron.hasFired()) {
            m_totalSpikes++;
        }
    }

    // 4. Update neuromodulator levels
    updateNeuromodulators(dt);

    // 5. Update synapse dynamics
    for (auto& syn : m_synapses) {
        bool preFired = m_neurons[syn.preNeuronId].hasFired();
        syn.step(dt, preFired);
    }

    // 6. Periodic exploration bursts - mimics C. elegans pirouette behavior
    //    Every few seconds, inject a burst into turning circuits
    if (m_stepCount % 300 == 0) {
        float burstSide = (rand() % 2 == 0) ? 1.0f : -1.0f;
        // RIA/SMD turning circuit
        if (burstSide > 0) {
            m_neurons[70].injectCurrent(15.0f);  // RIAL
            m_neurons[112].injectCurrent(10.0f); // SMDDL
        } else {
            m_neurons[71].injectCurrent(15.0f);  // RIAR
            m_neurons[113].injectCurrent(10.0f); // SMDDR
        }
    }

    // 7. Occasional reversal events (pirouettes)
    if (m_stepCount % 500 == 0 && (rand() % 3 == 0)) {
        m_neurons[60].injectCurrent(20.0f);  // AVAL - trigger reversal
        m_neurons[61].injectCurrent(20.0f);  // AVAR
    }
}

void Brain::propagateSynapses()
{
    for (auto& syn : m_synapses) {
        if (m_neurons[syn.preNeuronId].hasFired()) {
            float current = syn.effectiveWeight();

            // Neuromodulatory gain
            auto preType = m_neurons[syn.preNeuronId].ntType();
            if (preType == NeurotransmitterType::Modulatory) {
                current *= (1.0f + m_dopamine * 0.5f);
            }

            m_neurons[syn.postNeuronId].injectCurrent(current);
        }

        // Also propagate graded potentials (subthreshold) for sensory neurons
        // C. elegans neurons often use graded transmission, not just spikes
        auto& pre = m_neurons[syn.preNeuronId];
        if (pre.type() == NeuronType::Sensory && !pre.hasFired()) {
            float graded = (pre.voltage() - pre.params.v_rest) /
                           (pre.params.v_threshold - pre.params.v_rest);
            if (graded > 0.3f) {
                m_neurons[syn.postNeuronId].injectCurrent(
                    syn.weight * graded * 0.3f);
            }
        }
    }
}

void Brain::updateNeuromodulators(float dt)
{
    float dopaActivity = 0, seroActivity = 0, octaActivity = 0;
    int dopaCount = 0, seroCount = 0, octaCount = 0;

    for (int i = 270; i < (int)m_neurons.size(); i++) {
        if (i < 278) {
            dopaActivity += m_neurons[i].activity();
            dopaCount++;
        } else if (i < 286) {
            seroActivity += m_neurons[i].activity();
            seroCount++;
        } else if (i < 290) {
            octaActivity += m_neurons[i].activity();
            octaCount++;
        }
    }

    if (dopaCount > 0) dopaActivity /= dopaCount;
    if (seroCount > 0) seroActivity /= seroCount;
    if (octaCount > 0) octaActivity /= octaCount;

    float tau_mod = 500.0f;
    m_dopamine += (dopaActivity * 2.0f + 0.3f - m_dopamine) * (dt / tau_mod);
    m_serotonin += (seroActivity * 2.0f + 0.2f - m_serotonin) * (dt / tau_mod);
    m_octopamine += (octaActivity * 2.0f + 0.1f - m_octopamine) * (dt / tau_mod);

    m_dopamine = (std::clamp)(m_dopamine, 0.05f, 1.0f);
    m_serotonin = (std::clamp)(m_serotonin, 0.05f, 1.0f);
    m_octopamine = (std::clamp)(m_octopamine, 0.05f, 1.0f);
}

void Brain::reset()
{
    for (auto& n : m_neurons) n.reset();
    m_totalSpikes = 0;
    m_stepCount = 0;
    m_dopamine = 0.5f;
    m_serotonin = 0.5f;
    m_octopamine = 0.3f;

    // Re-warm the brain
    for (int i = 0; i < 200; i++) {
        step(1.0f);
    }
    m_totalSpikes = 0;
}

// ===== SENSORY INPUT =====

void Brain::setChemosensoryInput(float leftConc, float rightConc)
{
    // Higher gain - sensory neurons need strong input to drive behavior
    m_neurons[0].injectCurrent(leftConc * 20.0f);   // AWAL
    m_neurons[1].injectCurrent(rightConc * 20.0f);   // AWAR

    m_neurons[4].injectCurrent(leftConc * 15.0f);    // AWCL
    m_neurons[5].injectCurrent(rightConc * 15.0f);   // AWCR

    float totalConc = (leftConc + rightConc) * 0.5f;
    m_neurons[6].injectCurrent(totalConc * 18.0f);   // ASEL
    m_neurons[7].injectCurrent(totalConc * 18.0f);   // ASER

    // Food detected -> activate serotonergic system (slow down to feed)
    if (totalConc > 0.3f) {
        for (int i = 278; i < 286 && i < (int)m_neurons.size(); i++) {
            m_neurons[i].injectCurrent(totalConc * 10.0f);
        }
    }

    // Food gradient -> differential turning via AIY/AIZ
    float diff = leftConc - rightConc;
    if (std::abs(diff) > 0.02f) {
        if (diff > 0) {
            m_neurons[70].injectCurrent(std::abs(diff) * 15.0f);  // RIAL - turn left
        } else {
            m_neurons[71].injectCurrent(std::abs(diff) * 15.0f);  // RIAR - turn right
        }
    }
}

void Brain::setMechanosensoryInput(float anterior, float posterior, float noseTouch)
{
    // Anterior touch -> strong backward drive
    if (anterior > 0.05f) {
        m_neurons[24].injectCurrent(anterior * 25.0f);
        m_neurons[25].injectCurrent(anterior * 25.0f);
        m_neurons[26].injectCurrent(anterior * 20.0f);
        // Direct drive to AVA for fast response
        m_neurons[60].injectCurrent(anterior * 15.0f);
        m_neurons[61].injectCurrent(anterior * 15.0f);
    }

    // Posterior touch -> forward escape
    if (posterior > 0.05f) {
        m_neurons[28].injectCurrent(posterior * 25.0f);
        m_neurons[29].injectCurrent(posterior * 25.0f);
        m_neurons[30].injectCurrent(posterior * 20.0f);
        // Direct drive to AVB
        m_neurons[62].injectCurrent(posterior * 15.0f);
        m_neurons[63].injectCurrent(posterior * 15.0f);
    }

    // Nose touch -> immediate reversal + turn
    if (noseTouch > 0.05f) {
        for (int i = 32; i < 40; i++) {
            m_neurons[i].injectCurrent(noseTouch * 30.0f);
        }
        // Strong AVA activation for reversal
        m_neurons[60].injectCurrent(noseTouch * 25.0f);
        m_neurons[61].injectCurrent(noseTouch * 25.0f);
        // Also trigger a turn
        m_neurons[70 + (rand() % 2)].injectCurrent(noseTouch * 20.0f);
    }
}

void Brain::setProprioceptiveInput(const std::vector<float>& jointAngles)
{
    if (!jointAngles.empty()) {
        float totalBend = 0;
        for (float a : jointAngles) totalBend += std::abs(a);
        m_neurons[44].injectCurrent(totalBend * 8.0f);
        // Also feed back to motor coordination
        m_neurons[45].injectCurrent(totalBend * 5.0f);
    }
}

void Brain::setThermosensoryInput(float temperature)
{
    m_neurons[22].injectCurrent(temperature * 10.0f);
    m_neurons[23].injectCurrent(temperature * 10.0f);
}

// ===== MOTOR OUTPUT =====

float Brain::computePopulationActivity(int startId, int endId) const
{
    float sum = 0;
    int count = 0;
    for (int i = startId; i < endId && i < (int)m_neurons.size(); i++) {
        sum += m_neurons[i].activity();
        count++;
    }
    return count > 0 ? sum / count : 0.0f;
}

float Brain::getForwardDrive() const
{
    // B-class motor neurons (DB + VB)
    float db = computePopulationActivity(201, 208);
    float vb = computePopulationActivity(208, 219);
    // AVB command interneuron activity
    float avb = (m_neurons[62].activity() + m_neurons[63].activity()) * 0.5f;
    // Higher gain for motor output
    return ((db + vb) * 0.5f + avb * 0.5f) * 3.0f;
}

float Brain::getBackwardDrive() const
{
    float da = computePopulationActivity(180, 189);
    float va = computePopulationActivity(189, 201);
    float ava = (m_neurons[60].activity() + m_neurons[61].activity()) * 0.5f;
    return ((da + va) * 0.5f + ava * 0.5f) * 3.0f;
}

float Brain::getTurnSignal() const
{
    // SMD asymmetry
    float smdL = 0, smdR = 0;
    if (112 < (int)m_neurons.size()) smdL += m_neurons[112].activity();
    if (114 < (int)m_neurons.size()) smdL += m_neurons[114].activity();
    if (113 < (int)m_neurons.size()) smdR += m_neurons[113].activity();
    if (115 < (int)m_neurons.size()) smdR += m_neurons[115].activity();
    smdL *= 0.5f;
    smdR *= 0.5f;

    // RIA contribution
    float riaL = m_neurons[70].activity();
    float riaR = m_neurons[71].activity();

    // Higher gain for visible turning
    return ((smdR - smdL) * 2.0f + (riaR - riaL) * 1.5f) * 2.0f;
}

float Brain::getHeadBendSignal() const
{
    float rmdD = computePopulationActivity(130, 134);
    float rmdV = computePopulationActivity(134, 138);
    return (rmdD - rmdV) * 2.0f;
}

float Brain::getFeedingSignal() const
{
    return computePopulationActivity(264, 273) * 2.0f;
}

std::vector<float> Brain::getMotorOutputs() const
{
    std::vector<float> outputs;
    for (int i = 180; i < 270 && i < (int)m_neurons.size(); i++) {
        outputs.push_back(m_neurons[i].activity());
    }
    return outputs;
}

float Brain::meanFiringRate() const
{
    float sum = 0;
    for (auto& n : m_neurons) sum += n.activity();
    return m_neurons.empty() ? 0 : sum / m_neurons.size();
}

int Brain::findNeuron(const std::string& name) const
{
    auto it = m_nameToId.find(name);
    return it != m_nameToId.end() ? it->second : -1;
}
