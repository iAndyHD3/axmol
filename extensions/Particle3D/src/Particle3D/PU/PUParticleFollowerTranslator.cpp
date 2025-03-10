/****************************************************************************
 Copyright (C) 2013 Henry van Merode. All rights reserved.
 Copyright (c) 2015-2016 Chukong Technologies Inc.
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

#include "PUParticleFollowerTranslator.h"
#include "Particle3D/PU/PUParticleSystem3D.h"
#include "Particle3D/PU/PUDynamicAttribute.h"
#include "Particle3D/PU/PUDynamicAttributeTranslator.h"

namespace ax
{

PUParticleFollowerTranslator::PUParticleFollowerTranslator() {}
//-------------------------------------------------------------------------
bool PUParticleFollowerTranslator::translateChildProperty(PUScriptCompiler* compiler, PUAbstractNode* node)
{
    PUPropertyAbstractNode* prop = reinterpret_cast<PUPropertyAbstractNode*>(node);
    PUAffector* af               = static_cast<PUAffector*>(prop->parent->context);
    PUParticleFollower* affector = static_cast<PUParticleFollower*>(af);

    if (prop->name == token[TOKEN_MIN_DISTANCE])
    {
        // Property: min_distance
        if (passValidateProperty(compiler, prop, token[TOKEN_MIN_DISTANCE], VAL_REAL))
        {
            float val = 0.0f;
            if (getFloat(*prop->values.front(), &val))
            {
                affector->setMinDistance(val);
                return true;
            }
        }
    }
    else if (prop->name == token[TOKEN_FOLLOW_MIN_DISTANCE])
    {
        // Property: follower_min_distance (deprecated and replaced by 'min_distance')
        if (passValidateProperty(compiler, prop, token[TOKEN_FOLLOW_MIN_DISTANCE], VAL_REAL))
        {
            float val = 0.0f;
            if (getFloat(*prop->values.front(), &val))
            {
                affector->setMinDistance(val);
                return true;
            }
        }
    }
    else if (prop->name == token[TOKEN_MAX_DISTANCE])
    {
        // Property: max_distance
        if (passValidateProperty(compiler, prop, token[TOKEN_MAX_DISTANCE], VAL_REAL))
        {
            float val = 0.0f;
            if (getFloat(*prop->values.front(), &val))
            {
                affector->setMaxDistance(val);
                return true;
            }
        }
    }
    else if (prop->name == token[TOKEN_FOLLOW_MAX_DISTANCE])
    {
        // Property: follower_max_distance (deprecated and replaced by 'TOKEN_MAX_DISTANCE')
        if (passValidateProperty(compiler, prop, token[TOKEN_FOLLOW_MAX_DISTANCE], VAL_REAL))
        {
            float val = 0.0f;
            if (getFloat(*prop->values.front(), &val))
            {
                affector->setMaxDistance(val);
                return true;
            }
        }
    }

    return false;
}

bool PUParticleFollowerTranslator::translateChildObject(PUScriptCompiler* /*compiler*/, PUAbstractNode* /*node*/)
{
    // No objects
    return false;
}

}
