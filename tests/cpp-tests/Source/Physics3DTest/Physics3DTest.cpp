/****************************************************************************
 Copyright (c) 2012 cocos2d-x.org
 Copyright (c) 2015-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

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

#include "Physics3DTest.h"

#include "3d/Terrain.h"
#include "3d/Bundle3D.h"
#include "physics3d/Physics3D.h"
#include "Particle3D/PU/PUParticleSystem3D.h"
USING_NS_AX_EXT;
using namespace ax;

static const std::string_view boss[] = {"MeshRendererTest/boss.c3b", "MeshRendererTest/boss.obj"};
static const int boss_count          = sizeof(boss) / sizeof(boss[0]);

enum
{
    IDC_NEXT = 100,
    IDC_BACK,
    IDC_RESTART
};

#define START_POS_X -0.5
#define START_POS_Y -2.5
#define START_POS_Z -0.5

#define ARRAY_SIZE_X 4
#define ARRAY_SIZE_Y 3
#define ARRAY_SIZE_Z 4

static ax::Scene* physicsScene = nullptr;

Physics3DTests::Physics3DTests()
{
#if !defined(AX_ENABLE_3D_PHYSICS)
    ADD_TEST_CASE(Physics3DDemoDisabled);
#else
    ADD_TEST_CASE(BasicPhysics3DDemo);
    ADD_TEST_CASE(Physics3DConstraintDemo);
    ADD_TEST_CASE(Physics3DKinematicDemo);
    ADD_TEST_CASE(Physics3DCollisionCallbackDemo);
    ADD_TEST_CASE(Physics3DColliderDemo);
    ADD_TEST_CASE(Physics3DTerrainDemo);
#endif
};

#if !defined(AX_ENABLE_3D_PHYSICS)
void Physics3DDemoDisabled::onEnter()
{
    TTFConfig ttfConfig("fonts/arial.ttf", 16);
    auto label = Label::createWithTTF(ttfConfig, "Should enable AX_ENABLE_3D_PHYSICS\n to run this test case");

    auto size = Director::getInstance()->getWinSize();
    label->setPosition(Vec2(size.width / 2, size.height / 2));

    addChild(label);

    TestCase::onEnter();
}
#else
std::string Physics3DTestDemo::title() const
{
    return "Physics3D Test";
}

std::string Physics3DTestDemo::subtitle() const
{
    return "";
}

bool Physics3DTestDemo::init()
{
    if (!TestCase::init())
        return false;

    if (initWithPhysics())
    {
        getPhysics3DWorld()->setDebugDrawEnable(false);

        physicsScene = this;
        Size size = Director::getInstance()->getWinSize();
        _camera = Camera::createPerspective(30.0f, size.width / size.height, 1.0f, 1000.0f);
        _camera->setPosition3D(Vec3(0.0f, 50.0f, 100.0f));
        _camera->lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
        _camera->setCameraFlag(CameraFlag::USER1);
        this->addChild(_camera);

        auto listener = EventListenerTouchAllAtOnce::create();
        listener->onTouchesBegan = AX_CALLBACK_2(Physics3DTestDemo::onTouchesBegan, this);
        listener->onTouchesMoved = AX_CALLBACK_2(Physics3DTestDemo::onTouchesMoved, this);
        listener->onTouchesEnded = AX_CALLBACK_2(Physics3DTestDemo::onTouchesEnded, this);
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

        TTFConfig ttfConfig("fonts/arial.ttf", 10);
        auto label = Label::createWithTTF(ttfConfig, "DebugDraw OFF");
        auto menuItem = MenuItemLabel::create(label, [this, label](Object* /*sender*/) {
            if (getPhysics3DWorld()->isDebugDrawEnabled())
            {
                getPhysics3DWorld()->setDebugDrawEnable(false);
                label->setString("DebugDraw OFF");
            }
            else
            {
                getPhysics3DWorld()->setDebugDrawEnable(true);
                label->setString("DebugDraw ON");
            }
        });

        auto menu = Menu::create(menuItem, nullptr);
        menu->setPosition(Vec2::ZERO);
        menuItem->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
        menuItem->setPosition(Vec2(VisibleRect::left().x, VisibleRect::top().y - 50));
        this->addChild(menu);

        _angle = 0.0f;
        return true;
    }
    physicsScene = nullptr;
    return false;
}

void Physics3DTestDemo::onTouchesBegan(const std::vector<Touch*>& touches, ax::Event* event)
{
    _needShootBox = true;
    event->stopPropagation();
}

