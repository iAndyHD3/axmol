/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

//
//  Bug-14327.cpp
//  cocos2d_tests
//
//  Issue: https://github.com/cocos2d/cocos2d-x/pull/14327
//  Please test in Windows
//
//

#include "Bug-14327.h"

#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)

using namespace ax;

bool Bug14327Layer::init()
{
    if (BugsTestBase::init())
    {
        auto glView        = Director::getInstance()->getGLView();
        auto visibleOrigin = glView->getVisibleOrigin();
        auto visibleSize   = glView->getVisibleSize();

        auto pBg = Sprite::create("Images/HelloWorld.png");
        pBg->setPosition(Vec2(visibleOrigin.x + visibleSize.width / 2, visibleOrigin.y + visibleSize.height / 2));
        addChild(pBg);

        _removeTime = time(nullptr) + 20;

        _TTFShowTime = Label::createWithSystemFont("Edit control will be removed after 00:20!", "Arial", 20);
        _TTFShowTime->setPosition(
            Vec2(visibleOrigin.x + visibleSize.width / 2, visibleOrigin.y + visibleSize.height - 60));
        this->addChild(_TTFShowTime);

        auto editBoxSize = Size(visibleSize.width - 100, visibleSize.height * 0.1);

        std::string pNormalSprite = "extensions/green_edit.png";
        _edit = ui::EditBox::create(editBoxSize + Size(0, 20), ui::Scale9Sprite::create(pNormalSprite));
        _edit->setPosition(Vec2(visibleOrigin.x + visibleSize.width / 2, visibleOrigin.y + visibleSize.height / 2));
        _edit->setFontColor(Color3B::RED);
        _edit->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
        _edit->setDelegate(this);
        this->addChild(_edit);

        this->scheduleUpdate();
        return true;
    }

    return false;
}

void Bug14327Layer::update(float dt)
{
    int32_t delta = _removeTime - time(nullptr);
    if (delta > 0)
    {
        ldiv_t ret = ldiv(delta, 60L);
        char str[100];
        snprintf(str, 100, "%s%.2ld:%.2ld", "Edit control will be removed after ", ret.quot, ret.rem);
        _TTFShowTime->setString(str);
    }
    else
    {
        _edit->removeFromParent();
        _edit = nullptr;
        _TTFShowTime->setString("Edit control has been removed!\nIt should not crash.");
        this->unscheduleUpdate();
    }
}

void Bug14327Layer::editBoxEditingDidBegin(ax::ui::EditBox* editBox)
{
    AXLOGD("editBox {} DidBegin !", fmt::ptr(editBox));
}

void Bug14327Layer::editBoxEditingDidEndWithAction(ax::ui::EditBox* editBox,
                                                   ax::ui::EditBoxDelegate::EditBoxEndAction EditBoxEndAction)
{
    AXLOGD("editBox {} DidEnd !", fmt::ptr(editBox));
}

void Bug14327Layer::editBoxTextChanged(ax::ui::EditBox* editBox, std::string_view text)
{
    AXLOGD("editBox {} TextChanged, text: {} ", fmt::ptr(editBox), text);
}

void Bug14327Layer::editBoxReturn(ui::EditBox* editBox)
{
    AXLOGD("editBox {} was returned !", fmt::ptr(editBox));
}

#endif
