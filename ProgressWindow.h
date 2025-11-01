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

#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVector>
#include <QtCharts/QAreaSeries>

/**
 * @class ProgressWindow
 * @brief Generic progress window with configurable charts and progress bars
 *
 * Features:
 * - Up to two configurable progress bars
 * - Up to two configurable charts with customizable axes
 * - Real-time status updates
 * - Scrolling log area
 * - Optional information panel
 * - Pause/Resume/Cancel buttons
 *
 * @note All chart and progress bar methods use generic names (Primary/Secondary)
 *       to allow flexibility for different use cases (GA, MCMC, etc.)
 */
class ProgressWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     * @param title Window title
     */
    explicit ProgressWindow(QWidget* parent = nullptr, const QString& title = "Progress");

    /**
     * @brief Destructor
     */
    ~ProgressWindow();

    // ========================================================================
    // Progress Bar Methods
    // ========================================================================

    /**
     * @brief Set primary progress (0.0 to 1.0)
     * @param progress Progress value between 0 and 1
     */
    void SetProgress(double progress);

    /**
     * @brief Set secondary progress (0.0 to 1.0)
     * @param progress Progress value between 0 and 1
     */
    void SetSecondaryProgress(double progress);

    /**
     * @brief Show or hide the secondary progress bar
     * @param visible True to show, false to hide
     */
    void SetSecondaryProgressVisible(bool visible);

    /**
     * @brief Set label for primary progress bar
     * @param label Text label
     */
    void SetProgressLabel(const QString& label);

    /**
     * @brief Set label for secondary progress bar
     * @param label Text label
     */
    void SetSecondaryProgressLabel(const QString& label);

    // ========================================================================
    // Status and Logging Methods
    // ========================================================================

    /**
     * @brief Update status message
     * @param status Status text to display
     */
    void SetStatus(const QString& status);

    /**
     * @brief Append message to log
     * @param message Message to append
     */
    void AppendLog(const QString& message);
    void AppendLog(const std::string& message);


    /**
     * @brief Clear the log
     */
    void ClearLog();

    // ========================================================================
    // Information Panel Methods
    // ========================================================================

    /**
     * @brief Append text to information panel
     * @param text Text to append
     */
    void AppendInfo(const QString& text);

    /**
     * @brief Set text in information panel (replaces all content)
     * @param text Text to display
     */
    void SetInfoText(const QString& text);

    /**
     * @brief Clear information panel
     */
    void ClearInfo();

    /**
     * @brief Show or hide information panel
     * @param visible True to show, false to hide
     */
    void SetInfoPanelVisible(bool visible);

    /**
     * @brief Set information panel label
     * @param label Label text
     */
    void SetInfoPanelLabel(const QString& label);

    // ========================================================================
    // Primary Chart Methods
    // ========================================================================

    /**
     * @brief Add data point to primary chart
     * @param x X-axis value
     * @param y Y-axis value
     */
    void AddPrimaryChartPoint(double x, double y);

    /**
     * @brief Clear all primary chart data
     */
    void ClearPrimaryChartData();

    /**
     * @brief Set Y-axis range for primary chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void SetPrimaryChartYRange(double min, double max);

    /**
     * @brief Set X-axis range for primary chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void SetPrimaryChartXRange(double min, double max);

    /**
     * @brief Enable auto-scaling for primary chart Y-axis
     * @param enabled True to enable auto-scaling
     */
    void SetPrimaryChartAutoScale(bool enabled);

    /**
     * @brief Show or hide primary chart
     * @param visible True to show, false to hide
     */
    void SetPrimaryChartVisible(bool visible);

    /**
     * @brief Set primary chart title
     * @param title Chart title
     */
    void SetPrimaryChartTitle(const QString& title);

    /**
     * @brief Set X-axis title for primary chart
     * @param title X-axis title
     */
    void SetPrimaryChartXAxisTitle(const QString& title);

    /**
     * @brief Set Y-axis title for primary chart
     * @param title Y-axis title
     */
    void SetPrimaryChartYAxisTitle(const QString& title);

    /**
     * @brief Set line color for primary chart
     * @param color Line color
     */
    void SetPrimaryChartColor(const QColor& color);

    // ========================================================================
    // Secondary Chart Methods
    // ========================================================================

    /**
     * @brief Add data point to secondary chart
     * @param x X-axis value
     * @param y Y-axis value
     */
    void AddSecondaryChartPoint(double x, double y);

    /**
     * @brief Clear all secondary chart data
     */
    void ClearSecondaryChartData();

    /**
     * @brief Set Y-axis range for secondary chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void SetSecondaryChartYRange(double min, double max);

    /**
     * @brief Set X-axis range for secondary chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void SetSecondaryChartXRange(double min, double max);

    /**
     * @brief Enable auto-scaling for secondary chart Y-axis
     * @param enabled True to enable auto-scaling
     */
    void SetSecondaryChartAutoScale(bool enabled);

    /**
     * @brief Show or hide secondary chart
     * @param visible True to show, false to hide
     */
    void SetSecondaryChartVisible(bool visible);

    /**
     * @brief Set tertiary chart title
     * @param title Chart title
     */

    void SetTertiaryChartVisible(bool visible);

    /**
     * @brief Set secondary chart title
     * @param title Chart title
     */

    void SetSecondaryChartTitle(const QString& title);

    /**
     * @brief Set X-axis title for secondary chart
     * @param title X-axis title
     */
    void SetSecondaryChartXAxisTitle(const QString& title);

    /**
     * @brief Set Y-axis title for secondary chart
     * @param title Y-axis title
     */
    void SetSecondaryChartYAxisTitle(const QString& title);

    /**
     * @brief Set line color for secondary chart
     * @param color Line color
     */
    void SetSecondaryChartColor(const QColor& color);

    // Add these to the public section after secondary chart methods:

    // Add to Primary Chart Methods section:

    /**
     * @brief Set whether to keep minimum Y value at zero
     * @param keepAtZero True to keep min Y at zero, false to auto-scale min
     */
    void SetPrimaryChartKeepMinYAtZero(bool keepAtZero);

    // Add to Secondary Chart Methods section:

    /**
     * @brief Set whether to keep minimum Y value at zero
     * @param keepAtZero True to keep min Y at zero, false to auto-scale min
     */
    void SetSecondaryChartKeepMinYAtZero(bool keepAtZero);

    // Add to Tertiary Chart Methods section:

    /**
     * @brief Set whether to keep minimum Y value at zero
     * @param keepAtZero True to keep min Y at zero, false to auto-scale min
     */
    void SetTertiaryChartKeepMinYAtZero(bool keepAtZero);
    // ========================================================================
    // Tertiary Chart Methods
    // ========================================================================

    void AddTertiaryChartPoint(double x, double y);
    void ClearTertiaryChartData();
    void SetTertiaryChartYRange(double min, double max);
    void SetTertiaryChartXRange(double min, double max);
    void SetTertiaryChartAutoScale(bool enabled);
    void SetTertiaryChartTitle(const QString& title);
    void SetTertiaryChartXAxisTitle(const QString& title);
    void SetTertiaryChartYAxisTitle(const QString& title);
    void SetTertiaryChartColor(const QColor& color);


    // ========================================================================
    // Control Methods
    // ========================================================================

    /**
     * @brief Check if user requested pause
     * @return True if pause requested
     */
    bool IsPauseRequested() const { return pauseRequested_; }

    /**
     * @brief Check if user requested cancel
     * @return True if cancel requested
     */
    bool IsCancelRequested() const { return cancelRequested_; }

    /**
     * @brief Reset pause request (after handling)
     */
    void ResetPauseRequest() { pauseRequested_ = false; }

    /**
     * @brief Enable or disable pause button
     * @param enabled True to enable
     */
    void SetPauseEnabled(bool enabled);

    /**
     * @brief Show completion message
     * @param message Completion message
     */
    void SetComplete(const QString& message = "Complete!");

    bool DetailsOn() const;

    /**
     * @brief Append text to detailed panel
     * @param text Text to append
     */
    void AppendDetails(const QString& text);

    /**
     * @brief Clear detailed panel
     */
    void ClearDetails();

    // Add to Primary Chart Methods section:

    /**
     * @brief Manually update the primary chart display
     * @note Call this after adding multiple points to update the chart
     */
    void ReplotPrimaryChart();

    // Add to Secondary Chart Methods section:

    /**
     * @brief Manually update the secondary chart display
     * @note Call this after adding multiple points to update the chart
     */
    void ReplotSecondaryChart();

    // Add to Tertiary Chart Methods section:

    /**
     * @brief Manually update the tertiary chart display
     * @note Call this after adding multiple points to update the chart
     */
    void ReplotTertiaryChart();

    /**
    * @brief Check if user has cancelled
    * @return True if cancel was clicked
    */

    bool Cancelled() const { return cancelRequested_; }

