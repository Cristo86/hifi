//
//  Created by Bradley Austin Davis on 2016/05/14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLShared.h"

#include <mutex>

#include <QtCore/QThread>

#include <GPUIdent.h>
#include <NumericalConstants.h>
#include <fstream>

Q_LOGGING_CATEGORY(gpugllogging, "hifi.gpu.gl")

namespace gpu { namespace gl {

bool checkGLError(const char* name) {/*
    GLenum error = glGetError();
    if (!error) {
        return false;
    } else {
        switch (error) {
        case GL_INVALID_ENUM:
            qCDebug(gpugllogging) << "GLBackend::" << name << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_INVALID_VALUE:
            qCDebug(gpugllogging) << "GLBackend" << name << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
            break;
        case GL_INVALID_OPERATION:
            qCDebug(gpugllogging) << "GLBackend" << name << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            qCDebug(gpugllogging) << "GLBackend" << name << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_OUT_OF_MEMORY:
            qCDebug(gpugllogging) << "GLBackend" << name << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
            break;
        case GL_STACK_UNDERFLOW:
            qCDebug(gpugllogging) << "GLBackend" << name << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
            break;
        case GL_STACK_OVERFLOW:
            qCDebug(gpugllogging) << "GLBackend" << name << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
            break;
        }
        return true;
    }
*/}

bool checkGLErrorDebug(const char* name) {
#ifdef DEBUG
    return checkGLError(name);
#else
    Q_UNUSED(name);
    return false;
#endif
}

gpu::Size getDedicatedMemory() { }




ComparisonFunction comparisonFuncFromGL(GLenum func) {/*
    if (func == GL_NEVER) {
        return NEVER;
    } else if (func == GL_LESS) {
        return LESS;
    } else if (func == GL_EQUAL) {
        return EQUAL;
    } else if (func == GL_LEQUAL) {
        return LESS_EQUAL;
    } else if (func == GL_GREATER) {
        return GREATER;
    } else if (func == GL_NOTEQUAL) {
        return NOT_EQUAL;
    } else if (func == GL_GEQUAL) {
        return GREATER_EQUAL;
    } else if (func == GL_ALWAYS) {
        return ALWAYS;
    }

    return ALWAYS;
*/}

State::StencilOp stencilOpFromGL(GLenum stencilOp) {/*
    if (stencilOp == GL_KEEP) {
        return State::STENCIL_OP_KEEP;
    } else if (stencilOp == GL_ZERO) {
        return State::STENCIL_OP_ZERO;
    } else if (stencilOp == GL_REPLACE) {
        return State::STENCIL_OP_REPLACE;
    } else if (stencilOp == GL_INCR_WRAP) {
        return State::STENCIL_OP_INCR_SAT;
    } else if (stencilOp == GL_DECR_WRAP) {
        return State::STENCIL_OP_DECR_SAT;
    } else if (stencilOp == GL_INVERT) {
        return State::STENCIL_OP_INVERT;
    } else if (stencilOp == GL_INCR) {
        return State::STENCIL_OP_INCR;
    } else if (stencilOp == GL_DECR) {
        return State::STENCIL_OP_DECR;
    }

    return State::STENCIL_OP_KEEP;
*/}

State::BlendOp blendOpFromGL(GLenum blendOp) {/*
    if (blendOp == GL_FUNC_ADD) {
        return State::BLEND_OP_ADD;
    } else if (blendOp == GL_FUNC_SUBTRACT) {
        return State::BLEND_OP_SUBTRACT;
    } else if (blendOp == GL_FUNC_REVERSE_SUBTRACT) {
        return State::BLEND_OP_REV_SUBTRACT;
    } else if (blendOp == GL_MIN) {
        return State::BLEND_OP_MIN;
    } else if (blendOp == GL_MAX) {
        return State::BLEND_OP_MAX;
    }

    return State::BLEND_OP_ADD;
*/}

State::BlendArg blendArgFromGL(GLenum blendArg) {/*
    if (blendArg == GL_ZERO) {
        return State::ZERO;
    } else if (blendArg == GL_ONE) {
        return State::ONE;
    } else if (blendArg == GL_SRC_COLOR) {
        return State::SRC_COLOR;
    } else if (blendArg == GL_ONE_MINUS_SRC_COLOR) {
        return State::INV_SRC_COLOR;
    } else if (blendArg == GL_DST_COLOR) {
        return State::DEST_COLOR;
    } else if (blendArg == GL_ONE_MINUS_DST_COLOR) {
        return State::INV_DEST_COLOR;
    } else if (blendArg == GL_SRC_ALPHA) {
        return State::SRC_ALPHA;
    } else if (blendArg == GL_ONE_MINUS_SRC_ALPHA) {
        return State::INV_SRC_ALPHA;
    } else if (blendArg == GL_DST_ALPHA) {
        return State::DEST_ALPHA;
    } else if (blendArg == GL_ONE_MINUS_DST_ALPHA) {
        return State::INV_DEST_ALPHA;
    } else if (blendArg == GL_CONSTANT_COLOR) {
        return State::FACTOR_COLOR;
    } else if (blendArg == GL_ONE_MINUS_CONSTANT_COLOR) {
        return State::INV_FACTOR_COLOR;
    } else if (blendArg == GL_CONSTANT_ALPHA) {
        return State::FACTOR_ALPHA;
    } else if (blendArg == GL_ONE_MINUS_CONSTANT_ALPHA) {
        return State::INV_FACTOR_ALPHA;
    }

    return State::ONE;
*/}

void getCurrentGLState(State::Data& state) { /*
    {
        GLint modes[2];
        glGetIntegerv(GL_POLYGON_MODE, modes);
        if (modes[0] == GL_FILL) {
            state.fillMode = State::FILL_FACE;
        } else {
            if (modes[0] == GL_LINE) {
                state.fillMode = State::FILL_LINE;
            } else {
                state.fillMode = State::FILL_POINT;
            }
        }
    }
    {
        if (glIsEnabled(GL_CULL_FACE)) {
            GLint mode;
            glGetIntegerv(GL_CULL_FACE_MODE, &mode);
            state.cullMode = (mode == GL_FRONT ? State::CULL_FRONT : State::CULL_BACK);
        } else {
            state.cullMode = State::CULL_NONE;
        }
    }
    {
        GLint winding;
        glGetIntegerv(GL_FRONT_FACE, &winding);
        state.frontFaceClockwise = (winding == GL_CW);
        state.depthClampEnable = glIsEnabled(GL_DEPTH_CLAMP);
        state.scissorEnable = glIsEnabled(GL_SCISSOR_TEST);
        state.multisampleEnable = glIsEnabled(GL_MULTISAMPLE);
        state.antialisedLineEnable = glIsEnabled(GL_LINE_SMOOTH);
    }
    {
        if (glIsEnabled(GL_POLYGON_OFFSET_FILL)) {
            glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &state.depthBiasSlopeScale);
            glGetFloatv(GL_POLYGON_OFFSET_UNITS, &state.depthBias);
        }
    }
    {
        GLboolean isEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean writeMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &writeMask);
        GLint func;
        glGetIntegerv(GL_DEPTH_FUNC, &func);

        state.depthTest = State::DepthTest(isEnabled, writeMask, comparisonFuncFromGL(func));
    }
    {
        GLboolean isEnabled = glIsEnabled(GL_STENCIL_TEST);

        GLint frontWriteMask;
        GLint frontReadMask;
        GLint frontRef;
        GLint frontFail;
        GLint frontDepthFail;
        GLint frontPass;
        GLint frontFunc;
        glGetIntegerv(GL_STENCIL_WRITEMASK, &frontWriteMask);
        glGetIntegerv(GL_STENCIL_VALUE_MASK, &frontReadMask);
        glGetIntegerv(GL_STENCIL_REF, &frontRef);
        glGetIntegerv(GL_STENCIL_FAIL, &frontFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &frontDepthFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &frontPass);
        glGetIntegerv(GL_STENCIL_FUNC, &frontFunc);

        GLint backWriteMask;
        GLint backReadMask;
        GLint backRef;
        GLint backFail;
        GLint backDepthFail;
        GLint backPass;
        GLint backFunc;
        glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &backWriteMask);
        glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &backReadMask);
        glGetIntegerv(GL_STENCIL_BACK_REF, &backRef);
        glGetIntegerv(GL_STENCIL_BACK_FAIL, &backFail);
        glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &backDepthFail);
        glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, &backPass);
        glGetIntegerv(GL_STENCIL_BACK_FUNC, &backFunc);

        state.stencilActivation = State::StencilActivation(isEnabled, frontWriteMask, backWriteMask);
        state.stencilTestFront = State::StencilTest(frontRef, frontReadMask, comparisonFuncFromGL(frontFunc), stencilOpFromGL(frontFail), stencilOpFromGL(frontDepthFail), stencilOpFromGL(frontPass));
        state.stencilTestBack = State::StencilTest(backRef, backReadMask, comparisonFuncFromGL(backFunc), stencilOpFromGL(backFail), stencilOpFromGL(backDepthFail), stencilOpFromGL(backPass));
    }
    {
        GLint mask = 0xFFFFFFFF;
        if (glIsEnabled(GL_SAMPLE_MASK)) {
            glGetIntegerv(GL_SAMPLE_MASK, &mask);
            state.sampleMask = mask;
        }
        state.sampleMask = mask;
    }
    {
        state.alphaToCoverageEnable = glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }
    {
        GLboolean isEnabled = glIsEnabled(GL_BLEND);
        GLint srcRGB;
        GLint srcA;
        GLint dstRGB;
        GLint dstA;
        glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcA);
        glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &dstA);

        GLint opRGB;
        GLint opA;
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &opRGB);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &opA);

        state.blendFunction = State::BlendFunction(isEnabled,
            blendArgFromGL(srcRGB), blendOpFromGL(opRGB), blendArgFromGL(dstRGB),
            blendArgFromGL(srcA), blendOpFromGL(opA), blendArgFromGL(dstA));
    }
    {
        GLboolean mask[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, mask);
        state.colorWriteMask = (mask[0] ? State::WRITE_RED : 0)
            | (mask[1] ? State::WRITE_GREEN : 0)
            | (mask[2] ? State::WRITE_BLUE : 0)
            | (mask[3] ? State::WRITE_ALPHA : 0);
    }

    (void)CHECK_GL_ERROR();
*/}


class ElementResource {
public:
    gpu::Element _element;
    uint16 _resource;

