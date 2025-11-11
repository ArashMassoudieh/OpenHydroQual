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

#ifndef VISUALIZATIONDIALOG_H
#define VISUALIZATIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QColor>
#include <vector>
#include <utility>
#include <string>
#include "System.h"
#include "Expression.h"

using namespace std;

/**
 * @brief Dialog for visualizing system blocks and links with color-coded property values
 *
 * This dialog provides an interactive visualization of the water quality model,
 * displaying blocks as rectangles and links as connecting lines. Blocks are
 * color-coded based on the selected property value using a gradient from blue
 * (minimum) to red (maximum).
 */
class VisualizationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param system Pointer to the System object containing blocks and links
     * @param parent Parent widget (optional)
     */
    explicit VisualizationDialog(System* system, QWidget *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~VisualizationDialog();

private slots:
    /**
     * @brief Slot called when the property selection changes
     * @param index The index of the selected property in the combo box
     */
    void onPropertyChanged(int index);

    /**
     * @brief Slot called when the time slider value changes
     * @param value The slider position (0-1000)
     */
    void onTimeChanged(int value);

    /**
     * @brief Slot called when the reset view button is clicked
     */
    void onResetView();

    /**
     * @brief Slot called when the export button is clicked
     */
    void onExport();

private:
    /**
     * @brief Initialize the user interface
     */
    void setupUI();

    /**
     * @brief Populate the combo box with available properties
     */
    void populatePropertyComboBox();

    /**
     * @brief Draw all blocks and links based on current property selection
     */
    void drawVisualization();

    /**
     * @brief Get value for an object at the current selected time from Outputs
     * @param objectName Name of the block or link
     * @param propertyName Name of the property
     * @param defaultValue Value to return if not found in outputs
     * @return Interpolated value at current time, or defaultValue if not found
     */
    double getValueAtTime(const string& objectName, const string& propertyName, double defaultValue = 0.0);

    /**
     * @brief Check if simulation outputs are available
     * @return True if Outputs.AllOutputs has data
     */
    bool hasSimulationData() const;

    /**
     * @brief Initialize the time slider with proper range from simulation data
     */
    void setupTimeSlider();

    /**
     * @brief Calculate color based on value using gradient from blue to red
     * @param value The property value
     * @param minValue Minimum value in the dataset
     * @param maxValue Maximum value in the dataset
     * @return QColor representing the value
     */
    QColor getColorForValue(double value, double minValue, double maxValue);

    /**
     * @brief Get the minimum and maximum values for the current property at current time
     * @param propertyName Name of the property to analyze
     * @param minValue Output parameter for minimum value
     * @param maxValue Output parameter for maximum value
     */
    void getPropertyRange(const string& propertyName, double& minValue, double& maxValue);

    /**
     * @brief Calculate bounds of the visualization to properly set scene rect
     * @param minX Output parameter for minimum X coordinate
     * @param minY Output parameter for minimum Y coordinate
     * @param maxX Output parameter for maximum X coordinate
     * @param maxY Output parameter for maximum Y coordinate
     */
    void calculateBounds(double& minX, double& minY, double& maxX, double& maxY);

    /**
     * @brief Create and display the color legend
     */
    void createLegend(double minValue, double maxValue);

    // UI Components
    QComboBox* propertyComboBox;
    QGraphicsView* graphicsView;
    QWidget* legendWidget;
    QLabel* legendTitleLabel;
    QLabel* legendMaxLabel;
    QLabel* legendMidLabel;
    QLabel* legendMinLabel;
    QGraphicsScene* scene;
    QPushButton* resetViewButton;
    QPushButton* exportButton;
    QLabel* legendLabel;
    QGraphicsRectItem* legendGradient;
    QLabel* minValueLabel;
    QLabel* maxValueLabel;

    // Time control components
    QSlider* timeSlider;
    QLabel* timeLabel;
    QLabel* minTimeLabel;
    QLabel* maxTimeLabel;

    // Data
    System* system;
    vector<pair<string, string>> properties; // pair<property_name, description>
    string currentProperty;
    double currentTime; // Current selected time
    double minTime;     // Minimum time in simulation
    double maxTime;     // Maximum time in simulation
    double minValue;     // Minimum time in simulation
    double maxValue;     // Maximum time in simulation


    // Visualization parameters
    static constexpr double DEFAULT_BLOCK_WIDTH = 60.0;
    static constexpr double DEFAULT_BLOCK_HEIGHT = 40.0;
    static constexpr double LINK_WIDTH = 2.0;
    static constexpr int ARROW_SIZE = 10;
    static constexpr int TIME_SLIDER_RESOLUTION = 1000; // Slider steps
};

#endif // VISUALIZATIONDIALOG_H
