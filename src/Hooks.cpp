#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/RingObject.hpp>
#include "Settings.hpp"

using namespace geode::prelude;

static bool pick(int mode, bool p1) {
    return mode == 0 || (mode == 1 && p1) || (mode == 2 && !p1);
}

static int touching(PlayerObject* p, GameObjectType t) {
    int n = 0;
    if (!p || !p->m_touchingRings) return 0;
    for (auto* o : CCArrayExt<GameObject*>(p->m_touchingRings))
        if (o && o->m_objectType == t) n++;
    return n;
}

// "swift click" = press + release in the same tick
static void swift(GJBaseGameLayer* l, bool p1, int n = 1) {
    for (int i = 0; i < n; i++) {
        l->handleButton(true, 1, p1);
        l->handleButton(false, 1, p1);
    }
}

class $modify(CheatGJ, GJBaseGameLayer) {
    struct Fields {
        int tick = 0;
        bool held[2] = {false, false};
        bool flip[2] = {false, false};
        bool gravSet[2] = {false, false};
        bool wantUp[2] = {false, false};
    };

    bool active() {
        auto pl = PlayLayer::get();
        return pl && static_cast<GJBaseGameLayer*>(pl) == this && pl->m_player1 && !pl->m_player1->m_isDead;
    }

    void setHold(bool p1, bool want) {
        bool& h = m_fields->held[p1 ? 0 : 1];
        if (h == want) return;
        h = want;
        this->handleButton(want, 1, p1);
    }

    float gapMid(PlayerObject* p, float fallback) {
        float px = p->getPositionX(), py = p->getPositionY();
        float lo = -1e9f, hi = 1e9f;
        bool gotLo = false, gotHi = false;
        int s = std::max(0, (int)(px / 100.f));
        for (int k = s; k <= s + 1; k++) {
            if (k < 0 || k >= (int)m_sections.size()) continue;
            auto* v = m_sections[k];
            if (!v) continue;
            for (auto* o : *v) {
                if (!o || o->m_objectType != GameObjectType::Solid) continue;
                auto r = o->getObjectRect();
                if (r.getMaxX() < px || r.getMinX() > px + 120.f) continue;
                if (r.getMaxY() <= py + 2.f) {
                    if (r.getMaxY() > lo) { lo = r.getMaxY(); gotLo = true; }
                } else if (r.getMinY() >= py - 2.f) {
                    if (r.getMinY() < hi) { hi = r.getMinY(); gotHi = true; }
                }
            }
        }
        return (gotLo && gotHi) ? (lo + hi) * .5f : fallback;
    }

