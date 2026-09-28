// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QNetworkReply>
#include <QTimer>

namespace NetworkSafety {
// For small JSON replies that are consumed at finished(). Enforce the limit
// while receiving, including chunked/decompressed bodies and slow responses.
inline void boundJsonReply(QNetworkReply *reply, qint64 limit, int deadlineMs = 30000)
{
    reply->setReadBufferSize(limit + 1);
    const auto check = [reply, limit] {
        if (reply->bytesAvailable() > limit ||
            reply->header(QNetworkRequest::ContentLengthHeader).toLongLong() > limit) {
            reply->setProperty("lmgResponseTooLarge", true);
            reply->abort();
        }
    };
    QObject::connect(reply, &QIODevice::readyRead, reply, check);
    QObject::connect(reply, &QNetworkReply::metaDataChanged, reply, check);
    auto *deadline = new QTimer(reply);
    deadline->setSingleShot(true);
    QObject::connect(deadline, &QTimer::timeout, reply, [reply] {
        reply->setProperty("lmgDeadlineExceeded", true);
        reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::finished, deadline, &QTimer::stop);
    deadline->start(deadlineMs);
}
}
