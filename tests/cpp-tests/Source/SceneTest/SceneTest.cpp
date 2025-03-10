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

#include "SceneTest.h"
#include "../testResource.h"

using namespace ax;

SceneTests::SceneTests()
{
    ADD_TEST_CASE(SceneTestScene);
}

//------------------------------------------------------------------
//
// SceneTestLayer1
//
//------------------------------------------------------------------
enum
{
    MID_PUSHSCENE = 100,
    MID_PUSHSCENETRAN,
    MID_QUIT,
    MID_REPLACESCENE,
    MID_REPLACESCENETRAN,
    MID_GOBACK
};

SceneTestLayer1::SceneTestLayer1()
{
    auto item1 = MenuItemFont::create("Test pushScene", AX_CALLBACK_1(SceneTestLayer1::onPushScene, this));
    auto item2 =
        MenuItemFont::create("Test pushScene w/transition", AX_CALLBACK_1(SceneTestLayer1::onPushSceneTran, this));
    auto item3 = MenuItemFont::create("Quit", AX_CALLBACK_1(SceneTestLayer1::onQuit, this));

    auto menu = Menu::create(item1, item2, item3, nullptr);
    menu->alignItemsVertically();

    addChild(menu);

    auto s      = Director::getInstance()->getWinSize();
    auto sprite = Sprite::create(s_pathGrossini);
    addChild(sprite);
    sprite->setPosition(Vec2(s.width - 40, s.height / 2));
    auto rotate = RotateBy::create(2, 360);
    auto repeat = RepeatForever::create(rotate);
    sprite->runAction(repeat);

    schedule(AX_SCHEDULE_SELECTOR(SceneTestLayer1::testDealloc));
}

void SceneTestLayer1::testDealloc(float dt)
{
    // AXLOGD("SceneTestLayer1:testDealloc");
}

void SceneTestLayer1::onEnter()
{
    AXLOGD("SceneTestLayer1#onEnter");
    Layer::onEnter();
}

void SceneTestLayer1::onEnterTransitionDidFinish()
{
    AXLOGD("SceneTestLayer1#onEnterTransitionDidFinish");
    Layer::onEnterTransitionDidFinish();
}

SceneTestLayer1::~SceneTestLayer1()
{
    // NSLog(@"SceneTestLayer1 - dealloc");
}

void SceneTestLayer1::onPushScene(Object* sender)
{
    auto scene = SceneTestScene::create(2);
    Director::getInstance()->pushScene(scene);
}

void SceneTestLayer1::onPushSceneTran(Object* sender)
{
    auto scene = SceneTestScene::create(2);
    Director::getInstance()->pushScene(TransitionSlideInT::create(1, scene));
}

void SceneTestLayer1::onQuit(Object* sender)
{
    // getCocosApp()->exit();
    // Director::getInstance()->popScene();

    //// HA HA... no more terminate on sdk v3.0
    //// http://developer.apple.com/iphone/library/qa/qa2008/qa1561.html
    // if( [[UIApplication sharedApplication] respondsToSelector:@selector(terminate)] )
    //     [[UIApplication sharedApplication] performSelector:@selector(terminate)];
}

//------------------------------------------------------------------
//
// SceneTestLayer2
//
//------------------------------------------------------------------

SceneTestLayer2::SceneTestLayer2()
{
    _timeCounter = 0;

    auto item1 = MenuItemFont::create("replaceScene", AX_CALLBACK_1(SceneTestLayer2::onReplaceScene, this));
    auto item2 =
        MenuItemFont::create("replaceScene w/transition", AX_CALLBACK_1(SceneTestLayer2::onReplaceSceneTran, this));
    auto item3 = MenuItemFont::create("Go Back", AX_CALLBACK_1(SceneTestLayer2::onGoBack, this));

    auto menu = Menu::create(item1, item2, item3, nullptr);
    menu->alignItemsVertically();

    addChild(menu);

    auto s      = Director::getInstance()->getWinSize();
    auto sprite = Sprite::create(s_pathGrossini);
    addChild(sprite);
    sprite->setPosition(Vec2(s.width - 40, s.height / 2));
    auto rotate = RotateBy::create(2, 360);
    auto repeat = RepeatForever::create(rotate);
    sprite->runAction(repeat);

    schedule(AX_SCHEDULE_SELECTOR(SceneTestLayer2::testDealloc));
}

