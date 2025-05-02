#include "weatherretriever.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QJsonDocument>

WeatherRetriever::WeatherRetriever()
{

}

CPrecipitation WeatherRetriever::RetrivePrecipNOAA(const QPointF location, const QString &FIPS)
{
    CPrecipitation out;
    WeatherStationData stationwithlongestdata = findStationNOAA(location.x(),location.y(),"24");
    qDebug()<<"Station with longest record:";
    qDebug()<<stationwithlongestdata.name << ": Longitude: " <<stationwithlongestdata.longitude << ": Latitude: " << stationwithlongestdata.latitude << ": Time span [" <<stationwithlongestdata.mindate << "," << stationwithlongestdata.maxdate << "]";
    QVector<WeatherDataPoint> data = fetchPrecipitationDataNOAA(stationwithlongestdata.id,"2012-01-01","2013-01-01");
    for (int i=0; i<data.count(); i++)
    {
        qDebug()<<data[i].dateTime<<","<<data[i].value;
        out.append(qDateTimeToExcel(data[i].dateTime)-1.0/24.0,qDateTimeToExcel(data[i].dateTime),data[i].value*0.254/100);
    }
    return out;
}

CPrecipitation WeatherRetriever::RetrivePrecipNOAA(const double &startdate, const double &enddate, const QPointF location, const QString &FIPS)
{
    CPrecipitation out;
    WeatherStationData stationwithlongestdata = findStationNOAA(location.x(),location.y(),"24");
    qDebug()<<"Station with longest record:";
    qDebug()<<stationwithlongestdata.name << ": Longitude: " <<stationwithlongestdata.longitude << ": Latitude: " << stationwithlongestdata.latitude << ": Time span [" <<stationwithlongestdata.mindate << "," << stationwithlongestdata.maxdate << "]";
    QString StartDate = excelToQDateTime(startdate).toString(Qt::ISODate);
    QString EndDate = excelToQDateTime(enddate).toString(Qt::ISODate);
    QVector<WeatherDataPoint> data = fetchPrecipitationDataNOAA(stationwithlongestdata.id,StartDate,EndDate);
    for (int i=0; i<data.count(); i++)
    {
        out.append(qDateTimeToExcel(data[i].dateTime)-1.0/24.0,qDateTimeToExcel(data[i].dateTime),data[i].value*0.254/100);
    }
    return out;
}

CPrecipitation WeatherRetriever::RetrivePrecipOpenMeteo(const double &startdate, const double &enddate, const QPointF location)
{
    CPrecipitation out;

    qDebug() << ": Longitude: " <<location.y() << ": Latitude: " << location.x() << ": Time span [" <<startdate << "," << enddate << "]";
    QDate StartDate = excelToQDateTime(startdate).date();
    QDate EndDate = excelToQDateTime(enddate).date();
    QVector<WeatherDataPoint> data = fetchWeatherDataOpenMeteo(location.x(), location.y(), StartDate,EndDate, "precipitation");
    for (int i=0; i<data.count(); i++)
    {
        //qDebug()<<data[i].dateTime<<","<<data[i].value;
        out.append(qDateTimeToExcel(data[i].dateTime)-1.0/24.0,qDateTimeToExcel(data[i].dateTime),data[i].value/1000.0);
    }
    return out;
}


QVector<WeatherDataPoint> WeatherRetriever::fetchPrecipitationDataNOAA(const QString& stationId, const QString& startDate, const QString& endDate) {
    QVector<WeatherDataPoint> dataList;

    // NOAA API endpoint
    QString url;
    url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/data?datasetid=PRECIP_HLY&stationid=%1&datatypeid=HPCP&startdate=%2&enddate=%3&limit=1000").arg(stationId).arg(startDate).arg(endDate);
    qDebug()<<url;

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Create the request with API token
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("token", apiToken.toUtf8());

    // Make the GET request
    QNetworkReply* reply = manager.get(request);

    // Use QEventLoop to wait for the response
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        qDebug()<<"No Error";
        QByteArray responseData = reply->readAll();
        qDebug()<<responseData;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray resultsArray = rootObj["results"].toArray();

            // Iterate over the results and extract precipitation data
            for (const QJsonValue& resultValue : resultsArray) {
                QJsonObject resultObj = resultValue.toObject();
                QString dateTimeStr = resultObj["date"].toString();
                double precipitation = resultObj["value"].toDouble() / 10.0; // NOAA stores precipitation in tenths of mm

                WeatherDataPoint data;
                data.dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
                data.value = precipitation;

                dataList.append(data);
            }
        } else {
            qDebug() << "Failed to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching precipitation data:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();
    return dataList;
}