    ElementResource(Element&& elem, uint16 resource) : _element(elem), _resource(resource) {}
};

ElementResource getFormatFromGLUniform(GLenum gltype) {};

int makeUniformSlots(GLuint glprogram, const Shader::BindingSet& slotBindings,
    Shader::SlotSet& uniforms, Shader::SlotSet& textures, Shader::SlotSet& samplers) { }

const GLint UNUSED_SLOT = -1;
bool isUnusedSlot(GLint binding) {
    return (binding == UNUSED_SLOT);
}

int makeUniformBlockSlots(GLuint glprogram, const Shader::BindingSet& slotBindings, Shader::SlotSet& buffers) { }

int makeInputSlots(GLuint glprogram, const Shader::BindingSet& slotBindings, Shader::SlotSet& inputs) { }

int makeOutputSlots(GLuint glprogram, const Shader::BindingSet& slotBindings, Shader::SlotSet& outputs) {
    /*   GLint outputsCount = 0;

    glGetProgramiv(glprogram, GL_ACTIVE_, &outputsCount);

    for (int i = 0; i < inputsCount; i++) {
    const GLint NAME_LENGTH = 256;
    GLchar name[NAME_LENGTH];
    GLint length = 0;
    GLint size = 0;
    GLenum type = 0;
    glGetActiveAttrib(glprogram, i, NAME_LENGTH, &length, &size, &type, name);

    auto element = getFormatFromGLUniform(type);
    outputs.insert(Shader::Slot(name, i, element));
    }
    */
    return 0; //inputsCount;
}


bool compileShader(GLenum shaderDomain, const std::string& shaderSource, const std::string& defines, GLuint &shaderObject, GLuint &programObject) {  }

GLuint compileProgram(const std::vector<GLuint>& glshaders) { }


void makeProgramBindings(ShaderObject& shaderObject) { }

void serverWait() { }

void clientWait() { }

} }


using namespace gpu;


