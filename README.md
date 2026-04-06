# BrainSim - C. elegans Connectome Whole-Brain Emulation

Gerçek C. elegans (Caenorhabditis elegans) konnektom verisine dayanan, 302 nöron ve ~7500 sinaptik bağlantı içeren tam beyin emülasyonu simülatörü. Eon Systems'in meyve sineği projesinde kullandığı Leaky Integrate-and-Fire (LIF) nöron modelini temel alır.

## Özellikler

### Beyin Modeli
- **302 Leaky Integrate-and-Fire nöron** (gerçek C. elegans nöron isimleri ile)
- **~7500 kimyasal sinaps** (gerçek bağlantı paternleri)
- **7 ayrı sinir devresi:**
  - Kemotaksi (AWA/AWC → AIY → AIZ → RIA → SMD) - yiyecek arama
  - Hareket (AVA/AVB → DA/VA/DB/VB) - ileri/geri
  - Dokunma (ALM/PLM → AVD/PVC) - dokunma tepkisi
  - Beslenme (ASE → M1-M5) - farinks pompası
  - Kaçma (ASH → AVA/AVD → omega dönüşü)
  - Nöromodülasyon (DA, 5-HT, OA/TA)
  - CPG (Merkezi Patern Üreteci) - ritmik yürüme
- **3 nöromodülatör sistemi:** Dopamin, Serotonin, Oktopamin
- **Kısa süreli sinaptik plastisite** (Tsodyks-Markram modeli)

### Yaratık
- 6 bacaklı böcek benzeri gövde (tripod yürüyüşü)
- Her bacak 3 eklemli (coxa, femur, tibia)
- Kimyasal sensörler (sol/sağ anten)
- Mekanosensörler (ön/arka/burun dokunma)
- Proprioseptif sensörler (bacak pozisyonu)
- Tam duyusal-motor döngü (sensorimotor loop)

### Ortam
- OpenGL 3D dünya (yerçekimi, ışıklandırma)
- Yiyecek kaynakları (kimyasal gradyan alanı)
- Engeller (kayalar)
- Dünya sınırları

### Görselleştirme
- Gerçek zamanlı 3D beyin aktivitesi (nöron ateşlemeleri, sinaps akışları)
- Nöron tipi renk kodlaması (Duyusal=yeşil, Ara=mavi, Motor=turuncu, Modülatör=mor)
- Nöromodülatör seviyeleri HUD
- 4 kamera modu (Takip, Serbest, Kuşbakışı, Beyin Görünümü)
- Davranış durumu göstergesi

## Gereksinimler

- Qt 6.10+ (Core, Gui, Widgets, OpenGL, OpenGLWidgets)
- CMake 3.16+
- C++17 destekli derleyici
- OpenGL 2.1+ (Uyumluluk profili)

## Derleme

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
./BrainSim
```

## Klavye Kısayolları

| Tuş | İşlev |
|------|--------|
| SPACE | Başlat/Durdur |
| R | Sıfırla |
| S | Tek adım |
| B | Beyin görselini göster/gizle |
| 1 | Takip kamerası |
| 2 | Serbest kamera |
| 3 | Kuşbakışı |
| 4 | Beyin görünümü |
| +/- | Simülasyon hızı |

## Fare Kontrolleri

- **Sol tuş sürükleme:** Kamera döndürme
- **Sağ tuş sürükleme:** Kamera kaydırma
- **Tekerlek:** Yakınlaştırma/uzaklaştırma

## Mimari

```
src/
├── brain/
│   ├── neuron.{h,cpp}          # LIF nöron modeli
│   ├── synapse.h               # Sinaps + kısa süreli plastisite
│   ├── connectome_data.{h,cpp} # 302 nöron + devre tanımları
│   └── brain.{h,cpp}           # Sinir ağı simülasyonu
├── creature/
│   ├── creature.{h,cpp}        # Gövde + sensorimotor döngü
│   ├── leg.{h,cpp}             # 3 eklemli bacak + CPG
│   └── sensor.{h,cpp}          # Kimyasal/mekanik/proprioseptif
├── world/
│   ├── world.{h,cpp}           # Ortam yönetimi
│   ├── food.h                  # Yiyecek kaynakları
│   └── obstacle.h              # Engeller
├── renderer/
│   ├── creature_renderer.{h,cpp} # Yaratık OpenGL çizimi
│   ├── world_renderer.{h,cpp}   # Dünya OpenGL çizimi
│   └── brain_renderer.{h,cpp}   # Beyin aktivitesi görselleştirme
├── glwidget.{h,cpp}            # Ana OpenGL widget
├── mainwindow.{h,cpp}          # Qt ana pencere + UI
└── main.cpp                    # Giriş noktası
```

## Bilimsel Referanslar

- White, J.G. et al. (1986). "The structure of the nervous system of C. elegans." Phil. Trans. R. Soc. Lond. B
- Cook, S.J. et al. (2019). "Whole-animal connectomes of both C. elegans sexes." Nature
- FlyWire Consortium (2024). "Neuronal wiring diagram of an adult brain." Nature
- Eon Systems (2026). Whole-brain emulation of Drosophila melanogaster
- OpenWorm Project - open source C. elegans simulation

## Nasıl Çalışır

Eon Systems'in meyve sineği projesiyle aynı prensipleri kullanır:

1. **Konnektom verisi yüklenir** (302 nöron, gerçek bağlantı paternleri)
2. **Simülasyon başlar** - hiçbir davranış programlanmaz
3. **Duyusal girdiler** ortamdan toplanır (kimyasal gradyan, dokunma)
4. **Sinirsel aktivite** LIF denklemleriyle hesaplanır
5. **Motor çıktıları** beynin kendi devre dinamiklerinden ortaya çıkar
6. **Davranışlar** - yürüme, yiyecek arama, engelden kaçma - tamamen "emergent"tir

Tıpkı Eon Systems'in dijital sineğinde olduğu gibi, burada da **davranışlar öğretilmez, sinir devresinden kendiliğinden ortaya çıkar**.
