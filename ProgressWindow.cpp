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
 */

#include "ProgressWindow.h"
#include <QGridLayout>
#include <QDateTime>
#include <QApplication>
#include <QAreaSeries>

ProgressWindow::ProgressWindow(QWidget* parent, const QString& title)
    : QDialog(parent)
    , primaryAutoScale_(true)
    , secondaryAutoScale_(true)
    , primaryKeepMinYAtZero_(false)      
    , secondaryKeepMinYAtZero_(false)    
    , tertiaryKeepMinYAtZero_(false)     
    , pauseRequested_(false)
    , cancelRequested_(false)
    , isPaused_(false)
{
    setWindowTitle(title);
    setMinimumSize(800, 600);

    setupUI();
    createPrimaryChart();
    createSecondaryChart();
    createTertiaryChart();

    // Default: Show primary chart, hide secondary
    SetPrimaryChartVisible(true);
    SetSecondaryChartVisible(false);
    SetTertiaryChartVisible(false);
    SetSecondaryProgressVisible(false);

    
}

ProgressWindow::~ProgressWindow()
{
}

void ProgressWindow::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Status label
    statusLabel_ = new QLabel("Ready", this);
    statusLabel_->setStyleSheet("QLabel { font-weight: bold; font-size: 12pt; }");
    mainLayout->addWidget(statusLabel_);

    // Primary progress bar
    primaryProgressLabel_ = new QLabel("Progress:", this);
    mainLayout->addWidget(primaryProgressLabel_);

    primaryProgressBar_ = new QProgressBar(this);
    primaryProgressBar_->setRange(0, 1000);
    primaryProgressBar_->setValue(0);
    primaryProgressBar_->setTextVisible(true);
    primaryProgressBar_->setFormat("%p%");
    mainLayout->addWidget(primaryProgressBar_);

    // PRIMARY CHART will be inserted here at index 3

    // Chart placeholders (will be created later)
    primaryChartView_ = nullptr;
    secondaryChartView_ = nullptr;

    // Information Panel (initially hidden)
    infoPanelLabel_ = new QLabel("Information:", this);
    mainLayout->addWidget(infoPanelLabel_);

    infoTextEdit_ = new QTextEdit(this);
    infoTextEdit_->setReadOnly(true);
    infoTextEdit_->setMaximumHeight(200);
    infoTextEdit_->setStyleSheet(
        "QTextEdit { "
        "background-color: #f5f5f5; "
        "border: 1px solid #cccccc; "
        "border-radius: 3px; "
        "padding: 5px; "
        "font-family: 'Courier New', monospace; "
        "}"
    );

    // Create show details button
    showDetailsButton_ = new QPushButton("Show Details", this);
    connect(showDetailsButton_, &QPushButton::clicked, this, &ProgressWindow::onShowDetailsClicked);
   
    // Add to main layout
    mainLayout->addWidget(infoTextEdit_);

    // Initially hide info panel
    infoPanelLabel_->setVisible(false);
    infoTextEdit_->setVisible(false);

    // Secondary progress bar - ADD AFTER INFO PANEL
    // This way it naturally falls after primary chart when that's inserted
    secondaryProgressLabel_ = new QLabel("Secondary Progress:", this);
    secondaryProgressLabel_->setVisible(false);  // Initially hidden
    mainLayout->addWidget(secondaryProgressLabel_);

    secondaryProgressBar_ = new QProgressBar(this);
    secondaryProgressBar_->setRange(0, 1000);
    secondaryProgressBar_->setValue(0);
    secondaryProgressBar_->setTextVisible(true);
    secondaryProgressBar_->setFormat("%p%");
    secondaryProgressBar_->setVisible(false);  // Initially hidden
    mainLayout->addWidget(secondaryProgressBar_);

    // SECONDARY CHART will be inserted after secondary progress bar

    // Log area
    QLabel* logLabel = new QLabel("Log:", this);
    mainLayout->addWidget(logLabel);

    logTextEdit_ = new QTextEdit(this);
    logTextEdit_->setReadOnly(true);
    logTextEdit_->setMaximumHeight(150);
    mainLayout->addWidget(logTextEdit_);

    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    pauseResumeButton_ = new QPushButton("Pause", this);
    cancelButton_ = new QPushButton("Cancel", this);

    buttonLayout->addWidget(pauseResumeButton_);
    buttonLayout->addWidget(cancelButton_);
    
    // Create detailed panel (hidden by default) - use QTextBrowser as specified
    detailedPanel_ = new QTextEdit(this);
    detailedPanel_->setVisible(false);
    detailedPanel_->setMaximumHeight(200); // Optional: set a reasonable height

    // Add to the main layout
    buttonLayout->addWidget(showDetailsButton_);  // Assuming your main layout is called mainLayout
    
    mainLayout->addLayout(buttonLayout);

    
    mainLayout->addWidget(detailedPanel_);

    // Connect signals
    connect(pauseResumeButton_, &QPushButton::clicked, this, &ProgressWindow::onPauseResumeClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &ProgressWindow::onCancelClicked);
}