void Physics3DTestDemo::onTouchesMoved(const std::vector<Touch*>& touches, ax::Event* event)
{
    if (touches.size() && _camera)
    {
        auto touch = touches[0];
        auto delta = touch->getDelta();

        _angle -= AX_DEGREES_TO_RADIANS(delta.x);
        _camera->setPosition3D(Vec3(100.0f * sinf(_angle), 50.0f, 100.0f * cosf(_angle)));
        _camera->lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

        if (delta.lengthSquared() > 16)
        {
            _needShootBox = false;
        }
        event->stopPropagation();
    }
}

void Physics3DTestDemo::onTouchesEnded(const std::vector<Touch*>& touches, ax::Event* event)
{
    if (!_needShootBox)
        return;
    if (!touches.empty())
    {
        auto location = touches[0]->getLocationInView();

        Vec3 nearP(location.x, location.y, -1.0f), farP(location.x, location.y, 1.0f);
        nearP = _camera->unproject(nearP);
        farP = _camera->unproject(farP);
        Vec3 dir(farP - nearP);
        shootBox(_camera->getPosition3D() + dir * 10.0f);
        event->stopPropagation();
    }
}

Physics3DTestDemo::Physics3DTestDemo() {}

void Physics3DTestDemo::update(float /*delta*/) {}

Physics3DTestDemo::~Physics3DTestDemo() {}

void Physics3DTestDemo::shootBox(const ax::Vec3& des)
{
    Physics3DRigidBodyDes rbDes;
    Vec3 linearVel = des - _camera->getPosition3D();
    linearVel.normalize();
    linearVel *= 100.0f;
    rbDes.originalTransform.translate(_camera->getPosition3D());
    rbDes.mass = 1.f;
    rbDes.shape = Physics3DShape::createBox(Vec3(0.5f, 0.5f, 0.5f));
    auto mesh = PhysicsMeshRenderer::create("MeshRendererTest/box.c3t", &rbDes);
    mesh->setTexture("Images/Icon.png");

    auto rigidBody = static_cast<Physics3DRigidBody*>(mesh->getPhysicsObj());
    rigidBody->setLinearFactor(Vec3::ONE);
    rigidBody->setLinearVelocity(linearVel);
    rigidBody->setAngularVelocity(Vec3::ZERO);
    rigidBody->setCcdMotionThreshold(0.5f);
    rigidBody->setCcdSweptSphereRadius(0.4f);

    this->addChild(mesh);
    mesh->setPosition3D(_camera->getPosition3D());
    mesh->setScale(0.5f);
    mesh->syncNodeToPhysics();

    // optimize, only sync node to physics
    mesh->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::PHYSICS_TO_NODE);  // sync node to physics

    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
}

std::string BasicPhysics3DDemo::subtitle() const
{
    return "Basic Physics3D";
}

bool BasicPhysics3DDemo::init()
{
    if (!Physics3DTestDemo::init())
        return false;

    // create floor
    Physics3DRigidBodyDes rbDes;
    rbDes.mass = 0.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(60.0f, 1.0f, 60.0f));

    auto floor = PhysicsMeshRenderer::create("MeshRendererTest/box.c3t", &rbDes);
    floor->setTexture("MeshRendererTest/plane.png");
    floor->setScaleX(60);
    floor->setScaleZ(60);
    this->addChild(floor);
    floor->setCameraMask((unsigned short)CameraFlag::USER1);
    floor->syncNodeToPhysics();
    // static object sync is not needed
    floor->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::NONE);

    // create several boxes using PhysicsMeshRenderer
    rbDes.mass = 1.f;
    rbDes.shape = Physics3DShape::createBox(Vec3(0.8f, 0.8f, 0.8f));
    float start_x = START_POS_X - ARRAY_SIZE_X / 2;
    float start_y = START_POS_Y;
    float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

    for (int k = 0; k < ARRAY_SIZE_Y; k++)
    {
        for (int i = 0; i < ARRAY_SIZE_X; i++)
        {
            for (int j = 0; j < ARRAY_SIZE_Z; j++)
            {
                float x = 1.0 * i + start_x;
                float y = 5.0 + 1.0 * k + start_y;
                float z = 1.0 * j + start_z;

                auto mesh = PhysicsMeshRenderer::create("MeshRendererTest/box.c3t", &rbDes);
                mesh->setTexture("Images/CyanSquare.png");
                mesh->setPosition3D(Vec3(x, y, z));
                mesh->syncNodeToPhysics();
                mesh->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::PHYSICS_TO_NODE);
                mesh->setCameraMask((unsigned short)CameraFlag::USER1);
                mesh->setScale(0.8f);
                this->addChild(mesh);
            }
        }
    }

    physicsScene->setPhysics3DDebugCamera(_camera);

    return true;
}

