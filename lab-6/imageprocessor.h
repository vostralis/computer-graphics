#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <QObject>

#include <random>

using Kernel = std::vector<std::vector<double>>;

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    ImageProcessor();

    static QImage applySpotNoise(const QImage& src, int dotCount);
    static QImage applyLineNoise(const QImage& src, int lineCount);
    static QImage applyCircleNoise(const QImage& src, int circleCount);
    static QImage applyMedianFilter(const QImage& src, int kernelSize);
    static QImage applyGaussianFilter(const QImage& src, int kernelSize);

    static QImage applyBasicSharpeningFilter(const QImage& src, int k);
    static QImage applySobelFilter(const QImage& src, int k);
    static QImage applyPrewittFilter(const QImage& src, int k);
    static QImage applyRobertsFilter(const QImage& src, int k);
    static QImage applyLaplacian(const QImage& src, int k);

    static QImage applyGlassEffect(const QImage& src, int radius);

private:
    static QImage addWeighted(const QImage& src1, double alpha, const QImage& src2, double beta, double gamma);
    static QImage applyKernel(const QImage& src, const Kernel& aperture);
    static QImage applyEdgeDetection(const QImage& src, const Kernel& kx, const Kernel& ky);

private:
    static std::random_device rd;
    static std::mt19937 rng;
};

#endif // IMAGEPROCESSOR_H
