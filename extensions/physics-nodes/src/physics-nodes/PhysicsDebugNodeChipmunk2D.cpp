/* Copyright (c) 2012 Scott Lembcke and Howling Moon Software
 * Copyright (c) 2012 cocos2d-x.org
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 * Copyright (c) 2021 @aismann; Peter Eismann, Germany; dreifrankensoft
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "PhysicsDebugNodeChipmunk2D.h"

#include "chipmunk/chipmunk_private.h"

#include "base/Types.h"
#include "math/Math.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

NS_AX_EXT_BEGIN

Vec2 physicsDebugNodeOffset;

/*
 IMPORTANT - READ ME!

 This file sets pokes around in the private API a lot to provide efficient
 debug rendering given nothing more than reference to a Chipmunk space.
 It is not recommended to write rendering code like this in your own games
 as the private API may change with little or no warning.
 */

static const cpVect spring_verts[] = {
    {0.00f, 0.0f},  {0.20f, 0.0f},  {0.25f, 3.0f},  {0.30f, -6.0f}, {0.35f, 6.0f},
    {0.40f, -6.0f}, {0.45f, 6.0f},  {0.50f, -6.0f}, {0.55f, 6.0f},  {0.60f, -6.0f},
    {0.65f, 6.0f},  {0.70f, -3.0f}, {0.75f, 6.0f},  {0.80f, 0.0f},  {1.00f, 0.0f},
};
static const int spring_count = sizeof(spring_verts) / sizeof(cpVect);

static Color4B ColorForBody(cpBody* body)
{
    if (CP_BODY_TYPE_STATIC == cpBodyGetType(body) || cpBodyIsSleeping(body))
    {
        return Color4B(127, 127, 127, 127);
    }
    else if (body->sleeping.idleTime > cpBodyGetSpace(body)->sleepTimeThreshold)
    {
        return Color4B(85, 85, 85, 127);
    }
    else
    {
        return Color4B(255, 0, 0, 127);
    }
}

static Vec2 cpVert2Point(const cpVect& vert)
{
    return (Vec2(vert.x, vert.y) + physicsDebugNodeOffset);
}

static void DrawShape(cpShape* shape, DrawNode* renderer)
{
    cpBody* body  = cpShapeGetBody(shape);
    Color4B color = ColorForBody(body);

    switch (shape->klass->type)
    {
    case CP_CIRCLE_SHAPE:
    {
        cpCircleShape* circle = (cpCircleShape*)shape;
        cpVect center         = circle->tc;
        cpFloat radius        = circle->r;
        renderer->drawDot(cpVert2Point(center), cpfmax(radius, 1.0), color);
        renderer->drawSegment(cpVert2Point(center),
                              cpVert2Point(cpvadd(center, cpvmult(cpBodyGetRotation(body), radius))), 1.0, color);
    }
    break;
    case CP_SEGMENT_SHAPE:
    {
        cpSegmentShape* seg = (cpSegmentShape*)shape;
        renderer->drawSegment(cpVert2Point(seg->ta), cpVert2Point(seg->tb), cpfmax(seg->r, 1.0), color);
    }
    break;
    case CP_POLY_SHAPE:
    {
        cpPolyShape* poly = (cpPolyShape*)shape;
        Color4B line      = color;
        line.a            = cpflerp(color.a, 1.0, 0.5);
        int num           = poly->count;
        Vec2* pPoints     = new Vec2[num];
        for (int i = 0; i < num; ++i)
            pPoints[i] = cpVert2Point(poly->planes[i].v0);
        if (cpfmax(poly->r, 1.0) > 1.0)
        {
            renderer->drawPolygon(pPoints, num, Color4B(127, 127, 127,0), poly->r, color);
        }
        else
        {
            renderer->drawPolygon(pPoints, num, color, 1.0, line);
        }

        AX_SAFE_DELETE_ARRAY(pPoints);
    }
    break;
    default:
        cpAssertHard(false, "Bad assertion in DrawShape()");
    }
}

static Color4B CONSTRAINT_COLOR(0, 255, 0, 127);

