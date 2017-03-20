//
//  GLESBackendTransform.cpp
//  libraries/gpu-gl-android/src/gpu/gles
//
//  Created by Gabriel Calero & Cristian Duarte on 9/28/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLESBackend.h"

using namespace gpu;
using namespace gpu::gles;

void GLESBackend::initTransform() {
    glGenBuffers(1, &_transform._objectBuffer);
    //glGenBuffers(1, &_transform._cameraBuffer);
    glGenBuffers(1, &_transform._cameraMonoBuffer);
    glGenBuffers(1, &_transform._cameraStereoBuffer);
    glGenBuffers(1, &_transform._drawCallInfoBuffer);
    glGenTextures(1, &_transform._objectBufferTexture);
    size_t cameraSize = sizeof(TransformStageState::CameraBufferElement);
    while (_transform._cameraUboSize < cameraSize) {
        _transform._cameraUboSize += _uboAlignment;
    }
}

void GLESBackend::transferTransformState(const Batch& batch) const {
    // FIXME not thread safe
    static std::vector<uint8_t> bufferData;
    if (!_transform._cameras.empty()) {
        /*if (_transform._cameras.size() == 1 && !cameraMonoTransferredx) {
            bufferData.resize(_transform._cameraUboSize * _transform._cameras.size());
            memcpy(bufferData.data(), &_transform._cameras[0], sizeof(TransformStageState::CameraBufferElement));
            glBindBuffer(GL_UNIFORM_BUFFER, _transform._cameraMonoBuffer);
            qDebug() << "[CHECK-PERFORMANCE] glBufferData (mono) GL_UNIFORM_BUFFER "<< _transform._cameraMonoBuffer<< " (" << bufferData.size() << ")";
            glBufferData(GL_UNIFORM_BUFFER, bufferData.size(), bufferData.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            setCameraMonoTransferred(true);
        } else if (_transform._cameras.size() == 2 && !cameraStereoTransferredx) {*/
            bufferData.resize(_transform._cameraUboSize * _transform._cameras.size());
            for (size_t i = 0; i < _transform._cameras.size(); ++i) {
                memcpy(bufferData.data() + (_transform._cameraUboSize * i), &_transform._cameras[i], sizeof(TransformStageState::CameraBufferElement));
            }
            glBindBuffer(GL_UNIFORM_BUFFER, _transform._cameraStereoBuffer);
            qDebug() << "[CHECK-PERFORMANCE] glBufferData (stereo) GL_UNIFORM_BUFFER "<< _transform._cameraStereoBuffer<< " (" << bufferData.size() << ")";
            glBufferData(GL_UNIFORM_BUFFER, bufferData.size(), bufferData.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            setCameraStereoTransferred(true);
        //}
    }

    if (!batch._objects.empty()) {
#ifdef GPU_SSBO_DRAW_CALL_INFO
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _transform._objectBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sysmem.getSize(), sysmem.readData(), GL_STREAM_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#else
        glBindBuffer(GL_TEXTURE_BUFFER_EXT, _transform._objectBuffer);
        glBufferData(GL_TEXTURE_BUFFER_EXT, batch._objects.size() * sizeof(Batch::TransformObject), batch._objects.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_TEXTURE_BUFFER_EXT, 0);
#endif
    }

    if (!batch._namedData.empty()) {
        bufferData.clear();
        for (auto& data : batch._namedData) {
            auto currentSize = bufferData.size();
            auto bytesToCopy = data.second.drawCallInfos.size() * sizeof(Batch::DrawCallInfo);
            bufferData.resize(currentSize + bytesToCopy);
            memcpy(bufferData.data() + currentSize, data.second.drawCallInfos.data(), bytesToCopy);
            _transform._drawCallInfoOffsets[data.first] = (GLvoid*)currentSize;
        }

        //qDebug() << "GLESBackend::transferTransformState with size " << bufferData.size();
        glBindBuffer(GL_ARRAY_BUFFER, _transform._drawCallInfoBuffer);
        glBufferData(GL_ARRAY_BUFFER, bufferData.size(), bufferData.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

#ifdef GPU_SSBO_DRAW_CALL_INFO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRANSFORM_OBJECT_SLOT, _transform._objectBuffer);
#else
    glActiveTexture(GL_TEXTURE0 + TRANSFORM_OBJECT_SLOT);
    glBindTexture(GL_TEXTURE_BUFFER_EXT, _transform._objectBufferTexture);

    if (!batch._objects.empty()) {
        glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F, _transform._objectBuffer);
    }
#endif

    CHECK_GL_ERROR();

    // Make sure the current Camera offset is unknown before render Draw
    _transform._currentCameraOffset = INVALID_OFFSET;
}
