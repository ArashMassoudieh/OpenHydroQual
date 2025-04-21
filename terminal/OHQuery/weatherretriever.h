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

struct PrecipitationData {
    QDateTime dateTime;  // Timestamp of the measurement
    double precipitation; // Precipitation value in mm
};

class WeatherRetriever
{
public:
    WeatherRetriever();
    CPrecipitation RetrivePrecip(const QPointF location, const QString &FIPS);
    QMap<QString, WeatherStationData> fetchPrecipitationStations(const QString& FIPS);
    void SetAPIToken(const QString &token) {apiToken = token; }
    QMap<QString, WeatherStationData> findClosestStations(const QMap<QString, WeatherStationData>& stations, double latitude, double longitude, int n);
    WeatherStationData findLongestRecordStation(const QMap<QString, WeatherStationData>& stations);
    WeatherStationData findStation( const double &latitude, const double &longitude, const QString &FIPS);
    QVector<PrecipitationData> fetchPrecipitationData(const QString& stationId, const QString& startDate, const QString& endDate);
private:
    QString apiToken = "";
};

double haversine(double lat1, double lon1, double lat2, double lon2);
double qDateTimeToExcel(const QDateTime& dateTime);
QDateTime excelToQDateTime(double excelDate);
#endif // WEATHERRETRIEVER_H