static void DrawConstraint(cpConstraint* constraint, DrawNode* renderer)
{
    cpBody* body_a = cpConstraintGetBodyA(constraint);
    cpBody* body_b = cpConstraintGetBodyB(constraint);

    if (cpConstraintIsPinJoint(constraint))
    {
        cpVect a =
            cpvadd(cpBodyGetPosition(body_a), cpvrotate(cpPinJointGetAnchorA(constraint), cpBodyGetRotation(body_a)));
        cpVect b =
            cpvadd(cpBodyGetPosition(body_b), cpvrotate(cpPinJointGetAnchorB(constraint), cpBodyGetRotation(body_b)));

        renderer->drawDot(cpVert2Point(a), 3.0, CONSTRAINT_COLOR);
        renderer->drawDot(cpVert2Point(b), 3.0, CONSTRAINT_COLOR);
        renderer->drawSegment(cpVert2Point(a), cpVert2Point(b), 1.0, CONSTRAINT_COLOR);
    }
    else if (cpConstraintIsSlideJoint(constraint))
    {
        cpVect a =
            cpvadd(cpBodyGetPosition(body_a), cpvrotate(cpSlideJointGetAnchorA(constraint), cpBodyGetRotation(body_a)));
        cpVect b =
            cpvadd(cpBodyGetPosition(body_b), cpvrotate(cpSlideJointGetAnchorB(constraint), cpBodyGetRotation(body_b)));

        renderer->drawDot(cpVert2Point(a), 3.0, CONSTRAINT_COLOR);
        renderer->drawDot(cpVert2Point(b), 3.0, CONSTRAINT_COLOR);
        renderer->drawSegment(cpVert2Point(a), cpVert2Point(b), 1.0, CONSTRAINT_COLOR);
    }
    else if (cpConstraintIsPivotJoint(constraint))
    {
        cpVect a =
            cpvadd(cpBodyGetPosition(body_a), cpvrotate(cpPivotJointGetAnchorA(constraint), cpBodyGetRotation(body_a)));
        cpVect b =
            cpvadd(cpBodyGetPosition(body_b), cpvrotate(cpPivotJointGetAnchorB(constraint), cpBodyGetRotation(body_b)));

        renderer->drawDot(cpVert2Point(a), 3.0, CONSTRAINT_COLOR);
        renderer->drawDot(cpVert2Point(b), 3.0, CONSTRAINT_COLOR);
    }
    else if (cpConstraintIsGrooveJoint(constraint))
    {
        cpVect a = cpvadd(cpBodyGetPosition(body_a),
                          cpvrotate(cpGrooveJointGetGrooveA(constraint), cpBodyGetRotation(body_a)));
        cpVect b = cpvadd(cpBodyGetPosition(body_a),
                          cpvrotate(cpGrooveJointGetGrooveB(constraint), cpBodyGetRotation(body_a)));
        cpVect c = cpvadd(cpBodyGetPosition(body_b),
                          cpvrotate(cpGrooveJointGetAnchorB(constraint), cpBodyGetRotation(body_b)));

        renderer->drawDot(cpVert2Point(c), 3.0, CONSTRAINT_COLOR);
        renderer->drawSegment(cpVert2Point(a), cpVert2Point(b), 1.0, CONSTRAINT_COLOR);
    }
    else if (cpConstraintIsDampedSpring(constraint))
    {
        cpDampedSpring* spring = (cpDampedSpring*)constraint;

        cpVect a = cpTransformPoint(body_a->transform, spring->anchorA);
        cpVect b = cpTransformPoint(body_b->transform, spring->anchorB);

        renderer->drawDot(cpVert2Point(a), 3.0, CONSTRAINT_COLOR);
        renderer->drawDot(cpVert2Point(b), 3.0, CONSTRAINT_COLOR);

        cpVect delta = cpvsub(b, a);
        cpFloat cos  = delta.x;
        cpFloat sin  = delta.y;
        cpFloat s    = 1.0f / cpvlength(delta);

        cpVect r1 = cpv(cos, -sin * s);
        cpVect r2 = cpv(sin, cos * s);

        cpVect* verts = (cpVect*)alloca(spring_count * sizeof(cpVect));
        for (int i = 0; i < spring_count; i++)
        {
            cpVect v = spring_verts[i];
            verts[i] = cpv(cpvdot(v, r1) + a.x, cpvdot(v, r2) + a.y);
        }

        for (int i = 0; i < spring_count - 1; i++)
        {
            renderer->drawSegment(cpVert2Point(verts[i]), cpVert2Point(verts[i + 1]), 1.0, CONSTRAINT_COLOR);
        }
    }
    else
    {
        AXLOGD("Cannot draw constraint");
    }
}

// implementation of PhysicsDebugNode

void PhysicsDebugNodeChipmunk2D::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
    if (!_spacePtr)
    {
        return;
    }
    // clear the shapes information before draw current shapes.
    DrawNode::clear();

    cpSpaceEachShape(_spacePtr, (cpSpaceShapeIteratorFunc)DrawShape, this);
    cpSpaceEachConstraint(_spacePtr, (cpSpaceConstraintIteratorFunc)DrawConstraint, this);

    DrawNode::draw(renderer, transform, flags);
}

PhysicsDebugNodeChipmunk2D::PhysicsDebugNodeChipmunk2D() : _spacePtr(nullptr) {}

PhysicsDebugNodeChipmunk2D* PhysicsDebugNodeChipmunk2D::create(cpSpace* space)
{
    PhysicsDebugNodeChipmunk2D* node = new PhysicsDebugNodeChipmunk2D();
    node->init();
    node->_spacePtr = space;
    node->autorelease();
    return node;
}

PhysicsDebugNodeChipmunk2D::~PhysicsDebugNodeChipmunk2D() {}

cpSpace* PhysicsDebugNodeChipmunk2D::getSpace() const
{
    return _spacePtr;
}

void PhysicsDebugNodeChipmunk2D::setSpace(cpSpace* space)
{
    _spacePtr = space;
}

NS_AX_EXT_END