std::string Physics3DConstraintDemo::subtitle() const
{
    return "Physics3D Constraint";
}

std::string Physics3DKinematicDemo::subtitle() const
{
    return "Physics3D Kinematic + #879";
}

bool Physics3DKinematicDemo::init()
{
    if (!Physics3DTestDemo::init())
        return false;

    // create floor
    Physics3DRigidBodyDes rbDes;
    rbDes.mass = 0.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(60.0f, 1.0f, 60.0f));
    auto floor = PhysicsMeshRenderer::create("MeshRendererTest/box.c3t", &rbDes);
    floor->setTexture("MeshRendererTest/plane.png");
    floor->setScaleX(60);
    floor->setScaleZ(60);
    floor->setPosition3D(Vec3(0.f, -1.f, 0.f));
    this->addChild(floor);
    floor->setCameraMask((unsigned short)CameraFlag::USER1);
    floor->syncNodeToPhysics();
    // static object sync is not needed
    floor->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::NONE);

    // Issue #879 STARTS BELOW
    {
        Physics3DRigidBodyDes rbDes;
        std::string tree1 = "MeshRendererTest/tree-model/tree1.obj";

        float scale = 12.0f;
        std::vector<Vec3> trianglesList = Bundle3D::getTrianglesList(tree1);
        for (auto& it : trianglesList)
        {
            it *= scale;
        }

        rbDes.mass = 0.0f;
        rbDes.shape = Physics3DShape::createMesh(&trianglesList[0], (int)trianglesList.size() / 3);
        auto rigidBody = Physics3DRigidBody::create(&rbDes);
        auto component = Physics3DComponent::create(rigidBody);
        auto sprite = MeshRenderer::create(tree1);
        sprite->addComponent(component);
        static_cast<MeshRenderer*>(sprite->getChildren().at(1))->getMaterial(0)->setTransparent(true);
        static_cast<MeshRenderer*>(sprite->getChildren().at(1))->getMaterial(0)->getStateBlock().setCullFaceSide(CullFaceSide::NONE);
        sprite->setCameraMask((unsigned short)CameraFlag::USER1 | (unsigned short)CameraFlag::USER2 | (unsigned short)CameraFlag::USER3);
        sprite->setPosition3D(Vec3(20.0f, 0.0f, 0.0f));
        sprite->setScale(scale);
        this->addChild(sprite);
    }
    // Issue #879 ENDS HERE


    // create Kinematics
    for (unsigned int i = 0; i < 3; ++i)
    {
        rbDes.mass = 0.f;  // kinematic objects. zero mass so that it can not be affected by other dynamic objects
        rbDes.shape = Physics3DShape::createBox(Vec3(2.0f, 2.0f, 2.0f));

        auto mesh = PhysicsMeshRenderer::create("MeshRendererTest/box.c3t", &rbDes);
        mesh->setTexture("Images/CyanSquare.png");
        mesh->setCameraMask((unsigned short)CameraFlag::USER1);
        auto rigidBody = static_cast<Physics3DRigidBody*>(mesh->getPhysicsObj());
        rigidBody->setKinematic(true);

        this->addChild(mesh);

        mesh->setScale(2.0f);
        mesh->setPosition3D(Vec3(-15.0f, 0.0f, 15.0f - 15.0f * i));
        auto moveby = MoveBy::create(2.0f + i, Vec3(30.0f, 0.0f, 0.0f));
        mesh->runAction(RepeatForever::create(Sequence::create(moveby, moveby->reverse(), nullptr)));
    }

    // create Dynamic
    {
        // create several spheres
        rbDes.mass = 1.f;
        rbDes.shape = Physics3DShape::createSphere(0.5f);
        float start_x = START_POS_X - ARRAY_SIZE_X / 2;
        float start_y = START_POS_Y + 5.0f;
        float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

        for (int k = 0; k < ARRAY_SIZE_Y; k++)
        {
            for (int i = 0; i < ARRAY_SIZE_X; i++)
            {
                for (int j = 0; j < ARRAY_SIZE_Z; j++)
                {
                    float x = 1.0 * i + start_x;
                    float y = 5.0 + 1.0 * k + start_y;
                    float z = 1.0 * j + start_z;
                    rbDes.originalTransform.setIdentity();
                    rbDes.originalTransform.translate(x, y, z);

                    auto mesh = PhysicsMeshRenderer::create("MeshRendererTest/sphere.c3b", &rbDes);
                    mesh->setTexture("MeshRendererTest/plane.png");
                    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
                    mesh->setScale(1.0f / mesh->getContentSize().width);
                    this->addChild(mesh);
                    mesh->setPosition3D(Vec3(x, y, z));
                    mesh->syncNodeToPhysics();

                    mesh->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::PHYSICS_TO_NODE);
                }
            }
        }
    }

    physicsScene->setPhysics3DDebugCamera(_camera);
    return true;
}