void ProgressWindow::createPrimaryChart()
{
    // Create chart
    primaryChart_ = new QChart();
    primaryChart_->setTitle("Primary Chart");
    primaryChart_->setAnimationOptions(QChart::NoAnimation);
    primaryChart_->legend()->hide();

    // Create line series
    primarySeries_ = new QLineSeries();
    primarySeries_->setName("Primary Data");

    // Set line pen
    QPen pen(Qt::blue);
    pen.setWidth(2);

    // Create area series for filling
    primaryAreaSeries_ = new QAreaSeries(primarySeries_);
    primaryAreaSeries_->setName("Primary Data");

    // Set fill color with transparency
    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(0, 0, 255, 180));    // Blue at top
    gradient.setColorAt(1.0, QColor(0, 0, 255, 40));     // Light blue at bottom
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    primaryAreaSeries_->setBrush(gradient);
    primaryAreaSeries_->setPen(pen);

    // Add area series to chart
    primaryChart_->addSeries(primaryAreaSeries_);

    // Create axes
    primaryAxisX_ = new QValueAxis();
    primaryAxisX_->setTitleText("X");
    primaryAxisX_->setLabelFormat("%g");
    primaryAxisX_->setRange(0, 100);
    primaryChart_->addAxis(primaryAxisX_, Qt::AlignBottom);
    primaryAreaSeries_->attachAxis(primaryAxisX_);

    primaryAxisY_ = new QValueAxis();
    primaryAxisY_->setTitleText("Y");
    primaryAxisY_->setLabelFormat("%.2e");
    primaryAxisY_->setRange(0, 1);
    primaryChart_->addAxis(primaryAxisY_, Qt::AlignLeft);
    primaryAreaSeries_->attachAxis(primaryAxisY_);

    // Create chart view
    primaryChartView_ = new QChartView(primaryChart_, this);
    primaryChartView_->setRenderHint(QPainter::Antialiasing);
    primaryChartView_->setMinimumHeight(250);

    // Add to layout (after primary progress bar, index 3)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(3, primaryChartView_);
    }
}

void ProgressWindow::createSecondaryChart()
{
    // Create chart
    secondaryChart_ = new QChart();
    secondaryChart_->setTitle("Secondary Chart");
    secondaryChart_->setAnimationOptions(QChart::NoAnimation);
    secondaryChart_->legend()->hide();

    // Create line series
    secondarySeries_ = new QLineSeries();
    secondarySeries_->setName("Secondary Data");

    // Set line pen
    QPen pen(Qt::red);
    pen.setWidth(2);

    // Create area series for filling
    secondaryAreaSeries_ = new QAreaSeries(secondarySeries_);
    secondaryAreaSeries_->setName("Secondary Data");

    // Set fill color with transparency
    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(255, 0, 0, 180));    // Red at top
    gradient.setColorAt(1.0, QColor(255, 0, 0, 40));     // Light red at bottom
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    secondaryAreaSeries_->setBrush(gradient);
    secondaryAreaSeries_->setPen(pen);

    // Add area series to chart
    secondaryChart_->addSeries(secondaryAreaSeries_);

    // Create axes
    secondaryAxisX_ = new QValueAxis();
    secondaryAxisX_->setTitleText("X");
    secondaryAxisX_->setLabelFormat("%g");
    secondaryAxisX_->setRange(0, 1000);
    secondaryChart_->addAxis(secondaryAxisX_, Qt::AlignBottom);
    secondaryAreaSeries_->attachAxis(secondaryAxisX_);

    secondaryAxisY_ = new QValueAxis();
    secondaryAxisY_->setTitleText("Y");
    secondaryAxisY_->setLabelFormat("%.2f");
    secondaryAxisY_->setRange(-1000, 0);
    secondaryChart_->addAxis(secondaryAxisY_, Qt::AlignLeft);
    secondaryAreaSeries_->attachAxis(secondaryAxisY_);

    // Create chart view
    secondaryChartView_ = new QChartView(secondaryChart_, this);
    secondaryChartView_->setRenderHint(QPainter::Antialiasing);
    secondaryChartView_->setMinimumHeight(250);

    // Add to layout (after secondary progress bar)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        // Find where secondary progress bar is and insert chart right after it
        int secondaryBarIndex = mainLayout->indexOf(secondaryProgressBar_);
        if (secondaryBarIndex >= 0) {
            mainLayout->insertWidget(secondaryBarIndex + 1, secondaryChartView_);
        }
    }
}

