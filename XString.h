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
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#pragma once
#include <qstring.h>
#include "qstringlist.h"
#include "qmath.h"
#include "qdebug.h"
#include "enums.h"
#include <iostream>

class XString :
	public QString
{
public:
	XString() :QString() {};
	~XString() {};
	XString(const XString &X) : QString(X) { 
		unit = X.unit; 
		defaultUnit = X.defaultUnit;
		unitsList = X.unitsList; 
//		if (defaultUnit == ""  && unitsList.size()) defaultUnit = unitsList.first();
	};
	XString(const double &X);
	XString(const int &X);
    XString& operator+=(const XString x);
	XString operator=(const double x);
	XString operator=(const int x);
	XString(const QString &X) : QString(reform(X)) {};
	XString(const QStringList &XU) : QString(reform(XU[0])) {
		QStringList QL = XU;
		QL.removeFirst();
		if (QL.size()){
			unit = reform(QL.first());
			defaultUnit = unit;
			QL.removeFirst();
		}
		else{
			unit = "";
			defaultUnit = "";
		}
		if (QL.size()){
			defaultUnit = QL.last();
			QL.removeLast();
			setUnitsList(QL);
		}
	};

	XString(const QString &X, const QStringList &UL, const QString &defaultUnit) : QString(reform(X)) { unit = ""; setUnitsList(UL); this->defaultUnit = reform(defaultUnit); }; //only to set unitslist and default unit from mFile
	XString(const double &D, const QString &Unit, const QStringList &UL, const QString &defaultUnit){ *this = D;	unit = reform(Unit); setUnitsList(UL);	this->defaultUnit = defaultUnit; };
	XString(const QString &X, const QString &Unit, const QStringList &UL, const QString &defaultUnit) : QString(reform(X)) { unit = reform(Unit); setUnitsList(UL); this->defaultUnit = reform(defaultUnit); };
	XString(const char &X) : QString(reform(QString(X))) {};
	XString(const char X[]) : QString(reform(QString(X))) {};
	XString& operator= (const XString &X) { QString::operator= (X);		unit = X.unit;		unitsList = X.unitsList;	defaultUnit = X.defaultUnit;	return *this; };
	bool operator == (const XString &X) const { return (toQString() == X.toQString()) && (unit == X.unit);
	};
	bool operator != (const XString &X) const { return !((toQString() == X.toQString()) && (unit == X.unit));
	};
	bool operator == (const char Q[]) const { return (toQString() == Q); };
	bool operator != (const char Q[]) const { return !(toQString() == Q); };

	QStringList unitsList;
	QString unit, defaultUnit;
    QStringList valUnit() const { QStringList R; R << *this << unit; return R; };
//	QString value() const { return *this; };
	QString convertToDefaultUnit() const {
		return convertTo(defaultUnit);
	};
	void setString(const QString &Q) { *this = Q;};
	
	QString toQString() const { return *this; };
	std::string toStdString() const { return toQString().toStdString(); };
    QString fullFilename(QString fileWithRelativePath, QString  dir) const
    {
        return fileWithRelativePath.replace("./", dir.append('/'));
    }
    QString toFileName(QString path) const { return fullFilename(*this,path); };
	double toDouble(QString unit = "") const {
		if (unit =="") return toQString().toDouble();
		else return convertTo(reform(unit)).toDouble();
	};
	double toFloat(QString unit = "") const {return toDouble(unit);};
	double toFloatDefaultUnit() const { return convertToDefaultUnit().toDouble(); };
	bool toBool() const {
		QStringList trueList, falseList;
		trueList << "true" << "yes" << "on" << "1";
		falseList << "flase" << "no" << "off" << "0";
		if (trueList.contains(toQString().toLower()))
			return true;
		if (falseList.contains(toQString().toLower()))
			return false;
		return bool();
	}
	int toSolutionMethod() const {
		return 0;
		}

    QString toStringUnit() const { return (unit != "") ? QString("%1 (%2)").arg(*this).arg(unit) : QString(*this);	}
	QStringList list() const { 
		QStringList R, R1; 
		R << toQString() << unit;
		R1 << unitsList << defaultUnit;
		for (int i = 0; i < R1.size(); i++)
			if (R1[i].trimmed().size())
				R.append(R1[i]);
		return R; };
	void setValUnit(const QStringList &QL) { *this = QL[0]; unit = QL[1]; };
	void setUnitsList(const QStringList &UL) { 
		unitsList = QStringList(); 
		for (int i = 0; i < UL.size(); i++) 
			if (reform(UL[i]).trimmed().size())
				unitsList.append(reform(UL[i]));
	};
	QString compact() const {
		if (unit == "") return *this;
		QString r = "%compacted%";
		r.append(QString("%1;").arg(*this));
		r.append(QString("%1;").arg(unit));
        for (QString u : unitsList)
			r.append(QString("%1;").arg(u));
		r.append(QString("%1;").arg(defaultUnit));
		return r;
	};
	static XString unCompact(const QString &c) {
		QString q = c;
		if (!c.contains("%compacted%")) return XString(c);
		QStringList QSL1 = q.remove("%compacted%").split(";");
		QStringList QSL;
		for (int i = 0; i < QSL1.size(); i++)
			if (QSL1[i].trimmed().size())
				QSL.append(QSL1[i]);
		XString r;
		r.setString(QSL.first());
		QSL.removeFirst();
		if (QSL.size()){
			r.unit = QSL.first();
			r.defaultUnit = r.unit;
			QSL.removeFirst();
		}
		else{
			r.unit = "";
			r.defaultUnit = "";
		}
		if (QSL.size()){
			r.defaultUnit = QSL.last();
			QSL.removeLast();
			r.setUnitsList(QSL);
		}
		return r;
	}	
	static XString unCompactOld(const QString &c) {
		QString q = c;
		if (!c.contains("%compacted%")) return XString(c);
		QStringList QSL1 = q.remove("%compacted%").split(";");
		QStringList QSL;
		for (int i = 0; i < QSL1.size(); i++)
			if (QSL1[i].trimmed().size())
				QSL.append(QSL1[i]);
		XString r;
		r.setString(QSL.first());
		QSL.removeFirst();
		if (QSL.size()){
			r.unit = QSL.first();
			r.defaultUnit = r.unit;
			if (r.unit == "s/~^3~radicm") r.defaultUnit = "day/~^3~radicm";
			QSL.removeFirst();
		}
		else{
			r.unit = "";
			r.defaultUnit = "";
		}
		if (QSL.size()){
//			r.defaultUnit = QSL.last();
//			QSL.removeLast();
			if (r.unit == "s/~^3~radicm")
			{
				QSL.clear();
				QSL.append(r.unit);
			}

			r.setUnitsList(QSL);
		}
		return r;
	}

	// value[unit] 100[m^2]
	static XString fromQStringUnit(const QString &c) {
		////qDebug() << c.indexOf('[') << c.indexOf(']') << c.lastIndexOf('[') << c.lastIndexOf(']');
		QString valueTxt = c.left(c.indexOf('['));
        //double value = valueTxt.toDouble();

		if (c.indexOf('[')==-1 || c.indexOf(']')==-1 || c.indexOf('[') >= c.indexOf(']') ||
			c.indexOf('[') != c.lastIndexOf('[') || c.indexOf(']') != c.lastIndexOf(']'))
			return XString();

		QString unit = c.mid(c.indexOf('[')+1, c.indexOf(']') - c.indexOf('[')-1);
		if (!validUnit(unit))
			unit = "";
		//if (unit != "" && !unit.startsWith("~"))
		//	unit = QString("~%1").arg(unit);
		unit = reform(unit);
		return XString(valueTxt, unit, QStringList() << unit, unit);
	}


	XString convertTo(const QString _unit) const{
		if (unit == _unit) return XString(QString::number(toDouble()), _unit, unitsList, defaultUnit);
        //double a = conversionCoefficient(unit, _unit);
        //double b = toDouble() * conversionCoefficient(unit, _unit);
        //XString c = b;
        //QString d = QString::number(b);

		return XString(QString::number(toDouble() * conversionCoefficient(unit, _unit)), _unit, unitsList, defaultUnit);	};
	double conversionCoefficient(const XString &unit_from, const XString &unit_to) const{
		if (unit_from.isEmpty() || unit_to.isEmpty()) return 1;
        //float a = coefficient(unit_from.reformBack());
        //float b = coefficient(unit_to.reformBack());
		return coefficient(unit_from.reformBack()) / coefficient(unit_to.reformBack()); };
	static double coefficient(const QString expression) 	{
        QStringList Operators;
        QString leftOperand = expression, rightOperand, Operator;
        Operators << "/" << "." << "~^" << "^" << "~radic" << "~radical" << "~^3radic" << "~^3radical";
		if (containsOperator(leftOperand, rightOperand, Operator, Operators)){
            if (Operator == ".") return (coefficient(leftOperand) * coefficient(rightOperand));
            if (Operator == "/") return (coefficient(leftOperand) / coefficient(rightOperand));
            if (Operator == "^" || Operator == "~^") return qPow(coefficient(leftOperand), coefficient(rightOperand));
            if (Operator == "~radic" || Operator == "~radical") return (coefficient(leftOperand) * sqrt(coefficient(rightOperand)));
            if (Operator == "~^3radic" || Operator == "~^3radical") return (coefficient(leftOperand) * qPow(coefficient(rightOperand), 1.0 / 3.0));

		}
		else{
            if (leftOperand.toDouble()) return leftOperand.toDouble();
            QList <QString> UL;
			QList <float> CL;
            UL << "m" << "cm" << "mm" << "~microm" << "km" << "in" << "inch" << "ft" << "yd" <<
				"kg" << "g" << "ton" << "lb" << "kip" <<
				"day" << "hr" << "min" << "s" << "wk" <<
				"L" << "N" << 
                "~degreeC" << "Pa" << "J" << "W" << "si" << "SI" << "" << "" << "" << "" << "" << "" << "" << "" << "";
            CL << 1.0 << 0.01 << 0.001 << 0.000001 << 1000 << 2.54 / 100 << 2.54 / 100 << 12 * 2.54 / 100 << 3 * 12 * 2.54 / 100 <<
                1000 << 1 << 1000000 << 451000.0 / 1000 << 451000.0 <<
                1.0 << 1.0 / 24.0 << 1.0 / 24 / 60 << 1.0 / 24 / 60 / 60 << 7.0 <<
                .001 << 9.81
                << 1.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
            //double a = CL[UL.indexOf(leftOperand)];
            int _index = UL.indexOf(leftOperand);
            if (_index==-1)
                return 1;
            return CL[UL.indexOf(leftOperand)];
		};
        return 0;
    };
	static bool validUnit(const QString unit)  {
		if (coefficient(unit) != 0)
			return true;
		return false;
	}

	//set Unit not convert
	XString setUnit(const QString unit) {
		this->unit = reform(unit);
		return *this;
	}


    static bool containsOperator(QString &leftOperand, QString &rightOperand, QString &Operator, QStringList &Operators) 	{
        // For proper precedence, search for division/multiplication first (rightmost occurrence)
        // Then search for exponentiation (rightmost occurrence)
        for (int i = 0; i < Operators.size(); i++)
        {
            if (leftOperand.contains(Operators[i]))
            {
                int pos = leftOperand.lastIndexOf(Operators[i]);  // Changed to lastIndexOf for right-to-left
                if (pos >= 0)
                {
                    Operator = Operators[i];
                    rightOperand = leftOperand.right(leftOperand.size() - pos - Operator.size());
                    leftOperand = (pos) ? leftOperand.left(pos) : "1";
                    return true;
                }
            }
        }
        return false;
    }
    static QString reform(const QString &X)
    {
        QString R = X;

        // PREPROCESSING: Convert bare ^ to ~^ for backward compatibility
        // Only replace ^ that are NOT already preceded by ~
        if (R.contains("^") && !R.contains("~^"))
        {
            R.replace("^", "~^");
        }

        // Now process all the tildes
        if (!R.contains("~")) return R;  // Early exit if nothing to reform

        char16_t alpha = 945;	char16_t beta = 946;	char16_t gamma = 947;	char16_t delta = 948;	char16_t epsilon = 949;	char16_t zeta = 950;	char16_t eta = 951;	char16_t theta = 952;	char16_t iota = 953;	char16_t kappa = 954;	char16_t lambda = 955;
        char16_t mu = 956;	char16_t nu = 957;	char16_t xi = 958;	char16_t omicron = 959;	char16_t pi = 960;	char16_t rho = 961;	char16_t sigmaf = 962;	char16_t sigma = 963;	char16_t tau = 964;	char16_t upsilon = 965;	char16_t phi = 966;
        char16_t chi = 967;	char16_t psi = 968;	char16_t omega = 969;	char16_t thetasym = 977;	char16_t upsih = 978;	char16_t piv = 982;	char16_t sup2 = 178;	char16_t sup3 = 179;	char16_t frac14 = 188;	char16_t frac12 = 189;	char16_t frac34 = 190;
        char16_t radic = 8730;	char16_t degree = 176;

        R.replace("~alpha", QString::fromUtf16(&alpha, 1));	R.replace("~beta", QString::fromUtf16(&beta, 1));	R.replace("~gamma", QString::fromUtf16(&gamma, 1));
        R.replace("~delta", QString::fromUtf16(&delta, 1));	R.replace("~epsilon", QString::fromUtf16(&epsilon, 1));	R.replace("~zeta", QString::fromUtf16(&zeta, 1));	R.replace("~eta", QString::fromUtf16(&eta, 1));
        R.replace("~theta", QString::fromUtf16(&theta, 1));	R.replace("~iota", QString::fromUtf16(&iota, 1));	R.replace("~kappa", QString::fromUtf16(&kappa, 1));	R.replace("~lambda", QString::fromUtf16(&lambda, 1));
        R.replace("~mu", QString::fromUtf16(&mu, 1));	R.replace("~nu", QString::fromUtf16(&nu, 1));	R.replace("~xi", QString::fromUtf16(&xi, 1));	R.replace("~omicron", QString::fromUtf16(&omicron, 1));
        R.replace("~pi", QString::fromUtf16(&pi, 1));	R.replace("~rho", QString::fromUtf16(&rho, 1));	R.replace("~sigmaf", QString::fromUtf16(&sigmaf, 1));	R.replace("~sigma", QString::fromUtf16(&sigma, 1));
        R.replace("~tau", QString::fromUtf16(&tau, 1));	R.replace("~upsilon", QString::fromUtf16(&upsilon, 1));	R.replace("~phi", QString::fromUtf16(&phi, 1));	R.replace("~chi", QString::fromUtf16(&chi, 1));
        R.replace("~psi", QString::fromUtf16(&psi, 1));	R.replace("~omega", QString::fromUtf16(&omega, 1));	R.replace("~thetasym", QString::fromUtf16(&thetasym, 1));	R.replace("~upsih", QString::fromUtf16(&upsih, 1));
        R.replace("~piv", QString::fromUtf16(&piv, 1));	R.replace("~^2", QString::fromUtf16(&sup2, 1));	R.replace("~^3", QString::fromUtf16(&sup3, 1));	R.replace("~1/4", QString::fromUtf16(&frac14, 1));
        R.replace("~1/2", QString::fromUtf16(&frac12, 1));	R.replace("~3/4", QString::fromUtf16(&frac34, 1));	R.replace("~radic", QString::fromUtf16(&radic, 1));	R.replace("~degree", QString::fromUtf16(&degree, 1));

        return R;
    }
static	QString reformBack(QString R)
	{
		char16_t alpha = 945;	char16_t beta = 946;	char16_t gamma = 947;	char16_t delta = 948;	char16_t epsilon = 949;	char16_t zeta = 950;	char16_t eta = 951;	char16_t theta = 952;	char16_t iota = 953;	char16_t kappa = 954;	char16_t lambda = 955;
		char16_t mu = 956;	char16_t nu = 957;	char16_t xi = 958;	char16_t omicron = 959;	char16_t pi = 960;	char16_t rho = 961;	char16_t sigmaf = 962;	char16_t sigma = 963;	char16_t tau = 964;	char16_t upsilon = 965;	char16_t phi = 966;
		char16_t chi = 967;	char16_t psi = 968;	char16_t omega = 969;	char16_t thetasym = 977;	char16_t upsih = 978;	char16_t piv = 982;	char16_t sup2 = 178;	char16_t sup3 = 179;	char16_t frac14 = 188;	char16_t frac12 = 189;	char16_t frac34 = 190;
		char16_t radic = 8730;	char16_t degree = 176;	R.replace(QString::fromUtf16(&alpha, 1), "~alpha");	R.replace(QString::fromUtf16(&beta, 1), "~beta");	R.replace(QString::fromUtf16(&gamma, 1), "~gamma");
		R.replace(QString::fromUtf16(&delta, 1), "~delta");	R.replace(QString::fromUtf16(&epsilon, 1), "~epsilon");	R.replace(QString::fromUtf16(&zeta, 1), "~zeta");	R.replace(QString::fromUtf16(&eta, 1), "~eta");
		R.replace(QString::fromUtf16(&theta, 1), "~theta");	R.replace(QString::fromUtf16(&iota, 1), "~iota");	R.replace(QString::fromUtf16(&kappa, 1), "~kappa");	R.replace(QString::fromUtf16(&lambda, 1), "~lambda");
		R.replace(QString::fromUtf16(&mu, 1), "~mu");	R.replace(QString::fromUtf16(&nu, 1), "~nu");	R.replace(QString::fromUtf16(&xi, 1), "~xi");	R.replace(QString::fromUtf16(&omicron, 1), "~omicron");
		R.replace(QString::fromUtf16(&pi, 1), "~pi");	R.replace(QString::fromUtf16(&rho, 1), "~rho");	R.replace(QString::fromUtf16(&sigmaf, 1), "~sigmaf");	R.replace(QString::fromUtf16(&sigma, 1), "~sigma");
		R.replace(QString::fromUtf16(&tau, 1), "~tau");	R.replace(QString::fromUtf16(&upsilon, 1), "~upsilon");	R.replace(QString::fromUtf16(&phi, 1), "~phi");	R.replace(QString::fromUtf16(&chi, 1), "~chi");
		R.replace(QString::fromUtf16(&psi, 1), "~psi");	R.replace(QString::fromUtf16(&omega, 1), "~omega");	R.replace(QString::fromUtf16(&thetasym, 1), "~thetasym");	R.replace(QString::fromUtf16(&upsih, 1), "~upsih");
        R.replace(QString::fromUtf16(&piv, 1), "~piv");	R.replace(QString::fromUtf16(&sup2, 1), "~^2");	R.replace(QString::fromUtf16(&sup3, 1), "~^3");	R.replace(QString::fromUtf16(&frac14, 1), "~1/4");
		R.replace(QString::fromUtf16(&frac12, 1), "~1/2");	R.replace(QString::fromUtf16(&frac34, 1), "~3/4");	R.replace(QString::fromUtf16(&radic, 1), "~radic");	R.replace(QString::fromUtf16(&degree, 1), "~degree");	return R;
		/*	https://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references	*/
	}
    QString reformBack() const
	{
		char16_t alpha = 945;	char16_t beta = 946;	char16_t gamma = 947;	char16_t delta = 948;	char16_t epsilon = 949;	char16_t zeta = 950;	char16_t eta = 951;	char16_t theta = 952;	char16_t iota = 953;	char16_t kappa = 954;	char16_t lambda = 955;
		char16_t mu = 956;	char16_t nu = 957;	char16_t xi = 958;	char16_t omicron = 959;	char16_t pi = 960;	char16_t rho = 961;	char16_t sigmaf = 962;	char16_t sigma = 963;	char16_t tau = 964;	char16_t upsilon = 965;	char16_t phi = 966;
		char16_t chi = 967;	char16_t psi = 968;	char16_t omega = 969;	char16_t thetasym = 977;	char16_t upsih = 978;	char16_t piv = 982;	char16_t sup2 = 178;	char16_t sup3 = 179;	char16_t frac14 = 188;	char16_t frac12 = 189;	char16_t frac34 = 190;
		char16_t radic = 8730;	char16_t degree = 176;	QString R = *this;	R.replace(QString::fromUtf16(&alpha, 1), "~alpha");	R.replace(QString::fromUtf16(&beta, 1), "~beta");	R.replace(QString::fromUtf16(&gamma, 1), "~gamma");
		R.replace(QString::fromUtf16(&delta, 1), "~delta");	R.replace(QString::fromUtf16(&epsilon, 1), "~epsilon");	R.replace(QString::fromUtf16(&zeta, 1), "~zeta");	R.replace(QString::fromUtf16(&eta, 1), "~eta");
		R.replace(QString::fromUtf16(&theta, 1), "~theta");	R.replace(QString::fromUtf16(&iota, 1), "~iota");	R.replace(QString::fromUtf16(&kappa, 1), "~kappa");	R.replace(QString::fromUtf16(&lambda, 1), "~lambda");
		R.replace(QString::fromUtf16(&mu, 1), "~mu");	R.replace(QString::fromUtf16(&nu, 1), "~nu");	R.replace(QString::fromUtf16(&xi, 1), "~xi");	R.replace(QString::fromUtf16(&omicron, 1), "~omicron");
		R.replace(QString::fromUtf16(&pi, 1), "~pi");	R.replace(QString::fromUtf16(&rho, 1), "~rho");	R.replace(QString::fromUtf16(&sigmaf, 1), "~sigmaf");	R.replace(QString::fromUtf16(&sigma, 1), "~sigma");
		R.replace(QString::fromUtf16(&tau, 1), "~tau");	R.replace(QString::fromUtf16(&upsilon, 1), "~upsilon");	R.replace(QString::fromUtf16(&phi, 1), "~phi");	R.replace(QString::fromUtf16(&chi, 1), "~chi");
		R.replace(QString::fromUtf16(&psi, 1), "~psi");	R.replace(QString::fromUtf16(&omega, 1), "~omega");	R.replace(QString::fromUtf16(&thetasym, 1), "~thetasym");	R.replace(QString::fromUtf16(&upsih, 1), "~upsih");
        R.replace(QString::fromUtf16(&piv, 1), "~piv");	R.replace(QString::fromUtf16(&sup2, 1), "~^2");	R.replace(QString::fromUtf16(&sup3, 1), "~^3");	R.replace(QString::fromUtf16(&frac14, 1), "~1/4");
		R.replace(QString::fromUtf16(&frac12, 1), "~1/2");	R.replace(QString::fromUtf16(&frac34, 1), "~3/4");	R.replace(QString::fromUtf16(&radic, 1), "~radic");	R.replace(QString::fromUtf16(&degree, 1), "~degree");	return R;
		/*	https://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references	*/
	}

    QList<XString> split(QChar sep, Qt::SplitBehavior behavior = Qt::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const{
		QList<XString> R;
		QStringList QL = (*this).QString::split(sep, behavior, cs);
		for (int i = 0; i < QL.size(); i++)	{
			XString X(reform(QL[i]), unit, unitsList, defaultUnit);
			R.append(X);}
		return R;	};

    static	QStringList reform(const QStringList &X)
	{
		QStringList out; 
		for (int i = 0; i < X.size(); i++)
			out.append(XString::reform(X[i]));

		return out; 
	}
    static	QStringList reformBack(QStringList R)
	{
		QStringList out;
		for (int i = 0; i < R.size(); i++)
			out.append(XString::reformBack(R[i]));

		return out;
	}

private:
	


};