bool Physics3DConstraintDemo::init()
{
    if (!Physics3DTestDemo::init())
        return false;

    // PhysicsMeshRenderer = MeshRenderer + Physics3DComponent
    Physics3DRigidBodyDes rbDes;
    rbDes.disableSleep = true;
    // create box
    auto mesh = MeshRenderer::create("MeshRendererTest/orc.c3b");
    rbDes.mass = 10.f;
    rbDes.shape = Physics3DShape::createBox(Vec3(5.0f, 5.0f, 5.0f));
    auto rigidBody = Physics3DRigidBody::create(&rbDes);
    Quaternion quat;
    Quaternion::createFromAxisAngle(Vec3(0.f, 1.f, 0.f), AX_DEGREES_TO_RADIANS(180), &quat);
    auto component = Physics3DComponent::create(rigidBody, Vec3(0.f, -3.f, 0.f), quat);

    mesh->addComponent(component);
    addChild(mesh);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    mesh->setScale(0.4f);
    mesh->setPosition3D(Vec3(-20.f, 5.f, 0.f));
    // sync node position to physics
    component->syncNodeToPhysics();
    // physics controlled, we will not set position for it, so we can skip sync node position to physics
    component->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::PHYSICS_TO_NODE);

    physicsScene->setPhysics3DDebugCamera(_camera);

    // create point to point constraint
    Physics3DConstraint* constraint = Physics3DPointToPointConstraint::create(rigidBody, Vec3(2.5f, 2.5f, 2.5f));
    physicsScene->getPhysics3DWorld()->addPhysics3DConstraint(constraint);

    // create hinge constraint
    rbDes.mass = 1.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(8.0f, 8.0f, 1.f));
    rigidBody = Physics3DRigidBody::create(&rbDes);
    component = Physics3DComponent::create(rigidBody);
    mesh = MeshRenderer::create("MeshRendererTest/box.c3t");
    mesh->setTexture("MeshRendererTest/plane.png");
    mesh->setScaleX(8.f);
    mesh->setScaleY(8.f);
    mesh->setPosition3D(Vec3(5.f, 0.f, 0.f));
    mesh->addComponent(component);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(mesh);
    component->syncNodeToPhysics();
    rigidBody->setAngularVelocity(Vec3(0, 3, 0));
    constraint = Physics3DHingeConstraint::create(rigidBody, Vec3(4.f, 4.f, 0.5f), Vec3(0.f, 1.f, 0.f));
    physicsScene->getPhysics3DWorld()->addPhysics3DConstraint(constraint);

    // create slider constraint
    rbDes.mass = 1.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(3.0f, 2.0f, 3.f));
    rigidBody = Physics3DRigidBody::create(&rbDes);
    component = Physics3DComponent::create(rigidBody);
    mesh = MeshRenderer::create("MeshRendererTest/box.c3t");
    mesh->setTexture("MeshRendererTest/plane.png");
    mesh->setScaleX(3.f);
    mesh->setScaleZ(3.f);
    mesh->setPosition3D(Vec3(30.f, 15.f, 0.f));
    mesh->addComponent(component);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(mesh);
    component->syncNodeToPhysics();
    rigidBody->setLinearVelocity(Vec3(0, 3, 0));

    rbDes.mass = 0.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(3.0f, 3.0f, 3.f));
    auto rigidBodyB = Physics3DRigidBody::create(&rbDes);
    component = Physics3DComponent::create(rigidBodyB);
    mesh = MeshRenderer::create("MeshRendererTest/box.c3t");
    mesh->setTexture("MeshRendererTest/plane.png");
    mesh->setScale(3.f);
    mesh->setPosition3D(Vec3(30.f, 5.f, 0.f));
    mesh->addComponent(component);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(mesh);
    component->syncNodeToPhysics();

    Mat4 frameInA, frameInB;
    Mat4::createRotationZ(AX_DEGREES_TO_RADIANS(90), &frameInA);
    frameInB = frameInA;
    frameInA.m[13] = -5.f;
    frameInB.m[13] = 5.f;
    constraint = Physics3DSliderConstraint::create(rigidBody, rigidBodyB, frameInA, frameInB, false);
    physicsScene->getPhysics3DWorld()->addPhysics3DConstraint(constraint);
    ((Physics3DSliderConstraint*)constraint)->setLowerLinLimit(-5.f);
    ((Physics3DSliderConstraint*)constraint)->setUpperLinLimit(5.f);

    // create ConeTwist constraint
    rbDes.mass = 1.f;
    rbDes.shape = Physics3DShape::createBox(Vec3(3.f, 3.f, 3.f));
    rigidBody = Physics3DRigidBody::create(&rbDes);
    component = Physics3DComponent::create(rigidBody);
    mesh = MeshRenderer::create("MeshRendererTest/box.c3t");
    mesh->setTexture("MeshRendererTest/plane.png");
    mesh->setScale(3.f);
    mesh->setPosition3D(Vec3(-10.f, 5.f, 0.f));
    mesh->addComponent(component);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(mesh);
    component->syncNodeToPhysics();

    Mat4::createRotationZ(AX_DEGREES_TO_RADIANS(90), &frameInA);
    frameInA.m[12] = 0.f;
    frameInA.m[13] = -10.f;
    frameInA.m[14] = 0.f;
    constraint = Physics3DConeTwistConstraint::create(rigidBody, frameInA);
    physicsScene->getPhysics3DWorld()->addPhysics3DConstraint(constraint, true);
    ((Physics3DConeTwistConstraint*)constraint)
        ->setLimit(AX_DEGREES_TO_RADIANS(10), AX_DEGREES_TO_RADIANS(10), AX_DEGREES_TO_RADIANS(40));

    // create 6 dof constraint
    rbDes.mass = 1.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(3.0f, 3.0f, 3.f));
    rigidBody = Physics3DRigidBody::create(&rbDes);
    component = Physics3DComponent::create(rigidBody);
    mesh = MeshRenderer::create("MeshRendererTest/box.c3t");
    mesh->setTexture("MeshRendererTest/plane.png");
    mesh->setScale(3.f);
    mesh->setPosition3D(Vec3(30.f, -5.f, 0.f));
    mesh->addComponent(component);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(mesh);
    component->syncNodeToPhysics();
    frameInA.setIdentity();
    constraint = Physics3D6DofConstraint::create(rigidBody, frameInA, false);
    physicsScene->getPhysics3DWorld()->addPhysics3DConstraint(constraint);
    ((Physics3D6DofConstraint*)constraint)->setAngularLowerLimit(Vec3(0, 0, 0));
    ((Physics3D6DofConstraint*)constraint)->setAngularUpperLimit(Vec3(0, 0, 0));
    ((Physics3D6DofConstraint*)constraint)->setLinearLowerLimit(Vec3(-10, 0, 0));
    ((Physics3D6DofConstraint*)constraint)->setLinearUpperLimit(Vec3(10, 0, 0));

    return true;
}