void SceneTestLayer2::testDealloc(float dt)
{
    //_timeCounter += dt;
    // if( _timeCounter > 10 )
    //    onReplaceScene(this);
}

void SceneTestLayer2::onGoBack(Object* sender)
{
    Director::getInstance()->popScene();
}

void SceneTestLayer2::onReplaceScene(Object* sender)
{
    auto scene = SceneTestScene::create(3);
    Director::getInstance()->replaceScene(scene);
}

void SceneTestLayer2::onReplaceSceneTran(Object* sender)
{
    auto scene = SceneTestScene::create(3);
    Director::getInstance()->replaceScene(TransitionFlipX::create(2, scene));
}

//------------------------------------------------------------------
//
// SceneTestLayer3
//
//------------------------------------------------------------------

SceneTestLayer3::SceneTestLayer3() {}

bool SceneTestLayer3::init()
{
    if (LayerColor::initWithColor(Color4B(0, 0, 255, 255)))
    {
        auto s = Director::getInstance()->getWinSize();

        auto item0 =
            MenuItemFont::create("Touch to pushScene (self)", AX_CALLBACK_1(SceneTestLayer3::item0Clicked, this));
        auto item1 = MenuItemFont::create("Touch to popScene", AX_CALLBACK_1(SceneTestLayer3::item1Clicked, this));
        auto item2 =
            MenuItemFont::create("Touch to popToRootScene", AX_CALLBACK_1(SceneTestLayer3::item2Clicked, this));
        auto item3 = MenuItemFont::create("Touch to popToSceneStackLevel(2)",
                                          AX_CALLBACK_1(SceneTestLayer3::item3Clicked, this));

        auto menu = Menu::create(item0, item1, item2, item3, nullptr);
        this->addChild(menu);
        menu->alignItemsVertically();

        this->schedule(AX_SCHEDULE_SELECTOR(SceneTestLayer3::testDealloc));

        auto sprite = Sprite::create(s_pathGrossini);
        addChild(sprite);
        sprite->setPosition(Vec2(s.width / 2, 40.0f));
        auto rotate = RotateBy::create(2, 360);
        auto repeat = RepeatForever::create(rotate);
        sprite->runAction(repeat);
        return true;
    }
    return false;
}

void SceneTestLayer3::testDealloc(float dt)
{
    AXLOGI("Layer3:testDealloc");
}

void SceneTestLayer3::item0Clicked(Object* sender)
{
    auto s        = Director::getInstance()->getWinSize();
    auto newScene = Scene::createWithSize(s);
    newScene->addChild(SceneTestLayer3::create());
    Director::getInstance()->pushScene(TransitionFade::create(0.5, newScene, Color3B(0, 255, 255)));
}

void SceneTestLayer3::item1Clicked(Object* sender)
{
    Director::getInstance()->popScene();
}

void SceneTestLayer3::item2Clicked(Object* sender)
{
    Director::getInstance()->popToRootScene();
}

void SceneTestLayer3::item3Clicked(Object* sender)
{
    Director::getInstance()->popToSceneStackLevel(2);
}

SceneTestScene* SceneTestScene::create(int testIndex /* = 1 */)
{
    auto scene = new SceneTestScene;
    if (scene->init())
    {
        scene->autorelease();
        switch (testIndex)
        {
        case 1:
            scene->addChild(SceneTestLayer1::create());
            break;
        case 2:
            scene->addChild(SceneTestLayer2::create());
            break;
        case 3:
            scene->addChild(SceneTestLayer3::create());
            break;
        default:
            break;
        }
    }
    else
    {
        delete scene;
        scene = nullptr;
    }

    return scene;
}