// ============================================================================
// Progress Bar Methods
// ============================================================================

void ProgressWindow::SetProgress(double progress)
{
    int value = static_cast<int>(progress * 1000);
    primaryProgressBar_->setValue(value);
    QApplication::processEvents();
}

void ProgressWindow::SetSecondaryProgress(double progress)
{
    int value = static_cast<int>(progress * 1000);
    secondaryProgressBar_->setValue(value);
    QApplication::processEvents();
}

void ProgressWindow::SetSecondaryProgressVisible(bool visible)
{
    secondaryProgressLabel_->setVisible(visible);
    secondaryProgressBar_->setVisible(visible);
}

void ProgressWindow::SetProgressLabel(const QString& label)
{
    primaryProgressLabel_->setText(label);
}

void ProgressWindow::SetSecondaryProgressLabel(const QString& label)
{
    secondaryProgressLabel_->setText(label);
}

// ============================================================================
// Status and Logging Methods
// ============================================================================

void ProgressWindow::SetStatus(const QString& status)
{
    statusLabel_->setText(status);
    QApplication::processEvents();
}

void ProgressWindow::AppendLog(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logTextEdit_->append(QString("[%1] %2").arg(timestamp).arg(message));

    // Auto-scroll to bottom
    QTextCursor cursor = logTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    logTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::AppendLog(const std::string& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logTextEdit_->append(QString("[%1] %2").arg(timestamp).arg(QString::fromStdString(message)));

    // Auto-scroll to bottom
    QTextCursor cursor = logTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    logTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::ClearLog()
{
    logTextEdit_->clear();
}

// ============================================================================
// Information Panel Methods
// ============================================================================

void ProgressWindow::AppendInfo(const QString& text)
{
    infoTextEdit_->append(text);

    // Auto-scroll to bottom
    QTextCursor cursor = infoTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    infoTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::SetInfoText(const QString& text)
{
    infoTextEdit_->setPlainText(text);
    QApplication::processEvents();
}

void ProgressWindow::ClearInfo()
{
    infoTextEdit_->clear();
}

void ProgressWindow::SetInfoPanelVisible(bool visible)
{
    infoPanelLabel_->setVisible(visible);
    infoTextEdit_->setVisible(visible);
}

void ProgressWindow::SetInfoPanelLabel(const QString& label)
{
    infoPanelLabel_->setText(label);
}

// ============================================================================
// Primary Chart Methods
// ============================================================================

void ProgressWindow::AddPrimaryChartPoint(double x, double y)
{
    primaryData_.append(QPointF(x, y));
}

void ProgressWindow::ClearPrimaryChartData()
{
    primaryData_.clear();
    primarySeries_->clear();
}

void ProgressWindow::SetPrimaryChartYRange(double min, double max)
{
    primaryAutoScale_ = false;
    primaryAxisY_->setRange(min, max);
}

void ProgressWindow::SetPrimaryChartXRange(double min, double max)
{
    if (primaryAxisX_) {
        primaryAxisX_->setRange(min, max);
    }
}

void ProgressWindow::SetPrimaryChartAutoScale(bool enabled)
{
    primaryAutoScale_ = enabled;
}

void ProgressWindow::SetPrimaryChartVisible(bool visible)
{
    if (primaryChartView_) {
        primaryChartView_->setVisible(visible);
    }
}

void ProgressWindow::SetPrimaryChartTitle(const QString& title)
{
    if (primaryChart_) {
        primaryChart_->setTitle(title);
    }
}

void ProgressWindow::SetPrimaryChartXAxisTitle(const QString& title)
{
    if (primaryAxisX_) {
        primaryAxisX_->setTitleText(title);
    }
}

void ProgressWindow::SetPrimaryChartYAxisTitle(const QString& title)
{
    if (primaryAxisY_) {
        primaryAxisY_->setTitleText(title);
    }
}

void ProgressWindow::SetPrimaryChartColor(const QColor& color)
{
    if (primaryAreaSeries_) {
        // Update line pen
        QPen pen = primaryAreaSeries_->pen();
        pen.setColor(color);
        primaryAreaSeries_->setPen(pen);

        // Update gradient
        QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
        gradient.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 180));
        gradient.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), 40));
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        primaryAreaSeries_->setBrush(gradient);
    }
}

