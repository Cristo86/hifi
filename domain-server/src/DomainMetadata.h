//
//  DomainMetadata.h
//  domain-server/src
//
//  Created by Zach Pomerantz on 5/25/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#ifndef hifi_DomainMetadata_h
#define hifi_DomainMetadata_h

#include <QVariantMap>
#include <QJsonObject>

class DomainMetadata {
public:
    QVariantMap toVariantMap() { generate(); return _metadata; }
    QJsonObject toJSON() { generate(); return QJsonObject::fromVariantMap(_metadata); }

protected slots:
    // TODO: Connect appropriate signals to obviate JIT generation
    void generate();

protected:
    QVariantMap _metadata;
};

#endif // hifi_DomainMetadata_h
