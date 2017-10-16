//
//  RenderForwardTask.cpp
//  render-utils/src/
//
//  Created by Zach Pomerantz on 12/13/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderForwardTask.h"
#include "RenderDeferredTask.h"

#include <PerfStat.h>
#include <PathUtils.h>
#include <RenderArgs.h>
#include <ViewFrustum.h>
#include <gpu/Context.h>

#include "FramebufferCache.h"
#include "TextureCache.h"

#include <gpu/StandardShaderLib.h>

#include "nop_frag.h"

using namespace render;
extern void initForwardPipelines(ShapePlumber& plumber);
extern void initForwardOverlay3DPipelines(ShapePlumber& plumber);

RenderForwardTask::RenderForwardTask(RenderFetchCullSortTask::Output items) {
    // Prepare the ShapePipelines
    ShapePlumberPointer shapePlumber = std::make_shared<ShapePlumber>();
    initForwardPipelines(*shapePlumber);
    initForwardOverlay3DPipelines(*shapePlumber);

    // Extract opaques / transparents / lights / metas / overlays / background
    const auto opaques = items[RenderFetchCullSortTask::OPAQUE_SHAPE];
    const auto transparents = items[RenderFetchCullSortTask::TRANSPARENT_SHAPE];
    const auto lights = items[RenderFetchCullSortTask::LIGHT];
    const auto metas = items[RenderFetchCullSortTask::META];
    const auto overlayOpaques = items[RenderFetchCullSortTask::OVERLAY_OPAQUE_SHAPE];
    const auto overlayTransparents = items[RenderFetchCullSortTask::OVERLAY_TRANSPARENT_SHAPE];
    const auto background = items[RenderFetchCullSortTask::BACKGROUND];
    const auto spatialSelection = items[RenderFetchCullSortTask::SPATIAL_SELECTION];

    const auto framebuffer = addJob<PrepareFramebuffer>("PrepareFramebuffer");

    const auto lightingModel = addJob<MakeLightingModel>("LightingModel");

    const auto opaqueInputs = DrawDeferred::Inputs(opaques, lightingModel).hasVarying();
    addJob<Draw>("DrawOpaques", opaqueInputs, shapePlumber);
    
    addJob<Stencil>("Stencil");
    addJob<DrawBackground>("DrawBackground", background);
    
    // Bounds do not draw on stencil buffer, so they must come last
    //addJob<DrawBounds>("DrawBounds", opaques);
    
    // Overlays
    const auto overlayOpaquesInputs = DrawOverlay3D::Inputs(overlayOpaques, lightingModel).hasVarying();
    const auto overlayTransparentsInputs = DrawOverlay3D::Inputs(overlayTransparents, lightingModel).hasVarying();
    addJob<DrawOverlay3D>("DrawOverlay3DOpaque", overlayOpaquesInputs, true);
    addJob<DrawOverlay3D>("DrawOverlay3DTransparent", overlayTransparentsInputs, false);
    
    // Render transparent objects forward in LightingBuffer
    const auto transparentsInputs = DrawDeferred::Inputs(transparents, lightingModel).hasVarying();
    addJob<DrawTransparentDeferred>("DrawTransparentDeferred", transparentsInputs, shapePlumber);
    
    // Blit!
    addJob<Blit>("Blit", framebuffer);
}

void PrepareFramebuffer::run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext,
        gpu::FramebufferPointer& framebuffer) {
    auto framebufferCache = DependencyManager::get<FramebufferCache>();
    auto framebufferSize = framebufferCache->getFrameBufferSize();
    glm::uvec2 frameSize(framebufferSize.width(), framebufferSize.height());

    // Resizing framebuffers instead of re-building them seems to cause issues with threaded rendering
    if (_framebuffer && _framebuffer->getSize() != frameSize) {
        _framebuffer.reset();
    }

    if (!_framebuffer) {
        _framebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("forward"));

        auto colorFormat = gpu::Element::COLOR_SRGBA_32;
        auto defaultSampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_POINT);
        auto colorTexture = gpu::TexturePointer(gpu::Texture::create2D(colorFormat, frameSize.x, frameSize.y, defaultSampler));
        _framebuffer->setRenderBuffer(0, colorTexture);

        auto depthFormat = gpu::Element(gpu::SCALAR, gpu::UINT32, gpu::DEPTH_STENCIL); // Depth24_Stencil8 texel format
        auto depthTexture = gpu::TexturePointer(gpu::Texture::create2D(depthFormat, frameSize.x, frameSize.y, defaultSampler));
        _framebuffer->setDepthStencilBuffer(depthTexture, depthFormat);
    }

    auto args = renderContext->args;
    gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
        batch.enableStereo(false);
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);

        batch.setFramebuffer(_framebuffer);
        batch.clearFramebuffer(
            gpu::Framebuffer::BUFFER_COLOR0 |
            gpu::Framebuffer::BUFFER_DEPTH |
            gpu::Framebuffer::BUFFER_STENCIL,
            vec4(vec3(0), 1), 1.0, 0.0, true);
    });

    framebuffer = _framebuffer;
}