// Modify updatePrimaryChart() to use the new setting:

void ProgressWindow::updatePrimaryChart()
{
    if (primaryData_.isEmpty()) {
        return;
    }

    // Update series
    primarySeries_->replace(primaryData_);

    // Auto-scale if enabled
    if (primaryAutoScale_ && !primaryData_.isEmpty()) {
        // Find min/max
        double minVal = primaryData_[0].y();
        double maxVal = primaryData_[0].y();

        for (const QPointF& point : primaryData_) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
        }

        // Keep minimum at zero if option is enabled
        if (primaryKeepMinYAtZero_) {
            minVal = 0.0;
        }

        primaryAxisY_->setRange(minVal, maxVal);
    }

    QApplication::processEvents();
}
// ============================================================================
// Secondary Chart Methods
// ============================================================================

void ProgressWindow::AddSecondaryChartPoint(double x, double y)
{
    secondaryData_.append(QPointF(x, y));
}

void ProgressWindow::ClearSecondaryChartData()
{
    secondaryData_.clear();
    secondarySeries_->clear();
}

void ProgressWindow::SetSecondaryChartYRange(double min, double max)
{
    secondaryAutoScale_ = false;
    secondaryAxisY_->setRange(min, max);
}

void ProgressWindow::SetSecondaryChartXRange(double min, double max)
{
    if (secondaryAxisX_) {
        secondaryAxisX_->setRange(min, max);
    }
}

void ProgressWindow::SetSecondaryChartAutoScale(bool enabled)
{
    secondaryAutoScale_ = enabled;
}

void ProgressWindow::SetSecondaryChartVisible(bool visible)
{
    if (secondaryChartView_) {
        secondaryChartView_->setVisible(visible);
    }
}

void ProgressWindow::SetSecondaryChartTitle(const QString& title)
{
    if (secondaryChart_) {
        secondaryChart_->setTitle(title);
    }
}

void ProgressWindow::SetSecondaryChartXAxisTitle(const QString& title)
{
    if (secondaryAxisX_) {
        secondaryAxisX_->setTitleText(title);
    }
}

void ProgressWindow::SetSecondaryChartYAxisTitle(const QString& title)
{
    if (secondaryAxisY_) {
        secondaryAxisY_->setTitleText(title);
    }
}

void ProgressWindow::SetSecondaryChartColor(const QColor& color)
{
    if (secondaryAreaSeries_) {
        // Update line pen
        QPen pen = secondaryAreaSeries_->pen();
        pen.setColor(color);
        secondaryAreaSeries_->setPen(pen);

        // Update gradient
        QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
        gradient.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 180));
        gradient.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), 40));
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        secondaryAreaSeries_->setBrush(gradient);
    }
}

void ProgressWindow::updateSecondaryChart()
{
    if (secondaryData_.isEmpty()) {
        return;
    }

    // Update series
    secondarySeries_->replace(secondaryData_);

    // Auto-scale if enabled
    if (secondaryAutoScale_ && !secondaryData_.isEmpty()) {
        // Find min/max
        double minVal = secondaryData_[0].y();
        double maxVal = secondaryData_[0].y();

        for (const QPointF& point : secondaryData_) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
        }

        // Keep minimum at zero if option is enabled
        if (secondaryKeepMinYAtZero_) {
            minVal = 0.0;
        }

        secondaryAxisY_->setRange(minVal, maxVal);
    }

    QApplication::processEvents();
}

// ============================================================================
// Control Methods
// ============================================================================

void ProgressWindow::SetPauseEnabled(bool enabled)
{
    pauseResumeButton_->setEnabled(enabled);
}

