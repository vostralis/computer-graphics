#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <vector>

class ImageProcessor
{
public:
    ImageProcessor();

public:
    static QImage applyTransformations(const QImage& src, int brightness, int contrast, bool isGray, bool isNegative);
    static std::vector<int> calculateHistogram(const QImage& src);
    static QImage binarizeManual(const QImage& src, int threshold);
    static QImage binarizeOtsu(const QImage& src);
    static QImage binarizeAdaptive(const QImage& src);

private:
    static void applyGrayscaleAndNegative(QImage& src, bool isGray, bool isNegative);
};

#endif // IMAGEPROCESSOR_H
