//
//  Light.cpp
//  libraries/model/src/model
//
//  Created by Sam Gateau on 1/26/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "Light.h"

using namespace model;

Light::Light() {
    // only if created from nothing shall we create the Buffer to store the properties
    Schema schema;
    _schemaBuffer = std::make_shared<gpu::Buffer>(sizeof(Schema), (const gpu::Byte*) &schema);
    updateLightRadius();
}

Light::Light(const Light& light) :
    _flags(light._flags),
    _schemaBuffer(light._schemaBuffer),
    _transform(light._transform),
    _maximumRadius(light._maximumRadius) {
}

Light& Light::operator= (const Light& light) {
    _flags = (light._flags);
    _schemaBuffer = (light._schemaBuffer);
    _transform = (light._transform);
    _maximumRadius = (light._maximumRadius);

    return (*this);
}

Light::~Light() {
}

void Light::setPosition(const Vec3& position) {
    _transform.setTranslation(position);
    editSchema()._position = Vec4(position, 1.f);
}

void Light::setOrientation(const glm::quat& orientation) {
    setDirection(orientation * glm::vec3(0.0f, 0.0f, -1.0f));
    _transform.setRotation(orientation);
}

void Light::setDirection(const Vec3& direction) {
    editSchema()._direction = glm::normalize(direction);
}

const Vec3& Light::getDirection() const {
    return getSchema()._direction;
}

void Light::setColor(const Color& color) {
    editSchema()._color = color;
    updateLightRadius();
}

void Light::setIntensity(float intensity) {
    editSchema()._intensity = intensity;
    updateLightRadius();
}

void Light::setAmbientIntensity(float intensity) {
    editSchema()._ambientIntensity = intensity;
}

void Light::setSurfaceRadius(float radius) {
    if (radius <= 0.0f) {
        radius = 0.1f;
    }
    editSchema()._attenuation.x = radius;
    updateLightRadius();
}
void Light::setMaximumRadius(float radius) {
    if (radius <= 0.f) {
        radius = 1.0f;
    }
    editSchema()._attenuation.y = radius;
    updateLightRadius();
}

void Light::updateLightRadius() {
    float intensity = getIntensity() * std::max(std::max(getColor().x, getColor().y), getColor().z);
    float maximumDistance = getMaximumRadius() - getSurfaceRadius();

    float denom = maximumDistance / getSurfaceRadius() + 1;

    // The cutoff intensity biases the light towards the source.
    // If the source is small and the intensity high, many points may not be shaded.
    // If the intensity is >=1.0, the lighting attenuation equation gets borked (see Light.slh).
    // To maintain sanity, we cap it well before then.
    const float MAX_CUTOFF_INTENSITY = 0.01f; // intensity = maximumRadius = 1.0f, surfaceRadius = 0.1f
    float cutoffIntensity = std::min(intensity / (denom * denom), MAX_CUTOFF_INTENSITY);

    editSchema()._attenuation.z = cutoffIntensity;
}

#include <math.h>

void Light::setSpotAngle(float angle) {
    double dangle = angle;
    if (dangle <= 0.0) {
        dangle = 0.0;
    }
    if (dangle > glm::half_pi<double>()) {
        dangle = glm::half_pi<double>();
    }

    auto cosAngle = cos(dangle);
    auto sinAngle = sin(dangle);
    editSchema()._spot.x = (float) std::abs(cosAngle);
    editSchema()._spot.y = (float) std::abs(sinAngle);
    editSchema()._spot.z = (float) angle;
}

void Light::setSpotExponent(float exponent) {
    if (exponent <= 0.f) {
        exponent = 0.0f;
    }
    editSchema()._spot.w = exponent;
}

void Light::setShowContour(float show) {
    if (show <= 0.f) {
        show = 0.0f;
    }
    editSchema()._control.w = show;
}



