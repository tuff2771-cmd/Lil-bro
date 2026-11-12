#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include "Settings.hpp"

using namespace geode::prelude;

struct Field {
    enum Kind { Cycle, Float, Int, Toggle } kind;
    std::string label;
    void* ptr;
    std::vector<std::string> options;
};
struct Feature {
    std::string name;
    bool* on;
    std::vector<Field> fields;
};

inline const std::vector<std::string> PLAYERS = {"Both", "Player 1", "Player 2"};
inline Field cycleF(std::string l, int* p, std::vector<std::string> o = PLAYERS) { return {Field::Cycle, l, p, o}; }
inline Field floatF(std::string l, float* p) { return {Field::Float, l, p, {}}; }
inline Field intF(std::string l, int* p) { return {Field::Int, l, p, {}}; }
inline Field boolF(std::string l, bool* p) { return {Field::Toggle, l, p, {}}; }

inline std::vector<Feature>& features() {
    static std::vector<Feature> f = {
        {"Click Green Dash Orbs", &cfg.greenDash, {cycleF("Player", &cfg.greenPlayer)}},
        {"No Orb Effects", &cfg.noOrbFx, {}},
        {"Click Black Orbs", &cfg.blackOrb, {}},
        {"Easy Click Black Orb UFO", &cfg.easyBlackUfo, {}},
        {"Click Jump Pads", &cfg.jumpPads, {}},
        {"Click Gravity Pads", &cfg.gravPads, {}},
        {"Maintain Gravity", &cfg.maintainGrav, {cycleF("Player", &cfg.maintainPlayer)}},
        {"Auto Straight Fly", &cfg.straightFly, {cycleF("Player", &cfg.flyPlayer), floatF("Threshold", &cfg.flyThr)}},
        {"Auto Straight UFO", &cfg.straightUfo, {cycleF("Player", &cfg.ufoPlayer), floatF("Threshold", &cfg.ufoThr), floatF("Y Position Target", &cfg.ufoY)}},
        {"AutoClicker Player 1", &cfg.acP1, {intF("Click every (frames)", &cfg.acP1Every), intF("Hold for (frames)", &cfg.acP1Hold), boolF("Swift", &cfg.acP1Swift)}},
        {"AutoClicker Player 2", &cfg.acP2, {intF("Click every (frames)", &cfg.acP2Every), intF("Hold for (frames)", &cfg.acP2Hold), boolF("Swift", &cfg.acP2Swift)}},
        {"Noclip", &cfg.noclip, {cycleF("Player", &cfg.noclipPlayer)}},
        {"Auto Swift", &cfg.autoSwift, {}},
        {"2P Control Link", &cfg.link2p, {cycleF("Mode", &cfg.linkMode, {"Copy", "Invert"}), cycleF("Driving side", &cfg.linkDrive, {"Player 1", "Player 2"})}},
        {"Moving Wave Gap Spam", &cfg.waveGap, {
            boolF("Player 1", &cfg.wgP1On), floatF("P1 Threshold", &cfg.wgP1Thr),
            floatF("P1 Recenter (below)", &cfg.wgP1Center), boolF("P1 Blue Orb", &cfg.wgP1Blue),
            boolF("Player 2", &cfg.wgP2On), floatF("P2 Threshold", &cfg.wgP2Thr),
            floatF("P2 Recenter (below)", &cfg.wgP2Center), boolF("P2 Blue Orb", &cfg.wgP2Blue)}},
    };
    return f;
}