QMap<QString, WeatherStationData> WeatherRetriever::fetchPrecipitationStationsNOAA(const QString& FIPS) {
    QMap<QString, WeatherStationData> stationList;

    // NOAA API endpoint
    QString url;
    url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/stations?datasetid=PRECIP_HLY&datatypeid=HPCP&locationid=FIPS:%1&limit=1000").arg(FIPS);
    qDebug()<<url;
    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Create the request with API token
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("token", apiToken.toUtf8());

    // Make the GET request
    QNetworkReply* reply = manager.get(request);

    // Use QEventLoop to wait for the response
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray resultsArray = rootObj["results"].toArray();

            // Iterate over the stations and populate the QVector
            for (const QJsonValue& stationValue : resultsArray) {
                QJsonObject stationObj = stationValue.toObject();
                WeatherStationData metadata;
                qDebug()<<stationObj;
                metadata.id = stationObj["id"].toString();
                qDebug()<<metadata.id;
                metadata.name = stationObj["name"].toString();
                metadata.latitude = stationObj.contains("latitude") ? stationObj["latitude"].toDouble() : 0.0;
                metadata.longitude = stationObj.contains("longitude") ? stationObj["longitude"].toDouble() : 0.0;
                metadata.elevation = stationObj.contains("elevation") ? stationObj["elevation"].toDouble() : 0.0;
                metadata.mindate = stationObj["mindate"].toString();
                metadata.maxdate = stationObj["maxdate"].toString();
                metadata.datacoverage = stationObj.contains("datacoverage") ? stationObj["datacoverage"].toDouble() : 0.0;

                stationList[metadata.id] = metadata;
            }
        } else {
            qDebug() << "Failed to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching station metadata:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();
    return stationList;
}

QMap<QString, WeatherStationData> WeatherRetriever::findClosestStationsNOAA(
    const QMap<QString, WeatherStationData>& stations,
    double latitude,
    double longitude,
    int n)
{
    QVector<std::pair<double, QString>> distances;

    // Compute distances and store them with station IDs
    for (auto it = stations.constBegin(); it != stations.constEnd(); ++it) {
        double dist = haversine(latitude, longitude, it.value().latitude, it.value().longitude);
        distances.append({dist, it.key()});
    }

    // Sort by distance
    std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    // Construct result map
    QMap<QString, WeatherStationData> result;
    for (int i = 0; i < std::min(n, int(distances.size())); ++i) {
        const QString& id = distances[i].second;
        result.insert(id, stations[id]);
    }

    return result;
}

WeatherStationData WeatherRetriever::findLongestRecordStationNOAA(const QMap<QString, WeatherStationData>& stations) {
    int maxDays = -1;
    WeatherStationData longest;

    for (const auto& station : stations) {
        QDate minDate = QDate::fromString(station.mindate, "yyyy-MM-dd");
        QDate maxDate = QDate::fromString(station.maxdate, "yyyy-MM-dd");

        if (!minDate.isValid() || !maxDate.isValid())
            continue;

        int days = minDate.daysTo(maxDate);
        if (days > maxDays) {
            maxDays = days;
            longest = station;
        }
    }

    return longest;
}