void ProgressWindow::SetComplete(const QString& message)
{
    SetStatus(message);
    pauseResumeButton_->setEnabled(false);
    cancelButton_->setText("Close");
}

void ProgressWindow::onPauseResumeClicked()
{
    if (isPaused_) {
        // Resume
        isPaused_ = false;
        pauseRequested_ = false;
        pauseResumeButton_->setText("Pause");
        AppendLog(std::string("Resumed"));
        emit resumeClicked();
    }
    else {
        // Pause
        isPaused_ = true;
        pauseRequested_ = true;
        pauseResumeButton_->setText("Resume");
        AppendLog(std::string("Pause requested..."));
        emit pauseClicked();
    }
}

void ProgressWindow::onCancelClicked()
{
    if (cancelButton_->text() == "Close") {
        accept();
        return;
    }

    cancelRequested_ = true;
    AppendLog(std::string("Cancel requested..."));
    cancelButton_->setEnabled(false);
    emit cancelClicked();
}

// ========================================================================
// Tertiary Chart Methods
// ========================================================================

void ProgressWindow::createTertiaryChart()
{
    tertiaryChart_ = new QChart();
    tertiaryChart_->setTitle("Perturbation Coefficient");
    tertiaryChart_->legend()->hide();
    tertiaryChart_->setAnimationOptions(QChart::NoAnimation);

    tertiarySeries_ = new QLineSeries();
    tertiarySeries_->setName("Perturbation Coefficient");

    // Create area series
    QLineSeries* lowerBound = new QLineSeries();
    lowerBound->append(0, 0);
    tertiaryAreaSeries_ = new QAreaSeries(tertiarySeries_, lowerBound);
    tertiaryAreaSeries_->setName("Perturbation Coefficient");
    tertiaryAreaSeries_->setOpacity(0.3);
    tertiaryAreaSeries_->setColor(QColor(150, 100, 200));

    tertiaryChart_->addSeries(tertiaryAreaSeries_);
    tertiaryChart_->addSeries(tertiarySeries_);

    // X Axis
    tertiaryAxisX_ = new QValueAxis();
    tertiaryAxisX_->setTitleText("Sample");
    tertiaryAxisX_->setLabelFormat("%d");
    tertiaryChart_->addAxis(tertiaryAxisX_, Qt::AlignBottom);
    tertiarySeries_->attachAxis(tertiaryAxisX_);
    tertiaryAreaSeries_->attachAxis(tertiaryAxisX_);

    // Y Axis
    tertiaryAxisY_ = new QValueAxis();
    tertiaryAxisY_->setTitleText("Coefficient");
    tertiaryAxisY_->setLabelFormat("%.3f");
    tertiaryAxisY_->setMin(0.0);
    tertiaryAxisY_->setMax(1.0);
    tertiaryChart_->addAxis(tertiaryAxisY_, Qt::AlignLeft);
    tertiarySeries_->attachAxis(tertiaryAxisY_);
    tertiaryAreaSeries_->attachAxis(tertiaryAxisY_);

    tertiaryChartView_ = new QChartView(tertiaryChart_);
    tertiaryChartView_->setRenderHint(QPainter::Antialiasing);
    tertiaryChartView_->setMinimumHeight(200);

    tertiaryAutoScale_ = true;

    // Add to layout (after secondary chart if it exists)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        // Find where secondary chart view is and insert tertiary chart after it
        int secondaryChartIndex = mainLayout->indexOf(secondaryChartView_);
        if (secondaryChartIndex >= 0) {
            mainLayout->insertWidget(secondaryChartIndex + 1, tertiaryChartView_);
        }
    }
}

void ProgressWindow::AddTertiaryChartPoint(double x, double y)
{
    tertiaryData_.append(QPointF(x, y));

}