void Physics3DConstraintDemo::onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    // ray trace
    if (_camera)
    {
        auto touch = touches[0];
        auto location = touch->getLocationInView();
        Vec3 nearP(location.x, location.y, 0.0f), farP(location.x, location.y, 1.0f);

        auto size = Director::getInstance()->getWinSize();
        _camera->unproject(size, &nearP, &nearP);
        _camera->unproject(size, &farP, &farP);

        Physics3DWorld::HitResult result;
        bool ret = physicsScene->getPhysics3DWorld()->rayCast(nearP, farP, &result);
        if (ret && result.hitObj->getObjType() == Physics3DObject::PhysicsObjType::RIGID_BODY)
        {
            auto mat = result.hitObj->getWorldTransform().getInversed();
            Vec3 position;
            mat.transformPoint(result.hitPosition, &position);

            _constraint =
                Physics3DPointToPointConstraint::create(static_cast<Physics3DRigidBody*>(result.hitObj), position);
            physicsScene->getPhysics3DWorld()->addPhysics3DConstraint(_constraint, true);
            _pickingDistance = (result.hitPosition - nearP).length();
            event->stopPropagation();
            return;
        }
    }
    Physics3DTestDemo::onTouchesBegan(touches, event);
    _needShootBox = false;
}
void Physics3DConstraintDemo::onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    if (_constraint)
    {
        auto p2pConstraint = ((Physics3DPointToPointConstraint*)_constraint);

        auto touch = touches[0];
        auto location = touch->getLocationInView();
        Vec3 nearP(location.x, location.y, 0.0f), farP(location.x, location.y, 1.0f);

        auto size = Director::getInstance()->getWinSize();
        _camera->unproject(size, &nearP, &nearP);
        _camera->unproject(size, &farP, &farP);
        auto dir = (farP - nearP).getNormalized();
        p2pConstraint->setPivotPointInB(nearP + dir * _pickingDistance);
        event->stopPropagation();
        return;
    }
    Physics3DTestDemo::onTouchesMoved(touches, event);
}
void Physics3DConstraintDemo::onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    if (_constraint)
    {
        physicsScene->getPhysics3DWorld()->removePhysics3DConstraint(_constraint);
        _constraint = nullptr;
        event->stopPropagation();
        return;
    }
    Physics3DTestDemo::onTouchesEnded(touches, event);
}