class ConfigPopup : public Popup<Feature*> {
protected:
    bool setup(Feature* ft) override {
        m_bgSprite->setColor({0, 0, 0});
        this->setTitle(ft->name, "bigFont.fnt", .5f);
        auto sz = m_mainLayer->getContentSize();
        float y = sz.height - 50.f;
        float cx = sz.width - 70.f;

        for (auto& fd : ft->fields) {
            Field* f = &fd;
            auto lbl = CCLabelBMFont::create(f->label.c_str(), "bigFont.fnt");
            lbl->setAnchorPoint({0, .5f});
            lbl->limitLabelWidth(150.f, .45f, .2f);
            lbl->setPosition(15.f, y);
            m_mainLayer->addChild(lbl);

            if (f->kind == Field::Cycle) {
                int& v = *(int*)f->ptr;
                v = std::clamp(v, 0, (int)f->options.size() - 1);
                auto spr = ButtonSprite::create(f->options[v].c_str(), 90, true, "bigFont.fnt", "GJ_button_01.png", 26.f, .5f);
                auto btn = CCMenuItemExt::createSpriteExtra(spr, [f, spr](CCMenuItemSpriteExtra*) {
                    int& v = *(int*)f->ptr;
                    v = (v + 1) % (int)f->options.size();
                    spr->setString(f->options[v].c_str());
                    cfg.save();
                });
                auto menu = CCMenu::create();
                menu->setPosition({cx, y});
                menu->addChild(btn);
                m_mainLayer->addChild(menu);
            } else if (f->kind == Field::Toggle) {
                auto tog = CCMenuItemExt::createTogglerWithStandardSprites(.6f, [f](CCMenuItemToggler* t) {
                    *(bool*)f->ptr = !t->isToggled();
                    cfg.save();
                });
                tog->toggle(*(bool*)f->ptr);
                auto menu = CCMenu::create();
                menu->setPosition({cx, y});
                menu->addChild(tog);
                m_mainLayer->addChild(menu);
            } else {
                auto in = TextInput::create(80.f, "0", "bigFont.fnt");
                bool isInt = f->kind == Field::Int;
                in->setCommonFilter(isInt ? CommonFilter::Int : CommonFilter::Float);
                in->setString(isInt ? fmt::format("{}", *(int*)f->ptr) : fmt::format("{}", *(float*)f->ptr));
                in->setCallback([f, isInt](std::string const& s) {
                    if (isInt) {
                        if (auto r = numFromString<int>(s)) *(int*)f->ptr = r.unwrap();
                    } else {
                        if (auto r = numFromString<float>(s)) *(float*)f->ptr = r.unwrap();
                    }
                    cfg.save();
                });
                in->setPosition({cx, y});
                m_mainLayer->addChild(in);
            }
            y -= 28.f;
        }
        return true;
    }

public:
    static ConfigPopup* create(Feature* ft) {
        auto r = new ConfigPopup();
        if (r->initAnchored(300.f, 60.f + 28.f * ft->fields.size(), ft)) {
            r->autorelease();
            return r;
        }
        delete r;
        return nullptr;
    }
};

class CheatPopup : public Popup<> {
protected:
    CCNode* makeRow(Feature* f, float w) {
        auto row = CCNode::create();
        row->setContentSize({w, 30.f});
        auto menu = CCMenu::create();
        menu->setContentSize({w, 30.f});
        menu->setPosition({0, 0});
        row->addChild(menu);

        auto tog = CCMenuItemExt::createTogglerWithStandardSprites(.6f, [f](CCMenuItemToggler* t) {
            *f->on = !t->isToggled();
            cfg.save();
        });
        tog->toggle(*f->on);
        tog->setPosition({18.f, 15.f});
        menu->addChild(tog);

        auto lbl = CCLabelBMFont::create(f->name.c_str(), "bigFont.fnt");
        lbl->setAnchorPoint({0, .5f});
        lbl->limitLabelWidth(w - 100.f, .5f, .2f);
        lbl->setPosition({38.f, 15.f});
        row->addChild(lbl);

        if (!f->fields.empty()) {
            auto gs = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
            gs->setScale(.6f);
            auto gear = CCMenuItemExt::createSpriteExtra(gs, [f](CCMenuItemSpriteExtra*) {
                if (auto p = ConfigPopup::create(f)) p->show();
            });
            gear->setPosition({w - 22.f, 15.f});
            menu->addChild(gear);
        }
        return row;
    }

    bool setup() override {
        m_bgSprite->setColor({0, 0, 0});
        this->setTitle("Cheat Menu", "bigFont.fnt", .7f);
        auto sz = m_mainLayer->getContentSize();

        auto sec = CCLabelBMFont::create("Gameplay", "bigFont.fnt");
        sec->setScale(.55f);
        sec->setPosition({sz.width / 2, sz.height - 45.f});
        m_mainLayer->addChild(sec);

        float lw = sz.width - 40.f, lh = sz.height - 85.f;
        auto scroll = ScrollLayer::create({lw, lh});
        scroll->setPosition({20.f, 20.f});
        m_mainLayer->addChild(scroll);

        scroll->m_contentLayer->setLayout(
            ColumnLayout::create()
                ->setAxisReverse(true)
                ->setAxisAlignment(AxisAlignment::End)
                ->setAutoGrowAxis(lh)
                ->setGap(2.f));
        for (auto& f : features()) scroll->m_contentLayer->addChild(makeRow(&f, lw));
        scroll->m_contentLayer->updateLayout();
        return true;
    }

public:
    static CheatPopup* create() {
        auto r = new CheatPopup();
        if (r->initAnchored(360.f, 270.f)) {
            r->autorelease();
            return r;
        }
        delete r;
        return nullptr;
    }
};