void ProgressWindow::updateTertiaryChart()
{
    if (!tertiarySeries_) return;

    tertiarySeries_->replace(tertiaryData_);

    // Update area series lower bound
    if (tertiaryAreaSeries_) {
        QLineSeries* lowerSeries = qobject_cast<QLineSeries*>(tertiaryAreaSeries_->lowerSeries());
        if (lowerSeries && tertiaryData_.size() > 0) {
            lowerSeries->clear();
            lowerSeries->append(tertiaryData_.first().x(), 0);
            lowerSeries->append(tertiaryData_.last().x(), 0);
        }
    }

    if (tertiaryAutoScale_ && tertiaryData_.size() > 0) {
        // Auto-scale Y axis
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();

        for (const QPointF& point : tertiaryData_) {
            minY = std::min(minY, point.y());
            maxY = std::max(maxY, point.y());
        }

        double margin = (maxY - minY) * 0.1;
        if (margin < 0.01) margin = 0.1;

        // Keep minimum at zero if option is enabled
        if (tertiaryKeepMinYAtZero_) {
            minY = 0.0;
            margin = 0.0;  // No lower margin needed
        }

        tertiaryAxisY_->setRange(std::max(0.0, minY - margin), maxY + margin);

        // Auto-scale X axis
        double minX = tertiaryData_.first().x();
        double maxX = tertiaryData_.last().x();
        tertiaryAxisX_->setRange(minX, maxX);
    }
}

void ProgressWindow::ClearTertiaryChartData()
{
    tertiaryData_.clear();
    if (tertiarySeries_) {
        tertiarySeries_->clear();
    }
    if (tertiaryAreaSeries_) {
        QLineSeries* lowerSeries = qobject_cast<QLineSeries*>(tertiaryAreaSeries_->lowerSeries());
        if (lowerSeries) {
            lowerSeries->clear();
        }
    }
}

void ProgressWindow::SetTertiaryChartYRange(double min, double max)
{
    if (tertiaryAxisY_) {
        tertiaryAxisY_->setRange(min, max);
    }
}

void ProgressWindow::SetTertiaryChartXRange(double min, double max)
{
    if (tertiaryAxisX_) {
        tertiaryAxisX_->setRange(min, max);
    }
}

void ProgressWindow::SetTertiaryChartAutoScale(bool enabled)
{
    tertiaryAutoScale_ = enabled;
}

void ProgressWindow::SetTertiaryChartTitle(const QString& title)
{
    if (tertiaryChart_) {
        tertiaryChart_->setTitle(title);
    }
}

void ProgressWindow::SetTertiaryChartXAxisTitle(const QString& title)
{
    if (tertiaryAxisX_) {
        tertiaryAxisX_->setTitleText(title);
    }
}

void ProgressWindow::SetTertiaryChartYAxisTitle(const QString& title)
{
    if (tertiaryAxisY_) {
        tertiaryAxisY_->setTitleText(title);
    }
}

void ProgressWindow::SetTertiaryChartColor(const QColor& color)
{
    if (tertiarySeries_) {
        QPen pen = tertiarySeries_->pen();
        pen.setColor(color);
        pen.setWidth(2);
        tertiarySeries_->setPen(pen);
    }
    if (tertiaryAreaSeries_) {
        tertiaryAreaSeries_->setColor(color);
    }
}

void ProgressWindow::SetTertiaryChartVisible(bool visible)
{
    if (tertiaryChartView_) {
        tertiaryChartView_->setVisible(visible);
    }
}

// Add these method implementations:

bool ProgressWindow::DetailsOn() const
{
    return detailsVisible_;
}

void ProgressWindow::AppendDetails(const QString& text)
{
    if (detailedPanel_) {
        detailedPanel_->append(text);
    }
}

void ProgressWindow::ClearDetails()
{
    if (detailedPanel_) {
        detailedPanel_->clear();
    }
}

void ProgressWindow::onShowDetailsClicked()
{
    detailsVisible_ = !detailsVisible_;
    detailedPanel_->setVisible(detailsVisible_);
    showDetailsButton_->setText(detailsVisible_ ? "Hide Details" : "Show Details");
}

void ProgressWindow::ReplotPrimaryChart()
{
    updatePrimaryChart();
}

void ProgressWindow::ReplotSecondaryChart()
{
    updateSecondaryChart();
}

void ProgressWindow::ReplotTertiaryChart()
{
    updateTertiaryChart();
}

void ProgressWindow::SetPrimaryChartKeepMinYAtZero(bool keepAtZero)
{
    primaryKeepMinYAtZero_ = keepAtZero;
}

void ProgressWindow::SetSecondaryChartKeepMinYAtZero(bool keepAtZero)
{
    secondaryKeepMinYAtZero_ = keepAtZero;
}

void ProgressWindow::SetTertiaryChartKeepMinYAtZero(bool keepAtZero)
{
    tertiaryKeepMinYAtZero_ = keepAtZero;
}