bool Physics3DTerrainDemo::init()
{
    if (!Physics3DTestDemo::init())
        return false;

    Terrain::DetailMap r("TerrainTest/dirt.jpg"), g("TerrainTest/Grass2.jpg", 10), b("TerrainTest/road.jpg"),
        a("TerrainTest/GreenSkin.jpg", 20);

    Terrain::TerrainData data("TerrainTest/heightmap129.jpg", "TerrainTest/alphamap.png", r, g, b, a, Size(32, 32),
                              20.0f, 1.0f);
    auto terrain = Terrain::create(data, Terrain::CrackFixedType::SKIRT);
    terrain->setMaxDetailMapAmount(4);
    terrain->setCameraMask(2);
    terrain->setDrawWire(false);

    terrain->setSkirtHeightRatio(3);
    terrain->setLODDistance(64, 128, 192);
    terrain->setCameraMask((unsigned short)CameraFlag::USER1);

    // create terrain
    std::vector<float> heidata = terrain->getHeightData();
    auto size = terrain->getTerrainSize();
    Physics3DColliderDes colliderDes;
    colliderDes.shape =
        Physics3DShape::createHeightfield(size.width, size.height, &heidata[0], 1.0f, terrain->getMinHeight(),
                                          terrain->getMaxHeight(), true, false, true);
    auto collider = Physics3DCollider::create(&colliderDes);
    auto component = Physics3DComponent::create(collider);
    terrain->addComponent(component);
    this->addChild(terrain);
    component->syncNodeToPhysics();
    component->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::NONE);

    // create several spheres
    Physics3DRigidBodyDes rbDes;
    rbDes.mass = 1.f;
    rbDes.shape = Physics3DShape::createSphere(0.5f);
    float start_x = START_POS_X - ARRAY_SIZE_X / 2 + 5.0f;
    float start_y = START_POS_Y + 20.0f;
    float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

    for (int k = 0; k < ARRAY_SIZE_Y; k++)
    {
        for (int i = 0; i < ARRAY_SIZE_X; i++)
        {
            for (int j = 0; j < ARRAY_SIZE_Z; j++)
            {
                float x = 1.0 * i + start_x;
                float y = 5.0 + 1.0 * k + start_y;
                float z = 1.0 * j + start_z;

                auto mesh = PhysicsMeshRenderer::create("MeshRendererTest/sphere.c3b", &rbDes);
                mesh->setTexture("MeshRendererTest/plane.png");
                mesh->setCameraMask((unsigned short)CameraFlag::USER1);
                mesh->setScale(1.0f / mesh->getContentSize().width);
                mesh->setPosition3D(Vec3(x, y, z));
                this->addChild(mesh);
                mesh->syncNodeToPhysics();
                mesh->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::PHYSICS_TO_NODE);
            }
        }
    }

    // create mesh

    for (int i = 0; i < boss_count; i++)
    {
        std::vector<Vec3> trianglesList = Bundle3D::getTrianglesList(boss[i]);

        colliderDes.shape = Physics3DShape::createMesh(&trianglesList[0], (int)trianglesList.size() / 3);

        auto mesh = PhysicsMeshRenderer::createWithCollider(boss[i], &colliderDes);
        mesh->setRotation3D(Vec3(-90.0f, 0.0f, 0.0f));
        mesh->setPosition3D(Vec3(-5+ 15.0f *i, 15.0f, 0.0f));
        mesh->setCameraMask(2);
        this->addChild(mesh);
        mesh->syncNodeToPhysics();
        mesh->setSyncFlag(Physics3DComponent::PhysicsSyncFlag::NONE);

        std::vector<std::pair<Physics3DShape*, Mat4>> shapeList;
        {
            Mat4 localTrans;
            auto bodyshape = Physics3DShape::createBox(Vec3(2.0f, 4.0f, 2.0f));
            Mat4::createTranslation(0.0f, 2.0f, 0.0f, &localTrans);
            shapeList.emplace_back(std::make_pair(bodyshape, localTrans));
            auto headshape = Physics3DShape::createSphere(1.5f);
            Mat4::createTranslation(0.6f, 5.0f, -1.5f, &localTrans);
            shapeList.emplace_back(std::make_pair(headshape, localTrans));
            auto lhandshape = Physics3DShape::createBox(Vec3(1.0f, 3.0f, 1.0f));
            Mat4::createRotation(Vec3(1.0f, 0.0f, 0.0f), AX_DEGREES_TO_RADIANS(15.0f), &localTrans);
            localTrans.m[12] = -1.5f;
            localTrans.m[13] = 2.5f;
            localTrans.m[14] = -2.5f;
            shapeList.emplace_back(std::make_pair(lhandshape, localTrans));
            auto rhandshape = Physics3DShape::createBox(Vec3(1.0f, 3.0f, 1.0f));
            Mat4::createRotation(Vec3(1.0f, 0.0f, 0.0f), AX_DEGREES_TO_RADIANS(-15.0f), &localTrans);
            localTrans.m[12] = 2.0f;
            localTrans.m[13] = 2.5f;
            localTrans.m[14] = 1.f;
            shapeList.emplace_back(std::make_pair(rhandshape, localTrans));

            rbDes.mass = 10.0f;
            rbDes.shape = Physics3DShape::createCompoundShape(shapeList);
            auto rigidBody = Physics3DRigidBody::create(&rbDes);
            component = Physics3DComponent::create(rigidBody);
            auto mesh = MeshRenderer::create("MeshRendererTest/orc.c3b");
            mesh->addComponent(component);
            mesh->setRotation3D(Vec3(0.0f, 180.0f, 0.0f));
            mesh->setPosition3D(Vec3(-5.0f, 20.0f, 0.0f));
            mesh->setScale(0.4f);
            mesh->setCameraMask(2);
            this->addChild(mesh);
        }

    }

    physicsScene->setPhysics3DDebugCamera(_camera);
    return true;
}