signals:
    /**
     * @brief Emitted when user clicks pause
     */
    void pauseClicked();

    /**
     * @brief Emitted when user clicks resume
     */
    void resumeClicked();

    /**
     * @brief Emitted when user clicks cancel
     */
    void cancelClicked();

    /**
     * @brief Check if detailed panel is visible
     * @return True if panel is visible, false if hidden
     */
    


private slots:
    void onPauseResumeClicked();
    void onCancelClicked();
    void onShowDetailsClicked();

private:
    void setupUI();
    void createPrimaryChart();
    void createSecondaryChart();
    void createTertiaryChart();
    void updatePrimaryChart();
    void updateSecondaryChart();
    void updateTertiaryChart();

    // UI Components
    QLabel* statusLabel_;
    QLabel* primaryProgressLabel_;
    QLabel* secondaryProgressLabel_;
    QProgressBar* primaryProgressBar_;
    QProgressBar* secondaryProgressBar_;
    QTextEdit* logTextEdit_;

    // Detailed Panel
    QTextEdit* detailedPanel_ = nullptr;
    QPushButton* showDetailsButton_;
    bool detailsVisible_ = false;

    // Information Panel
    QLabel* infoPanelLabel_;
    QTextEdit* infoTextEdit_;

    QPushButton* pauseResumeButton_;
    QPushButton* cancelButton_;

    // Primary Chart
    QChart* primaryChart_;
    QChartView* primaryChartView_;
    QLineSeries* primarySeries_;
    QAreaSeries* primaryAreaSeries_;
    QValueAxis* primaryAxisX_;
    QValueAxis* primaryAxisY_;
    bool primaryAutoScale_;
    QVector<QPointF> primaryData_;

    // Secondary Chart
    QChart* secondaryChart_;
    QChartView* secondaryChartView_;
    QLineSeries* secondarySeries_;
    QAreaSeries* secondaryAreaSeries_;
    QValueAxis* secondaryAxisX_;
    QValueAxis* secondaryAxisY_;
    bool secondaryAutoScale_;
    QVector<QPointF> secondaryData_;


    QChart* tertiaryChart_;
    QChartView* tertiaryChartView_;
    QLineSeries* tertiarySeries_;
    QAreaSeries* tertiaryAreaSeries_;
    QValueAxis* tertiaryAxisX_;
    QValueAxis* tertiaryAxisY_;
    bool tertiaryAutoScale_;
    QVector<QPointF> tertiaryData_;

    // State
    bool pauseRequested_;
    bool cancelRequested_;
    bool isPaused_;

    bool primaryKeepMinYAtZero_;
    bool secondaryKeepMinYAtZero_;
    bool tertiaryKeepMinYAtZero_;
};

#endif // PROGRESSWINDOW_H
