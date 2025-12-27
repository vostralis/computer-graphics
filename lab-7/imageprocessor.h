#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QImage>

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    static QImage applyNearestNeighbourScaling(const QImage& src, float scale);
    static QImage applyBilinearScaling(const QImage& src, float scale);
};

#endif // IMAGEPROCESSOR_H