    void cheats() {
        int tick = ++m_fields->tick;
        for (int i = 0; i < 2; i++) {
            bool p1 = i == 0;
            PlayerObject* p = p1 ? m_player1 : m_player2;
            if (!p || (!p1 && !m_gameState.m_isDualMode)) continue;
            float vy = p->m_yVelocity * (p->m_isUpsideDown ? -1.f : 1.f);

            // green dash orbs (more orbs = more clicks)
            if (cfg.greenDash && pick(cfg.greenPlayer, p1)) {
                int n = touching(p, GameObjectType::DashRing);
                if (n) swift(this, p1, std::min(n, 6));
            }

            // black orbs / easy black orb ufo
            if (cfg.blackOrb || cfg.easyBlackUfo) {
                int n = touching(p, GameObjectType::DropRing);
                bool skip = cfg.easyBlackUfo && p->m_isBird && vy < 0;
                if (n && !skip) swift(this, p1);
            }

            // maintain gravity
            if (cfg.maintainGrav && pick(cfg.maintainPlayer, p1)) {
                bool holding = p->m_holdingButtons[1];
                if (!m_fields->gravSet[i] || holding) {
                    m_fields->wantUp[i] = p->m_isUpsideDown;
                    m_fields->gravSet[i] = true;
                } else if (p->m_isUpsideDown != m_fields->wantUp[i]) {
                    p->flipGravity(m_fields->wantUp[i], true);
                }
            } else {
                m_fields->gravSet[i] = false;
            }

            // auto straight fly (ship)
            if (cfg.straightFly && p->m_isShip && pick(cfg.flyPlayer, p1)) {
                if (vy < -cfg.flyThr) setHold(p1, true);
                else if (vy > cfg.flyThr) setHold(p1, false);
            }

            // auto straight ufo
            if (cfg.straightUfo && p->m_isBird && pick(cfg.ufoPlayer, p1)) {
                float err = cfg.ufoY - p->getPositionY();
                if (p->m_isUpsideDown) err = -err;
                if (err > cfg.ufoThr) swift(this, p1);
            }

            // autoclickers
            bool acOn = p1 ? cfg.acP1 : cfg.acP2;
            if (acOn) {
                int every = std::max(1, p1 ? cfg.acP1Every : cfg.acP2Every);
                int hold = std::clamp(p1 ? cfg.acP1Hold : cfg.acP2Hold, 1, every);
                bool sw = p1 ? cfg.acP1Swift : cfg.acP2Swift;
                int c = tick % every;
                if (sw) { if (c == 0) swift(this, p1); }
                else setHold(p1, c < hold);
            }

            // auto swift
            if (cfg.autoSwift) swift(this, p1);

            // moving wave gap spam
            bool wgOn = p1 ? cfg.wgP1On : cfg.wgP2On;
            if (cfg.waveGap && wgOn && p->m_isDart) {
                float thr = p1 ? cfg.wgP1Thr : cfg.wgP2Thr;
                float center = p1 ? cfg.wgP1Center : cfg.wgP2Center;
                bool blue = p1 ? cfg.wgP1Blue : cfg.wgP2Blue;
                if (blue && touching(p, GameObjectType::GravityRing)) swift(this, p1);

                float mid = this->gapMid(p, center);
                float y = p->getPositionY();
                bool up = y < mid - thr, down = y > mid + thr;
                if (p->m_isUpsideDown) std::swap(up, down);
                if (up) setHold(p1, true);
                else if (down) setHold(p1, false);
                else {
                    m_fields->flip[i] = !m_fields->flip[i];
                    setHold(p1, m_fields->flip[i]);
                }
            }
        }
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        if (this->active()) this->cheats();
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }

    // 2P control link
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
        static bool busy = false;
        if (busy || !cfg.link2p || !this->active()) return;
        bool driveP1 = cfg.linkDrive == 0;
        if (isPlayer1 != driveP1) return;
        busy = true;
        GJBaseGameLayer::handleButton(cfg.linkMode == 0 ? down : !down, button, !isPlayer1);
        busy = false;
    }
};

// pads
class $modify(CheatPlayer, PlayerObject) {
    void collidedWithObject(float dt, GameObject* obj, cocos2d::CCRect rect, bool b) {
        PlayerObject::collidedWithObject(dt, obj, rect, b);
        auto pl = PlayLayer::get();
        if (!pl || !obj || !pl->m_player1) return;
        bool p1 = this == pl->m_player1;
        if (!p1 && this != pl->m_player2) return;
        auto t = obj->m_objectType;
        bool jump = t == GameObjectType::YellowJumpPad || t == GameObjectType::PinkJumpPad || t == GameObjectType::RedJumpPad;
        if ((cfg.jumpPads && jump) || (cfg.gravPads && t == GameObjectType::GravityPad))
            swift(pl, p1);
    }
};

// noclip
class $modify(CheatPlay, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (obj != m_anticheatSpike && cfg.noclip && pick(cfg.noclipPlayer, player == m_player1))
            return;
        PlayLayer::destroyPlayer(player, obj);
    }
};

// no orb effects
class $modify(CheatRing, RingObject) {
    void spawnCircle() {
        if (cfg.noOrbFx) return;
        RingObject::spawnCircle();
    }
};
