#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "Menu.hpp"

$on_mod(Loaded) {
    cfg.load();
}

class $modify(CheatPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto spr = ButtonSprite::create("Cheats", 60, true, "bigFont.fnt", "GJ_button_01.png", 28.f, .6f);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [](CCMenuItemSpriteExtra*) {
            if (auto p = CheatPopup::create()) p->show();
        });

        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            auto m = CCMenu::create();
            m->addChild(btn);
            m->setPosition(CCDirector::get()->getWinSize().width - 50.f, 40.f);
            this->addChild(m);
        }
    }
};