std::string Physics3DTerrainDemo::subtitle() const
{
    return "Physics3D Terrain Issue #861" ;
}

std::string Physics3DCollisionCallbackDemo::subtitle() const
{
    return "Physics3D CollisionCallback c3b/obj Issue #861";
}

bool Physics3DCollisionCallbackDemo::init()
{
    if (!Physics3DTestDemo::init())
        return false;

    {
        for (size_t i = 0; i < boss_count; i++)
        {
            Physics3DRigidBodyDes rbDes;

            float scale = 2.0f;
            std::vector<Vec3> trianglesList = Bundle3D::getTrianglesList(boss[i]);
            for (auto&& it : trianglesList)
            {
                it *= scale;
            }

            rbDes.mass = 0.0f;
            rbDes.shape = Physics3DShape::createMesh(&trianglesList[0], (int)trianglesList.size() / 3);
            auto rigidBody = Physics3DRigidBody::create(&rbDes);
            auto component = Physics3DComponent::create(rigidBody);
            auto mesh = MeshRenderer::create(boss[i]);
            mesh->addComponent(component);
            mesh->setRotation3D(Vec3(-90.0f, 0.0f, 0.0f));
            mesh->setPosition3D(Vec3(-5 + 15.0f * i, -5.0f, 0.0f));
            mesh->setScale(scale);
            mesh->setCameraMask((unsigned short)CameraFlag::USER1);
            this->addChild(mesh);
            // preload
            //
            rigidBody->setCollisionCallback([this](const Physics3DCollisionInfo& ci) {
                if (!ci.collisionPointList.empty())
                {
                    if (ci.objA->getMask() != 0)
                    {
                        auto ps = PUParticleSystem3D::create("Particle3D/scripts/mp_hit_04.pu");
                        ps->setPosition3D(ci.collisionPointList[0].worldPositionOnB);
                        ps->setScale(0.05f);
                        ps->startParticleSystem();
                        ps->setCameraMask(2);
                        this->addChild(ps);
                        ps->runAction(Sequence::create(DelayTime::create(1.0f),
                                                       CallFunc::create([=]() { ps->removeFromParent(); }), nullptr));
                        ci.objA->setMask(0);
                    }
                }
                // AXLOGD("------------BoxB Collision Info------------");
                // AXLOGD("Collision Point Num: {}", ci.collisionPointList.size());
                // for (auto&& iter : ci.collisionPointList){
                //	AXLOGD("Collision Position On A: ({:.2},{:.2}, {:.2})", iter.worldPositionOnA.x, iter.worldPositionOnA.y,
                // iter.worldPositionOnA.z); 	AXLOGD("Collision Position On B: ({:.2}, {:.2}, {:.2})",
                // iter.worldPositionOnB.x, iter.worldPositionOnB.y, iter.worldPositionOnB.z); 	AXLOGD("Collision Normal
                // On B: ({:.2}, {:.2}, {:.2})", iter.worldNormalOnB.x, iter.worldNormalOnB.y, iter.worldNormalOnB.z);
                // }
                // AXLOGD("------------BoxB Collision Info------------");
            });
        }
    }

    physicsScene->setPhysics3DDebugCamera(_camera);
    return true;
}

