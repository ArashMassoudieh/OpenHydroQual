/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QMap>
#include <QString>

struct TimeSeries {
    QList<double> t;
    QList<double> value;
};

using TimeSeriesMap = QMap<QString, TimeSeries>;

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(const QUrl& endpoint, QObject* parent = nullptr);
    void fetchAllSeries(const QJsonObject& requestData);

signals:
    void dataReady(const TimeSeriesMap& data);
    void requestFailed(const QString& error);

private slots:
    void handleReply(QNetworkReply* reply);

private:
    QNetworkAccessManager manager;
    QUrl url;
};

#endif // APICLIENT_H
