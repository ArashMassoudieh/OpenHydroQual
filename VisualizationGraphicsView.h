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

#ifndef VISUALIZATIONGRAPHICSVIEW_H
#define VISUALIZATIONGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>

/**
 * @brief Custom QGraphicsView with zoom and pan functionality for visualization
 *
 * This class extends QGraphicsView to provide:
 * - Mouse wheel zooming
 * - Click and drag panning
 * - Anti-aliased rendering
 * - Proper transformation anchoring
 */
class VisualizationGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param scene The graphics scene to display
     * @param parent Parent widget (optional)
     */
    explicit VisualizationGraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~VisualizationGraphicsView() = default;

protected:
    /**
     * @brief Handle mouse wheel events for zooming
     * @param event The wheel event
     */
    void wheelEvent(QWheelEvent* event) override;

private:
    static constexpr double ZOOM_FACTOR = 1.15; ///< Zoom scaling factor per wheel notch
};

#endif // VISUALIZATIONGRAPHICSVIEW_H