std::string Physics3DColliderDemo::subtitle() const
{
    return "Physics3D Trigger";
}

bool Physics3DColliderDemo::init()
{
    if (!Physics3DTestDemo::init())
        return false;

    Physics3DRigidBodyDes rbDes;
    rbDes.mass = 1.0f;
    rbDes.shape = Physics3DShape::createBox(Vec3(3.0f, 3.0f, 3.f));
    auto playerBody = Physics3DRigidBody::create(&rbDes);
    auto component = Physics3DComponent::create(playerBody);
    playerBody->setKinematic(true);
    auto mesh = MeshRenderer::create("MeshRendererTest/box.c3t");
    mesh->setTexture("MeshRendererTest/plane.png");
    mesh->setScale(3.f);
    mesh->setPosition3D(Vec3(0.0f, 0.f, 30.f));
    mesh->addComponent(component);
    mesh->setCameraMask((unsigned short)CameraFlag::USER1);
    auto moveby = MoveBy::create(5.0f, Vec3(0.0f, 0.0f, -60.0f));
    mesh->runAction(RepeatForever::create(Sequence::create(moveby, moveby->reverse(), nullptr)));
    this->addChild(mesh);

    {
        Physics3DColliderDes colliderDes;
        colliderDes.shape = Physics3DShape::createSphere(10.0f);
        colliderDes.isTrigger = true;
        auto collider = Physics3DCollider::create(&colliderDes);
        auto component = Physics3DComponent::create(collider);
        auto node = Node::create();
        node->addComponent(component);
        node->setCameraMask((unsigned short)CameraFlag::USER1);
        this->addChild(node);

        Physics3DRigidBodyDes rbDes;
        rbDes.mass = 1.0f;
        rbDes.shape = Physics3DShape::createBox(Vec3(10.0f, 10.0f, 1.f));
        auto rigidBody = Physics3DRigidBody::create(&rbDes);
        component = Physics3DComponent::create(rigidBody);
        rigidBody->setKinematic(true);
        auto doorLeft = MeshRenderer::create("MeshRendererTest/box.c3t");
        doorLeft->setTexture("MeshRendererTest/plane.png");
        doorLeft->setScaleX(10.0f);
        doorLeft->setScaleY(10.0f);
        doorLeft->setScaleZ(1.0f);
        doorLeft->setPosition3D(Vec3(-5.0f, 0.0f, 0.0f));
        doorLeft->addComponent(component);
        doorLeft->setCameraMask((unsigned short)CameraFlag::USER1);
        node->addChild(doorLeft);

        rbDes.mass = 1.0f;
        rbDes.shape = Physics3DShape::createBox(Vec3(10.0f, 10.0f, 1.f));
        rigidBody = Physics3DRigidBody::create(&rbDes);
        component = Physics3DComponent::create(rigidBody);
        rigidBody->setKinematic(true);
        auto doorRight = MeshRenderer::create("MeshRendererTest/box.c3t");
        doorRight->setTexture("MeshRendererTest/plane.png");
        doorRight->setScaleX(10.0f);
        doorRight->setScaleY(10.0f);
        doorRight->setScaleZ(1.0f);
        doorRight->setPosition3D(Vec3(5.0f, 0.0f, 0.0f));
        doorRight->addComponent(component);
        doorRight->setCameraMask((unsigned short)CameraFlag::USER1);
        node->addChild(doorRight);

        collider->onTriggerEnter = [=](Physics3DObject* otherObject) {
            if (otherObject == playerBody)
            {
                auto moveby = MoveBy::create(1.0f, Vec3(-5.0f, 0.0f, 0.0f));
                doorLeft->runAction(moveby);
                doorRight->runAction(moveby->reverse());
            }
        };

        collider->onTriggerExit = [=](Physics3DObject* otherObject) {
            if (otherObject == playerBody)
            {
                auto moveby = MoveBy::create(1.0f, Vec3(5.0f, 0.0f, 0.0f));
                doorLeft->runAction(moveby);
                doorRight->runAction(moveby->reverse());
            }
        };
    }

    physicsScene->setPhysics3DDebugCamera(_camera);
    return true;
}

#endif
