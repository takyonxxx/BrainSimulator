#include "platform.h"
#include "connectome_data.h"
#include <cmath>
#include <cstdlib>

// Neuron ID ranges:
//   0-59:   Sensory neurons
//   60-179: Interneurons (command, ring, processing)
//   180-269: Motor neurons (A-class, B-class, D-class)
//   270-301: Modulatory neurons

void ConnectomeData::buildConnectome(std::vector<NeuronDef>& neurons,
                                     std::vector<SynapseDef>& synapses)
{
    neurons.clear();
    synapses.clear();
    neurons.reserve(302);
    synapses.reserve(8000);

    buildSensoryNeurons(neurons);
    buildInterneurons(neurons);
    buildMotorNeurons(neurons);
    buildModulatoryNeurons(neurons);

    // Named circuits (specific known pathways)
    buildChemotaxisCircuit(synapses);
    buildLocomotionCircuit(synapses);
    buildTouchCircuit(synapses);
    buildFeedingCircuit(synapses);
    buildAvoidanceCircuit(synapses);
    buildModulatoryCircuit(synapses);
    buildCPGCircuit(synapses);

    // ===================================================================
    // DENSE BACKGROUND CONNECTIVITY
    // Real C. elegans has ~7000 chemical synapses + ~900 gap junctions
    // The named circuits above only cover the well-characterized pathways.
    // Below we add dense probabilistic connectivity based on known
    // connection statistics from the C. elegans connectome.
    // ===================================================================

    srand(12345); // Deterministic for reproducibility

    int nNeurons = (int)neurons.size();

    // --- Sensory -> Interneuron dense connections ---
    // Each sensory neuron connects to 5-15 interneurons
    for (int s = 0; s < 60; s++) {
        int nConn = 5 + (rand() % 11); // 5-15 connections
        for (int c = 0; c < nConn; c++) {
            int target = 60 + (rand() % 120); // Random interneuron
            float w = 1.0f + (rand() % 30) / 10.0f; // 1.0-4.0
            if (rand() % 5 == 0) w = -w; // 20% inhibitory
            synapses.push_back({s, target, w,
                w > 0 ? NeurotransmitterType::Excitatory : NeurotransmitterType::Inhibitory});
        }
    }

    // --- Interneuron -> Interneuron dense connections ---
    // Rich recurrent connectivity - each interneuron connects to 15-35 others
    for (int i = 60; i < 180; i++) {
        int nConn = 15 + (rand() % 21); // 15-35 connections
        for (int c = 0; c < nConn; c++) {
            int target = 60 + (rand() % 120); // Another interneuron
            if (target == i) continue;
            float w = 0.8f + (rand() % 25) / 10.0f;
            if (rand() % 3 == 0) w = -w;
            synapses.push_back({i, target, w,
                w > 0 ? NeurotransmitterType::Excitatory : NeurotransmitterType::Inhibitory});
        }
        // Also connect to some motor neurons directly
        int nMotor = 2 + (rand() % 4);
        for (int c = 0; c < nMotor; c++) {
            int target = 180 + (rand() % 90);
            float w = 1.0f + (rand() % 20) / 10.0f;
            if (rand() % 4 == 0) w = -w;
            synapses.push_back({i, target, w,
                w > 0 ? NeurotransmitterType::Excitatory : NeurotransmitterType::Inhibitory});
        }
    }

    // --- Command interneuron -> Motor neuron dense connections ---
    // AVB (62,63) -> all B-class motor neurons (201-218)
    for (int cmd = 62; cmd <= 63; cmd++) {
        for (int m = 201; m < 219; m++) {
            float w = 2.0f + (rand() % 20) / 10.0f;
            synapses.push_back({cmd, m, w, NeurotransmitterType::Excitatory});
        }
    }
    // AVA (60,61) -> all A-class motor neurons (180-200)
    for (int cmd = 60; cmd <= 61; cmd++) {
        for (int m = 180; m < 201; m++) {
            float w = 2.0f + (rand() % 20) / 10.0f;
            synapses.push_back({cmd, m, w, NeurotransmitterType::Excitatory});
        }
    }

    // --- Interneuron -> Motor neuron (non-command) ---
    // Ring/processing interneurons connect to motor neurons
    for (int i = 66; i < 130; i++) {
        int nConn = 3 + (rand() % 5);
        for (int c = 0; c < nConn; c++) {
            int target = 180 + (rand() % 90); // Random motor neuron
            float w = 1.0f + (rand() % 20) / 10.0f;
            if (rand() % 4 == 0) w = -w;
            synapses.push_back({i, target, w,
                w > 0 ? NeurotransmitterType::Excitatory : NeurotransmitterType::Inhibitory});
        }
    }

    // --- Motor -> Motor adjacent connections (wave propagation) ---
    // Sequential motor neurons connect to neighbors for coordinated waves
    // B-class forward wave
    for (int m = 201; m < 218; m++) {
        synapses.push_back({m, m + 1, 2.0f, NeurotransmitterType::Excitatory});
        if (m > 201) {
            synapses.push_back({m, m - 1, -1.0f, NeurotransmitterType::Inhibitory});
        }
    }
    // A-class backward wave
    for (int m = 189; m < 200; m++) {
        synapses.push_back({m, m + 1, -1.0f, NeurotransmitterType::Inhibitory});
        if (m > 189) {
            synapses.push_back({m, m - 1, 2.0f, NeurotransmitterType::Excitatory});
        }
    }

    // --- D-class cross inhibition (dorsal-ventral alternation) ---
    // DD neurons inhibit VB, VD neurons inhibit DB
    for (int dd = 219; dd < 225; dd++) {
        for (int vb = 208; vb < 219; vb++) {
            if (rand() % 3 == 0) {
                synapses.push_back({dd, vb, -2.5f, NeurotransmitterType::Inhibitory});
            }
        }
    }
    for (int vd = 225; vd < 238; vd++) {
        for (int db = 201; db < 208; db++) {
            if (rand() % 3 == 0) {
                synapses.push_back({vd, db, -2.5f, NeurotransmitterType::Inhibitory});
            }
        }
    }

    // --- Motor -> D-class activation ---
    for (int da = 180; da < 189; da++) {
        int dd = 219 + (da - 180) % 6;
        synapses.push_back({da, dd, 2.0f, NeurotransmitterType::Excitatory});
    }
    for (int va = 189; va < 201; va++) {
        int vd = 225 + (va - 189) % 13;
        if (vd < 238)
            synapses.push_back({va, vd, 2.0f, NeurotransmitterType::Excitatory});
    }

    // --- Motor neuron -> Proprioceptive feedback ---
    for (int m = 180; m < 250; m++) {
        if (rand() % 5 == 0) {
            int propTarget = 44 + (rand() % 16); // Proprioceptive sensory range
            if (propTarget < 60) {
                synapses.push_back({m, propTarget, 0.5f, NeurotransmitterType::Excitatory});
            }
        }
    }

    // --- Modulatory -> widespread connections ---
    // Dopaminergic neurons (270-277) -> modulate command + motor
    for (int d = 270; d < 278; d++) {
        // DA -> command interneurons
        for (int cmd = 60; cmd < 78; cmd++) {
            if (rand() % 3 == 0) {
                synapses.push_back({d, cmd, 1.5f, NeurotransmitterType::Modulatory});
            }
        }
        // DA -> motor neurons (sparse)
        for (int m = 180; m < 250; m++) {
            if (rand() % 10 == 0) {
                synapses.push_back({d, m, 1.0f, NeurotransmitterType::Modulatory});
            }
        }
    }

    // Serotonergic neurons (278-285) -> slow down locomotion, enhance feeding
    for (int s5ht = 278; s5ht < 286; s5ht++) {
        // 5-HT -> inhibit forward command
        synapses.push_back({s5ht, 62, -2.0f, NeurotransmitterType::Inhibitory});
        synapses.push_back({s5ht, 63, -2.0f, NeurotransmitterType::Inhibitory});
        // 5-HT -> excite pharynx
        for (int ph = 264; ph < 273 && ph < nNeurons; ph++) {
            synapses.push_back({s5ht, ph, 2.0f, NeurotransmitterType::Excitatory});
        }
        // 5-HT -> modulate sensory processing
        for (int sens = 0; sens < 24; sens++) {
            if (rand() % 4 == 0) {
                synapses.push_back({s5ht, sens, 1.0f, NeurotransmitterType::Modulatory});
            }
        }
    }

    // Tyraminergic (286-289) -> enhance reversal/escape
    for (int tyr = 286; tyr < 290; tyr++) {
        synapses.push_back({tyr, 60, 3.0f, NeurotransmitterType::Modulatory});
        synapses.push_back({tyr, 61, 3.0f, NeurotransmitterType::Modulatory});
        // Tyramine inhibits forward
        synapses.push_back({tyr, 62, -2.0f, NeurotransmitterType::Inhibitory});
        synapses.push_back({tyr, 63, -2.0f, NeurotransmitterType::Inhibitory});
    }

    // --- Sensory -> Modulatory (food detection activates DA/5-HT) ---
    for (int s = 0; s < 24; s++) {
        if (rand() % 3 == 0) {
            int modTarget = 270 + (rand() % 20);
            if (modTarget < nNeurons) {
                synapses.push_back({s, modTarget, 2.0f, NeurotransmitterType::Excitatory});
            }
        }
    }

    // --- Head motor circuitry (SMD/RMD for turning) ---
    // SMD neurons (112-115) receive from RIA (70-71) and drive head muscles
    for (int smd = 112; smd <= 115; smd++) {
        // SMD -> head motor neurons
        for (int hm = 258; hm < 264 && hm < nNeurons; hm++) {
            synapses.push_back({smd, hm, 2.5f, NeurotransmitterType::Excitatory});
        }
        // SMD -> A-class (contribute to omega turn)
        synapses.push_back({smd, 180 + (smd - 112), 2.0f, NeurotransmitterType::Excitatory});
    }

    // --- RIA hub connectivity (major integration center) ---
    // RIA receives from many interneurons and projects to SMD
    for (int src = 62; src < 90; src++) {
        if (rand() % 4 == 0) {
            synapses.push_back({src, 70, 1.5f, NeurotransmitterType::Excitatory});
        }
        if (rand() % 4 == 0) {
            synapses.push_back({src, 71, 1.5f, NeurotransmitterType::Excitatory});
        }
    }

    // --- Reciprocal inhibition for decision circuits ---
    // AVA <-> AVB mutual inhibition (forward vs backward decision)
    synapses.push_back({60, 62, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({60, 63, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({61, 62, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({61, 63, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({62, 60, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({62, 61, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({63, 60, -4.0f, NeurotransmitterType::Inhibitory});
    synapses.push_back({63, 61, -4.0f, NeurotransmitterType::Inhibitory});

    // --- Gap junction analogs (electrical coupling between same-class neurons) ---
    // Left-right pairs tend to be electrically coupled
    auto addGapJunction = [&](int a, int b, float strength) {
        if (a < nNeurons && b < nNeurons) {
            synapses.push_back({a, b, strength, NeurotransmitterType::Excitatory});
            synapses.push_back({b, a, strength, NeurotransmitterType::Excitatory});
        }
    };

    // Sensory L-R pairs
    for (int i = 0; i < 24; i += 2) addGapJunction(i, i + 1, 1.5f);
    // Command interneuron L-R pairs
    for (int i = 60; i < 78; i += 2) addGapJunction(i, i + 1, 2.0f);
    // Ring interneuron L-R pairs
    for (int i = 78; i < 110; i += 2) addGapJunction(i, i + 1, 1.0f);
    // Motor neuron chain coupling
    for (int i = 180; i < 218; i++) {
        if (i + 1 < 219) addGapJunction(i, i + 1, 0.5f);
    }
}

// ===================================================================
// Individual neuron type builders (unchanged from before)
// ===================================================================

void ConnectomeData::buildSensoryNeurons(std::vector<NeuronDef>& n)
{
    const char* amphidNames[] = {
        "AWAL","AWAR","AWBL","AWBR","AWCL","AWCR",
        "ASEL","ASER","ASGL","ASGR","ASHL","ASHR",
        "ASIL","ASIR","ASJL","ASJR","ASKL","ASKR",
        "ADLL","ADLR","ADFL","ADFR","AFDL","AFDR"
    };
    for (int i = 0; i < 24; i++) {
        float angle = (float)i / 24.0f * 2.0f * M_PI;
        n.push_back({amphidNames[i], NeuronType::Sensory, NeurotransmitterType::Excitatory,
                     0.9f * cosf(angle), 0.9f * sinf(angle), 0.9f});
    }

    const char* mechNames[] = {
        "ALML","ALMR","AVM","AVMR","PLML","PLMR","PVM","PVMR",
        "OLQVL","OLQVR","OLQDL","OLQDR","IL1VL","IL1VR","IL1DL","IL1DR",
        "FLPL","FLPR","PVDL","PVDR"
    };
    for (int i = 0; i < 20; i++) {
        float z = 0.8f - (float)i / 20.0f * 1.6f;
        float angle = (float)i / 20.0f * M_PI;
        n.push_back({mechNames[i], NeuronType::Sensory, NeurotransmitterType::Excitatory,
                     0.7f * cosf(angle), 0.7f * sinf(angle), z});
    }

    const char* propNames[] = {
        "DVA","DVB","PVR","PVNL","PVNR","PQR",
        "PHBL","PHBR","PHAL","PHAR","PHCL","PHCR",
        "CEPDL","CEPDR","CEPVL","CEPVR"
    };
    for (int i = 0; i < 16; i++) {
        float z = -0.5f + (float)i / 16.0f * 0.4f;
        float angle = (float)i / 16.0f * 2.0f * M_PI;
        n.push_back({propNames[i], NeuronType::Sensory, NeurotransmitterType::Excitatory,
                     0.5f * cosf(angle), 0.5f * sinf(angle), z});
    }
}

void ConnectomeData::buildInterneurons(std::vector<NeuronDef>& n)
{
    const char* cmdNames[] = {
        "AVAL","AVAR","AVBL","AVBR","AVDL","AVDR","AVEL","AVER",
        "PVCL","PVCR","AVFL","AVFR","AVHL","AVHR","AVJL","AVJR",
        "AVKL","AVKR"
    };
    for (int i = 0; i < 18; i++) {
        float angle = (float)i / 18.0f * 2.0f * M_PI;
        n.push_back({cmdNames[i], NeuronType::Inter, NeurotransmitterType::Excitatory,
                     0.3f * cosf(angle), 0.3f * sinf(angle), 0.3f});
    }

    const char* ringNames[] = {
        "AIAL","AIAR","AIBL","AIBR","AINL","AINR","AIYL","AIYR",
        "AIZL","AIZR","RIAL","RIAR","RIBL","RIBR","RICL","RICR",
        "RIDL","RIDR","RIFL","RIFR","RIGL","RIGR","RIHL","RIHR",
        "RIML","RIMR","RINL","RINR","RIPL","RIPR","RIRL","RIRR"
    };
    for (int i = 0; i < 32; i++) {
        float angle = (float)i / 32.0f * 2.0f * M_PI;
        float r = 0.45f + 0.1f * sinf(angle * 3.0f);
        n.push_back({ringNames[i], NeuronType::Inter,
                     (i % 4 < 2) ? NeurotransmitterType::Excitatory : NeurotransmitterType::Inhibitory,
                     r * cosf(angle), r * sinf(angle), 0.5f});
    }

    const char* procNames[] = {
        "SAADL","SAADR","SAAVL","SAAVR","SIADL","SIADR","SIAVL","SIAVR",
        "SIBDL","SIBDR","SIBVL","SIBVR","SMBDL","SMBDR","SMBVL","SMBVR",
        "SMDDL","SMDDR","SMDVL","SMDVR","URADL","URADR","URAVL","URAVR",
        "URBL","URBR","URXL","URXR","URYDL","URYDR","URYVL","URYVR",
        "BAGDL","BAGDR","IL2VL","IL2VR","IL2DL","IL2DR","BAGL","BAGR",
        "RMDDL","RMDDR","RMDVL","RMDVR","RMDL","RMDR","RMED","RMEV",
        "RMEL","RMER","RMFL","RMFR","RMGL","RMGR","RMHL","RMHR",
        "LUAL","LUAR","PVAL","PVAR","PVPL","PVPR","PVQL","PVQR",
        "PVWL","PVWR","PVTL","PVTR","ADAL","ADAR"
    };
    for (int i = 0; i < 70; i++) {
        float t = (float)i / 70.0f;
        float angle = t * 4.0f * M_PI;
        float r = 0.3f + 0.2f * t;
        float z = 0.6f - t * 1.2f;
        n.push_back({procNames[i], NeuronType::Inter,
                     (i % 3 == 0) ? NeurotransmitterType::Inhibitory : NeurotransmitterType::Excitatory,
                     r * cosf(angle), r * sinf(angle), z});
    }
}

void ConnectomeData::buildMotorNeurons(std::vector<NeuronDef>& n)
{
    for (int i = 0; i < 9; i++) {
        float z = 0.4f - (float)i / 9.0f * 1.4f;
        n.push_back({"DA" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Excitatory,
                     0.15f, 0.2f, z});
    }
    for (int i = 0; i < 12; i++) {
        float z = 0.4f - (float)i / 12.0f * 1.4f;
        n.push_back({"VA" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Excitatory,
                     -0.15f, -0.2f, z});
    }
    for (int i = 0; i < 7; i++) {
        float z = 0.4f - (float)i / 7.0f * 1.4f;
        n.push_back({"DB" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Excitatory,
                     0.2f, 0.15f, z});
    }
    for (int i = 0; i < 11; i++) {
        float z = 0.4f - (float)i / 11.0f * 1.4f;
        n.push_back({"VB" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Excitatory,
                     -0.2f, -0.15f, z});
    }
    for (int i = 0; i < 6; i++) {
        float z = 0.3f - (float)i / 6.0f * 1.2f;
        n.push_back({"DD" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Inhibitory,
                     0.1f, 0.25f, z});
    }
    for (int i = 0; i < 13; i++) {
        float z = 0.35f - (float)i / 13.0f * 1.3f;
        n.push_back({"VD" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Inhibitory,
                     -0.1f, -0.25f, z});
    }
    for (int i = 0; i < 11; i++) {
        float z = 0.35f - (float)i / 11.0f * 1.3f;
        n.push_back({"AS" + std::to_string(i+1), NeuronType::Motor, NeurotransmitterType::Excitatory,
                     0.25f, 0.0f, z});
    }
    const char* headMotor[] = {"RIVL","RIVR","RMGL_M","RMGR_M","SABD","SABV"};
    for (int i = 0; i < 6; i++) {
        float angle = (float)i / 6.0f * 2.0f * M_PI;
        n.push_back({headMotor[i], NeuronType::Motor, NeurotransmitterType::Excitatory,
                     0.3f * cosf(angle), 0.3f * sinf(angle), 0.7f});
    }
    const char* pharynx[] = {"M1","M2L","M2R","M3L","M3R","M4","M5","I1L","I1R"};
    for (int i = 0; i < 9; i++) {
        float angle = (float)i / 9.0f * 2.0f * M_PI;
        n.push_back({pharynx[i], NeuronType::Motor, NeurotransmitterType::Excitatory,
                     0.15f * cosf(angle), 0.15f * sinf(angle), 0.85f});
    }
    int remaining = 270 - (int)n.size();
    for (int i = 0; i < remaining; i++) {
        n.push_back({"MN" + std::to_string(i), NeuronType::Motor, NeurotransmitterType::Excitatory,
                     0.2f * cosf((float)i), 0.2f * sinf((float)i), -0.5f + (float)i * 0.05f});
    }
}

void ConnectomeData::buildModulatoryNeurons(std::vector<NeuronDef>& n)
{
    const char* dopaNames[] = {"CEPDL_D","CEPDR_D","CEPVL_D","CEPVR_D","ADEL_D","ADER_D","PDEL","PDER"};
    for (int i = 0; i < 8; i++) {
        float angle = (float)i / 8.0f * 2.0f * M_PI;
        n.push_back({dopaNames[i], NeuronType::Modulatory, NeurotransmitterType::Modulatory,
                     0.6f * cosf(angle), 0.6f * sinf(angle), 0.4f});
    }
    const char* seroNames[] = {"NSML","NSMR","HSNL","HSNR","ADFL_S","ADFR_S","AIML","AIMR"};
    for (int i = 0; i < 8; i++) {
        float angle = (float)i / 8.0f * 2.0f * M_PI + 0.3f;
        n.push_back({seroNames[i], NeuronType::Modulatory, NeurotransmitterType::Modulatory,
                     0.55f * cosf(angle), 0.55f * sinf(angle), 0.2f});
    }
    const char* tyraNames[] = {"RIML_T","RIMR_T","RICL_T","RICR_T"};
    for (int i = 0; i < 4; i++) {
        float angle = (float)i / 4.0f * 2.0f * M_PI;
        n.push_back({tyraNames[i], NeuronType::Modulatory, NeurotransmitterType::Modulatory,
                     0.4f * cosf(angle), 0.4f * sinf(angle), 0.35f});
    }
    int remaining = 302 - (int)n.size();
    for (int i = 0; i < remaining; i++) {
        n.push_back({"MOD" + std::to_string(i), NeuronType::Modulatory, NeurotransmitterType::Modulatory,
                     0.45f * cosf(i * 1.1f), 0.45f * sinf(i * 1.1f), 0.1f});
    }
}

// ===================================================================
// Named circuit builders - key identified pathways
// ===================================================================

void ConnectomeData::buildChemotaxisCircuit(std::vector<SynapseDef>& s)
{
    // AWA -> AIY (attractive chemotaxis)
    s.push_back({0, 84, 4.0f, NeurotransmitterType::Excitatory});   // AWAL->AIYL (66+18=84? NO, ring starts at 78)
    // Recalculate: cmd=60-77(18), ring=78-109(32), proc=110-179(70)
    // AIY = ring index 6,7 => neuron 78+6=84, 78+7=85
    s.push_back({0, 84, 4.0f, NeurotransmitterType::Excitatory});
    s.push_back({1, 85, 4.0f, NeurotransmitterType::Excitatory});
    // AWC -> AIY
    s.push_back({4, 84, 3.0f, NeurotransmitterType::Excitatory});
    s.push_back({5, 85, 3.0f, NeurotransmitterType::Excitatory});
    // AWB -> AIZ (avoidance odor)
    // AIZ = ring index 8,9 => 78+8=86, 78+9=87
    s.push_back({2, 86, 3.0f, NeurotransmitterType::Excitatory});
    s.push_back({3, 87, 3.0f, NeurotransmitterType::Excitatory});
    // ASE -> AIY
    s.push_back({6, 84, 5.0f, NeurotransmitterType::Excitatory});
    s.push_back({7, 85, 5.0f, NeurotransmitterType::Excitatory});
    // AIY -> AIZ
    s.push_back({84, 86, 3.5f, NeurotransmitterType::Excitatory});
    s.push_back({85, 87, 3.5f, NeurotransmitterType::Excitatory});
    // AIZ -> RIA (ring index 10,11 => 78+10=88, 78+11=89)
    s.push_back({86, 88, 4.0f, NeurotransmitterType::Excitatory});
    s.push_back({87, 89, 4.0f, NeurotransmitterType::Excitatory});
    // RIA -> SMD (proc index 16-19 => 110+16=126? No, proc starts at 110)
    // SMD = proc index 16,17,18,19 => 110+16=126,127,128,129
    // Wait - let me recount: cmd(18) + ring(32) = 50, so proc starts at 60+50=110
    // SMDDL=proc[16]=126, SMDDR=proc[17]=127
    s.push_back({88, 126, 3.5f, NeurotransmitterType::Excitatory});
    s.push_back({89, 127, 3.5f, NeurotransmitterType::Excitatory});
    // ASH (nociceptive, id=10,11) -> AVA (backward)
    s.push_back({10, 60, 6.0f, NeurotransmitterType::Excitatory});
    s.push_back({11, 61, 6.0f, NeurotransmitterType::Excitatory});
    // AFD (thermosensory, id=22,23) -> AIY
    s.push_back({22, 84, 3.0f, NeurotransmitterType::Excitatory});
    s.push_back({23, 85, 3.0f, NeurotransmitterType::Excitatory});
}

void ConnectomeData::buildLocomotionCircuit(std::vector<SynapseDef>& s)
{
    // PVC -> AVB (forward trigger)
    s.push_back({68, 62, 4.0f, NeurotransmitterType::Excitatory});
    s.push_back({69, 63, 4.0f, NeurotransmitterType::Excitatory});
    // AVD -> AVA (backward cascade)
    s.push_back({64, 60, 3.0f, NeurotransmitterType::Excitatory});
    s.push_back({65, 61, 3.0f, NeurotransmitterType::Excitatory});
    // AVE -> AVA
    s.push_back({66, 60, 2.0f, NeurotransmitterType::Excitatory});
    s.push_back({67, 61, 2.0f, NeurotransmitterType::Excitatory});
}

void ConnectomeData::buildTouchCircuit(std::vector<SynapseDef>& s)
{
    // ALM -> AVD (anterior touch -> backward)
    s.push_back({24, 64, 5.0f, NeurotransmitterType::Excitatory});
    s.push_back({25, 65, 5.0f, NeurotransmitterType::Excitatory});
    s.push_back({26, 60, 4.0f, NeurotransmitterType::Excitatory});
    // PLM -> PVC (posterior touch -> forward)
    s.push_back({28, 68, 5.0f, NeurotransmitterType::Excitatory});
    s.push_back({29, 69, 5.0f, NeurotransmitterType::Excitatory});
    s.push_back({30, 62, 4.0f, NeurotransmitterType::Excitatory});
    // Nose touch -> AVA
    for (int i = 32; i < 40; i++) {
        s.push_back({i, 60, 3.0f, NeurotransmitterType::Excitatory});
        s.push_back({i, 61, 3.0f, NeurotransmitterType::Excitatory});
    }
    // Harsh touch
    s.push_back({40, 60, 6.0f, NeurotransmitterType::Excitatory});
    s.push_back({41, 68, 6.0f, NeurotransmitterType::Excitatory});
}

void ConnectomeData::buildFeedingCircuit(std::vector<SynapseDef>& s)
{
    // Pharynx: M1(264)-M5(268), I1L(272), I1R(273)
    // Food sensors -> pharynx
    s.push_back({6, 264, 3.0f, NeurotransmitterType::Excitatory});
    s.push_back({0, 264, 2.0f, NeurotransmitterType::Excitatory});
    // Pharyngeal sequence
    for (int i = 264; i < 272; i++) {
        if (i + 1 < 273)
            s.push_back({i, i + 1, 2.5f, NeurotransmitterType::Excitatory});
    }
}

void ConnectomeData::buildAvoidanceCircuit(std::vector<SynapseDef>& s)
{
    // ADL -> AVA
    s.push_back({18, 60, 4.0f, NeurotransmitterType::Excitatory});
    s.push_back({19, 61, 4.0f, NeurotransmitterType::Excitatory});
    // SMD -> DA1 (head motor for omega turn)
    s.push_back({126, 180, 3.0f, NeurotransmitterType::Excitatory});
    s.push_back({127, 189, 3.0f, NeurotransmitterType::Excitatory});
}

void ConnectomeData::buildModulatoryCircuit(std::vector<SynapseDef>& s)
{
    // (Handled in dense connectivity section above)
    (void)s;
}

void ConnectomeData::buildCPGCircuit(std::vector<SynapseDef>& s)
{
    // (Handled in dense connectivity section above - motor chain + D-class)
    (void)s;
}
