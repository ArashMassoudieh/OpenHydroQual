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


#ifndef WEATHERRETRIEVER_H
#define WEATHERRETRIEVER_H

#include "BTC.h"
#include <QPointF>
#include "Precipitation.h"


struct WeatherStationData {
    QString id;         // Station ID
    QString name;       // Station Name
    double latitude;    // Latitude
    double longitude;   // Longitude
    double elevation;   // Elevation in meters
    QString mindate;    // Earliest data available
    QString maxdate;    // Latest data available
    double datacoverage; // Data coverage percentage};
};

struct WeatherDataPoint {
    QDateTime dateTime;  // Timestamp of the measurement
    double value; // values
};

class WeatherRetriever
{
public:
    WeatherRetriever();
    CPrecipitation RetrivePrecipNOAA(const QPointF location, const QString &FIPS);
    CPrecipitation RetrivePrecipNOAA(const double &startdate, const double &enddate, const QPointF location, const QString &FIPS);
    QMap<QString, WeatherStationData> fetchPrecipitationStationsNOAA(const QString& FIPS);
    void SetAPIToken(const QString &token) {apiToken = token; }
    QMap<QString, WeatherStationData> findClosestStationsNOAA(const QMap<QString, WeatherStationData>& stations, double latitude, double longitude, int n);
    WeatherStationData findLongestRecordStationNOAA(const QMap<QString, WeatherStationData>& stations);
    WeatherStationData findStationNOAA( const double &latitude, const double &longitude, const QString &FIPS);
    QVector<WeatherDataPoint> fetchPrecipitationDataNOAA(const QString& stationId, const QString& startDate, const QString& endDate);
    QString SelectedStation() {return StationName;}
    QVector<WeatherDataPoint> fetchWeatherDataOpenMeteo(double latitude, double longitude, const QDate &startDate, const QDate &endDate, const QString &dataType, QObject* parent = nullptr);
    CPrecipitation RetrivePrecipOpenMeteo(const double &startdate, const double &enddate, const QPointF location);
private:
    QString apiToken = "";
    QString StationName = "";
};

double haversine(double lat1, double lon1, double lat2, double lon2);
double qDateTimeToExcel(const QDateTime& dateTime);
QDateTime excelToQDateTime(double excelDate);
#endif // WEATHERRETRIEVER_H
