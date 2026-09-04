#pragma once

#include <QColor>
#include <QSize>
#include <QWidget>

#include "viewer/ViewerScene.h"

class QPaintEvent;
class QPainter;

// ViewerWidget is the minimal viewer presentation surface.
//
// It presents a deterministic synthetic 360-degree test scene by unwrapping
// the whole sphere into an equirectangular view at the identity orientation.
// There is deliberately no camera state here yet: no yaw/pitch/roll/FOV input,
// no ViewportState connection, no media, and no camera-specific logic. This
// subtask only establishes the presentation surface and proves it can display
// the deterministic scene.
class ViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewerWidget(QWidget *parent = nullptr);

    const ViewerScene &scene() const { return m_scene; }
    void setScene(const ViewerScene &scene);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Deterministic mapping of a canonical scene angle to a widget pixel
    // position for the identity equirectangular presentation.
    static double pixelXForYaw(double yaw, double width);
    static double pixelYForPitch(double pitch, double height);

    void drawBackground(QPainter &painter);
    void drawOrientationGrid(QPainter &painter);
    void drawMarkers(QPainter &painter);
    QColor markerColor(int index) const;

    ViewerScene m_scene;
};
