#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include "Menu.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    cfg.load();
}

class $modify(CheatPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        // add one frame later so other mods' pause buttons are already in place
        this->scheduleOnce(schedule_selector(CheatPause::addCheatButton), 0.f);
    }

    void addCheatButton(float) {
        if (this->getChildByIDRecursive("cheat-menu-button")) return;

        auto lbl = CCLabelBMFont::create("HU", "bigFont.fnt");
        lbl->setScale(.55f);
        auto spr = CircleButtonSprite::create(lbl, CircleBaseColor::Green, CircleBaseSize::Medium);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [](CCMenuItemSpriteExtra*) {
            if (auto p = CheatPopup::create()) p->show();
        });
        btn->setID("cheat-menu-button");

        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            auto m = CCMenu::create();
            m->setID("cheat-menu");
            m->setPosition({0.f, 0.f});
            auto w = CCDirector::get()->getWinSize();
            btn->setPosition({w.width - 30.f, 40.f});
            m->addChild(btn);
            this->addChild(m);
        }
    }
};
