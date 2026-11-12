#pragma once
#include <Geode/Geode.hpp>

#define CFG_FIELDS(X) \
    X(greenDash) X(noOrbFx) X(blackOrb) X(easyBlackUfo) X(jumpPads) X(gravPads) \
    X(maintainGrav) X(straightFly) X(straightUfo) X(acP1) X(acP2) X(acP1Swift) \
    X(acP2Swift) X(noclip) X(autoSwift) X(link2p) X(waveGap) X(wgP1On) X(wgP1Blue) \
    X(wgP2On) X(wgP2Blue) X(greenPlayer) X(maintainPlayer) X(flyPlayer) X(ufoPlayer) \
    X(noclipPlayer) X(acP1Every) X(acP1Hold) X(acP2Every) X(acP2Hold) X(linkMode) \
    X(linkDrive) X(flyThr) X(ufoThr) X(ufoY) X(wgP1Thr) X(wgP1Center) X(wgP2Thr) \
    X(wgP2Center)

struct Cfg {
    bool greenDash = false, noOrbFx = false, blackOrb = false, easyBlackUfo = false;
    bool jumpPads = false, gravPads = false, maintainGrav = false;
    bool straightFly = false, straightUfo = false;
    bool acP1 = false, acP2 = false, acP1Swift = false, acP2Swift = false;
    bool noclip = false, autoSwift = false, link2p = false, waveGap = false;
    bool wgP1On = true, wgP1Blue = false, wgP2On = true, wgP2Blue = false;

    // player mode: 0 = both, 1 = player 1, 2 = player 2
    int greenPlayer = 0, maintainPlayer = 0, flyPlayer = 0, ufoPlayer = 0, noclipPlayer = 0;
    int acP1Every = 10, acP1Hold = 1, acP2Every = 10, acP2Hold = 1;
    int linkMode = 0;   // 0 = copy, 1 = invert
    int linkDrive = 0;  // 0 = P1 drives, 1 = P2 drives

    float flyThr = 1.f, ufoThr = 5.f, ufoY = 150.f;
    float wgP1Thr = 3.f, wgP1Center = 150.f, wgP2Thr = 3.f, wgP2Center = 150.f;

    void load() {
#define L(n) n = geode::Mod::get()->getSavedValue<decltype(n)>(#n, n);
        CFG_FIELDS(L)
#undef L
    }
    void save() {
#define S(n) geode::Mod::get()->setSavedValue(#n, n);
        CFG_FIELDS(S)
#undef S
    }
};

inline Cfg cfg;