void Draw::run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext,
        const Inputs& inputs) {
    RenderArgs* args = renderContext->args;

    const auto& inItems = inputs.get0();
    const auto& lightingModel = inputs.get1();

    gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;

        // Setup projection
        glm::mat4 projMat;
        Transform viewMat;
        args->getViewFrustum().evalProjectionMatrix(projMat);
        args->getViewFrustum().evalViewTransform(viewMat);
        batch.setProjectionTransform(projMat);
        batch.setViewTransform(viewMat);
        //batch.setModelTransform(Transform());

        // Setup lighting model for all items;
        batch.setUniformBuffer(render::ShapePipeline::Slot::LIGHTING_MODEL, lightingModel->getParametersBuffer());

        // Render items
        renderStateSortShapes(sceneContext, renderContext, _shapePlumber, inItems, -1);
    });
    args->_batch = nullptr;
}

void DrawTransparentDeferred::run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const Inputs& inputs) {
    PROFILE_RANGE_EX(render, "DrawTransparentDeferred", 0xff555500, (uint64_t)1)
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    const auto& inItems = inputs.get0();
    const auto& lightingModel = inputs.get1();
    RenderArgs* args = renderContext->args;
    gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;
        
        // Setup camera, projection and viewport for all items
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);
        glm::mat4 projMat;
        Transform viewMat;
        args->getViewFrustum().evalProjectionMatrix(projMat);
        args->getViewFrustum().evalViewTransform(viewMat);
        batch.setProjectionTransform(projMat);
        batch.setViewTransform(viewMat);
        // Setup lighting model for all items;
        batch.setUniformBuffer(render::ShapePipeline::Slot::LIGHTING_MODEL, lightingModel->getParametersBuffer());
        renderShapes(sceneContext, renderContext, _shapePlumber, inItems, -1);//_maxDrawn);
        args->_batch = nullptr;
    });
}

const gpu::PipelinePointer Stencil::getPipeline() {
    if (!_stencilPipeline) {
        auto vs = gpu::StandardShaderLib::getDrawUnitQuadTexcoordVS();
        auto ps = gpu::Shader::createPixel(std::string(nop_frag));
        gpu::ShaderPointer program = gpu::Shader::createProgram(vs, ps);
        gpu::Shader::makeProgram(*program);

        auto state = std::make_shared<gpu::State>();
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        const gpu::int8 STENCIL_OPAQUE = 1;
        state->setStencilTest(true, 0xFF, gpu::State::StencilTest(STENCIL_OPAQUE, 0xFF, gpu::ALWAYS,
                    gpu::State::STENCIL_OP_REPLACE,
                    gpu::State::STENCIL_OP_REPLACE,
                    gpu::State::STENCIL_OP_KEEP));

        _stencilPipeline = gpu::Pipeline::create(program, state);
    }
    return _stencilPipeline;
}

void Stencil::run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) {
    RenderArgs* args = renderContext->args;

    gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;

        batch.enableStereo(false);
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);

        batch.setPipeline(getPipeline());
        batch.draw(gpu::TRIANGLE_STRIP, 4);
    });
    args->_batch = nullptr;
}

void DrawBackground::run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext,
        const Inputs& background) {
    RenderArgs* args = renderContext->args;

    gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;

        batch.enableSkybox(true);
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);

        // Setup projection
        /*glm::mat4 projMat;
        Transform viewMat;
        args->getViewFrustum().evalProjectionMatrix(projMat);
        args->getViewFrustum().evalViewTransform(viewMat);
        batch.setProjectionTransform(projMat);
        batch.setViewTransform(viewMat);*/

        renderItems(sceneContext, renderContext, background);
    });
    args->_batch = nullptr;
}