// Helper function to compute Haversine distance between two lat/lon points
double haversine(double lat1, double lon1, double lat2, double lon2) {
    constexpr double R = 6371.0; // Earth's radius in kilometers
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLon = qDegreesToRadians(lon2 - lon1);
    lat1 = qDegreesToRadians(lat1);
    lat2 = qDegreesToRadians(lat2);

    double a = std::pow(std::sin(dLat / 2), 2) +
               std::pow(std::sin(dLon / 2), 2) *
               std::cos(lat1) * std::cos(lat2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

WeatherStationData WeatherRetriever::findStationNOAA( const double &latitude, const double &longitude, const QString &FIPS)
{
    QMap<QString, WeatherStationData> stations =  fetchPrecipitationStationsNOAA(FIPS);
    qDebug()<<"All Stations:";
    for (QString stationkey: stations.keys())
    {
        qDebug()<<stationkey << ": Longitude: " <<stations[stationkey].longitude << ": Latitude: " << stations[stationkey].latitude << ": Time span [" <<stations[stationkey].mindate << "," << stations[stationkey].maxdate << "]";
    }

    QMap<QString, WeatherStationData> closeststations = findClosestStationsNOAA(stations,latitude,longitude,3);
    qDebug()<<"Closest 3 stations:";
    for (QString stationkey: closeststations.keys())
    {
        qDebug()<<stationkey << ": Longitude: " <<closeststations[stationkey].longitude << ": Latitude: " << closeststations[stationkey].latitude << ": Time span [" <<closeststations[stationkey].mindate << "," << closeststations[stationkey].maxdate << "]";
    }
    WeatherStationData stationwithlongestdata = findLongestRecordStationNOAA(closeststations);
    qDebug()<<"Station with longest record:";
    qDebug()<<stationwithlongestdata.name << ": Longitude: " <<stationwithlongestdata.longitude << ": Latitude: " << stationwithlongestdata.latitude << ": Time span [" <<stationwithlongestdata.mindate << "," << stationwithlongestdata.maxdate << "]";
    StationName = stationwithlongestdata.id;
    return stationwithlongestdata;
}

double qDateTimeToExcel(const QDateTime& dateTime) {
    // Define the base date for Excel (January 1, 1900)
    QDate excelBaseDate(1900, 1, 1);

    // Check if the date is valid and before the Excel base date
    if (!dateTime.isValid() || dateTime.date() < excelBaseDate) {
        qDebug() << "Invalid date or date is before the Excel base date.";
        return 0.0;
    }

    // Calculate the number of days between the Excel base date and the given date
    qint64 daysSinceExcelBase = excelBaseDate.daysTo(dateTime.date());

    // Excel incorrectly treats 1900 as a leap year; we need to add an extra day for dates after February 28, 1900
    if (dateTime.date() > QDate(1900, 2, 28)) {
        daysSinceExcelBase += 1;
    }

    // Calculate the fraction of the day for the time portion
    qint64 secondsSinceMidnight = dateTime.time().secsTo(QTime(0, 0, 0));
    double fractionalDay = 1.0 - static_cast<double>(secondsSinceMidnight) / 86400.0;

    // Combine the days and the fractional day to get the Excel date/time number
    double excelDateTime = static_cast<double>(daysSinceExcelBase) + fractionalDay;

    return excelDateTime;
}

QDateTime excelToQDateTime(double excelDate) {
    if (excelDate < 1.0) {
        qDebug() << "Invalid Excel date. Must be >= 1.0.";
        return QDateTime();
    }

    // Excel base date: January 1, 1900
    QDate excelBaseDate(1900, 1, 1);

    // Excel leap year bug: Skip February 29, 1900 (nonexistent day)
    int days = static_cast<int>(excelDate); // Integer part represents days
    if (days >= 60) {
        days -= 1; // Correct for Excel's leap year bug
    }

    // Add the integer days to the base date
    QDate convertedDate = excelBaseDate.addDays(days - 1);

    // Extract the fractional part for time
    double fractionalDay = excelDate - static_cast<int>(excelDate);
    int totalSeconds = static_cast<int>(fractionalDay * 86400); // Convert fraction to seconds
    QTime convertedTime = QTime(0, 0, 0).addSecs(totalSeconds);

    // Combine the date and time
    return QDateTime(convertedDate, convertedTime);
}

QVector<WeatherDataPoint> WeatherRetriever::fetchWeatherDataOpenMeteo(double latitude, double longitude, const QDate &startDate, const QDate &endDate, const QString &dataType, QObject* parent)
{
    QVector<WeatherDataPoint> results;

    QString urlString = QString("https://archive-api.open-meteo.com/v1/era5?"
                                "latitude=%1&longitude=%2&start_date=%3&end_date=%4"
                                "&hourly=%5")
                            .arg(latitude)
                            .arg(longitude)
                            .arg(startDate.toString(Qt::ISODate))
                            .arg(endDate.toString(Qt::ISODate))
                            .arg(QUrl::toPercentEncoding(dataType));

    QUrl url(urlString);
    QNetworkAccessManager manager(parent);
    QNetworkRequest request(url);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return results;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return results;
    }

    QJsonObject root = doc.object();
    QJsonObject hourly = root.value("hourly").toObject();
    QJsonArray timeArray = hourly.value("time").toArray();
    QJsonArray valueArray = hourly.value(dataType).toArray();

    if (timeArray.size() != valueArray.size()) {
        qWarning() << "Mismatched time and data array sizes.";
        return results;
    }

    for (int i = 0; i < timeArray.size(); ++i) {
        QDateTime dt = QDateTime::fromString(timeArray.at(i).toString(), Qt::ISODate);
        dt.setTimeSpec(Qt::UTC);
        double val = valueArray.at(i).toDouble();
        results.append({dt, val});
    }

    return results;
}
