//
//  GPUConfig.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 12/4/14.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_gpu_GPUConfig_h
#define hifi_gpu_GPUConfig_h

#define GL_GLEXT_PROTOTYPES 1

#include <GLES2/gl2ext.h>
#include <GLES3/gl31.h>
#include <EGL/egl.h>

#if defined(__APPLE__)

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/OpenGL.h>

#endif

#if defined(WIN32)

#include <GL/wglew.h>

#endif

#endif // hifi_gpu_GPUConfig_h
