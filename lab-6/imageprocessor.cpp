#include "imageprocessor.h"

#include <QPainter>

std::random_device ImageProcessor::rd = {};
std::mt19937 ImageProcessor::rng(rd());

ImageProcessor::ImageProcessor() = default;

QImage ImageProcessor::applySpotNoise(const QImage& src, int dotCount) {
    if (src.isNull()) return QImage();

    QImage result = src;

    std::uniform_int_distribution<> distX(0, src.width() - 1);
    std::uniform_int_distribution<> distY(0, src.height() - 1);

    for (size_t i = 0; i < dotCount; ++i) {
        auto [x, y] = std::pair{ distX(rng), distY(rng) };
        QRgb *row = reinterpret_cast<QRgb*>(result.scanLine(y));
        row[x] = qRgb(255, 255, 255);
    }

    return result;
}

QImage ImageProcessor::applyLineNoise(const QImage& src, int lineCount) {
    if (src.isNull()) return QImage();

    QImage result = src;

    std::uniform_int_distribution<> distX(0, src.width() - 1);
    std::uniform_int_distribution<> distY(0, src.height() - 1);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::white, 2));

    for (size_t i = 0; i < lineCount; ++i) {
        QPoint p1 = QPoint(distX(rng), distY(rng));
        QPoint p2 = QPoint(distX(rng), distY(rng));
        painter.drawLine(p1, p2);
    }

    painter.end();

    return result;
}

QImage ImageProcessor::applyCircleNoise(const QImage& src, int circleCount) {
    if (src.isNull()) return QImage();

    QImage result = src;

    std::uniform_int_distribution<> distX(0, src.width() - 1);
    std::uniform_int_distribution<> distY(0, src.height() - 1);
    std::uniform_int_distribution<> distR(3, 10);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::white, 2));

    for (size_t i = 0; i < circleCount; ++i) {
        QPoint center = QPoint(distX(rng), distY(rng));
        int radius = distR(rng);
        painter.drawEllipse(center, radius, radius);
    }

    painter.end();

    return result;
}

QImage ImageProcessor::applyMedianFilter(const QImage& src, int kernelSize) {
    if (src.isNull()) return QImage();

    if (kernelSize % 2 == 0) kernelSize++;

    QImage result = src;
    int width = src.width();
    int height = src.height();
    int radius = kernelSize / 2;
    auto cmp = [](const QRgb& lhs, const QRgb& rhs) {
        return qGray(lhs) < qGray(rhs);
    };

    std::vector<QRgb> window;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            window.clear();

            for (int wy = -radius; wy <= radius; ++wy) {
                for (int wx = -radius; wx <= radius; ++wx) {
                    // Handle edges
                    int px = std::clamp(x + wx, 0, width - 1);
                    int py = std::clamp(y + wy, 0, height - 1);

                    QRgb pixel = src.pixel(px, py);
                    window.emplace_back(pixel);
                }
            }

            std::sort(window.begin(), window.end(), cmp);
            result.setPixel(x, y, window[window.size() / 2]);
        }
    }

    return result;
}

QImage ImageProcessor::applyGaussianFilter(const QImage& src, int kernelSize) {
    if (src.isNull()) return QImage();

    if (kernelSize % 2 == 0) kernelSize++;

    int width = src.width();
    int height = src.height();
    int radius = kernelSize / 2;

    QImage result = src.copy();

    Kernel kernel(kernelSize, std::vector<double>(kernelSize));
    double sum = 0.0;

    double sigma = (kernelSize - 1) / 6.0;
    double twoSigmaSq = 2.0 * sigma * sigma;

    // Calculate h_{l,k}
    for (int l = -radius; l <= radius; ++l) {
        for (int k = -radius; k <= radius; ++k) {
            double exponent = static_cast<double>(-(l*l + k*k)) / twoSigmaSq;
            double value = std::exp(exponent);

            kernel[l + radius][k + radius] = value;
            sum += value;
        }
    }

    // Normalize (calculate A * h_{l, k})
    for (int i = 0; i < kernelSize; ++i) {
        for (int j = 0; j < kernelSize; ++j) {
            kernel[i][j] /= sum;
        }
    }

    // Apply
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double r = 0.0;
            double g = 0.0;
            double b = 0.0;

            for (int wy = -radius; wy <= radius; ++wy) {
                for (int wx = -radius; wx <= radius; ++wx) {
                    // Handle edges
                    int px = std::clamp(x + wx, 0, width - 1);
                    int py = std::clamp(y + wy, 0, height - 1);

                    QRgb pixel = src.pixel(px, py);

                    double weight = kernel[wy + radius][wx + radius];

                    r += qRed(pixel)   * weight;
                    g += qGreen(pixel) * weight;
                    b += qBlue(pixel)  * weight;
                }
            }

            // Write back
            result.setPixel(x, y, qRgb(
                std::clamp(static_cast<int>(r), 0, 255),
                std::clamp(static_cast<int>(g), 0, 255),
                std::clamp(static_cast<int>(b), 0, 255)
            ));
        }
    }

    return result;
}

QImage ImageProcessor::applyBasicSharpeningFilter(const QImage& src, int k) {
    double c = k + 1;
    double b = -k / 8.0;
    Kernel kernel = { { b, b, b }, { b, c, b }, { b, b, b } };

    return applyKernel(src, kernel);
}

QImage ImageProcessor::applySobelFilter(const QImage& src, int k) {
    Kernel kernelY = { { 1, 2, 1 }, { 0, 0, 0 }, { -1, -2, -1 } };
    Kernel kernelX = { { 1, 0, -1 }, { 2, 0, -2 }, { 1, 0, -1 } };

    QImage edges = applyEdgeDetection(src, kernelX, kernelY);
    return addWeighted(src, 1.0, edges, k / 20.0, 0.0);
}

QImage ImageProcessor::applyPrewittFilter(const QImage& src, int k) {
    Kernel kernelY = { { 1, 1, 1 }, { 0, 0, 0 }, { -1, -1, -1 } };
    Kernel kernelX = { { 1, 0, -1 }, { 1, 0, -1 }, { 1, 0, -1 } };

    QImage edges = applyEdgeDetection(src, kernelX, kernelY);
    return addWeighted(src, 1.0, edges, k / 20.0, 0.0);
}

QImage ImageProcessor::applyRobertsFilter(const QImage& src, int k) {
    Kernel kernelY = { { 0, 0, 0 }, { 0, -1, 0 }, { 0, 0, 1 } };
    Kernel kernelX = { { 0, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 } };

    QImage edges = applyEdgeDetection(src, kernelX, kernelY);
    return addWeighted(src, 1.0, edges, k / 20.0, 0.0);
}

QImage ImageProcessor::applyLaplacian(const QImage& src, int k) {
    double strength = k / 20.0;
    double center = 1.0 + 4.0 * strength;
    double neighbor = -strength;

    Kernel combinedKernel = {
        {    0.0,  neighbor,      0.0 },
        { neighbor,  center, neighbor },
        {    0.0,  neighbor,      0.0 }
    };

    return applyKernel(src, combinedKernel);
}

QImage ImageProcessor::applyEdgeDetection(const QImage& src, const Kernel& kx, const Kernel& ky) {
    int width = src.width();
    int height = src.height();

    int radius = 1; // 3x3 matrix

    QImage result = src.copy();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double gx = 0;
            double gy = 0;

            for (int j = -radius; j <= radius; ++j) {
                for (int i = -radius; i <= radius; ++i) {
                    int nx = std::clamp(x + i, 0, width - 1);
                    int ny = std::clamp(y + j, 0, height - 1);

                    int intensity = qGray(src.pixel(nx, ny));

                    gx += intensity * kx[j + radius][i + radius];
                    gy += intensity * ky[j + radius][i + radius];
                }
            }

            // Calculate magnitude
            double magnitude = std::sqrt(gx * gx + gy * gy);
            int val = std::clamp(static_cast<int>(magnitude), 0, 255);

            result.setPixel(x, y, qRgb(val, val, val));
        }
    }

    return result;
}

QImage ImageProcessor::addWeighted(const QImage& src1, double alpha, const QImage& src2, double beta, double gamma) {
    if (src1.size() != src2.size()) return src1;

    QImage result = src1.copy();
    int w = src1.width();
    int h = src1.height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            QRgb p1 = src1.pixel(x, y);
            QRgb p2 = src2.pixel(x, y);

            int r = static_cast<int>(qRed(p1) * alpha + qRed(p2) * beta + gamma);
            int g = static_cast<int>(qGreen(p1) * alpha + qGreen(p2) * beta + gamma);
            int b = static_cast<int>(qBlue(p1) * alpha + qBlue(p2) * beta + gamma);

            result.setPixel(x, y, qRgb(std::clamp(r, 0, 255),
                                       std::clamp(g, 0, 255),
                                       std::clamp(b, 0, 255)));
        }
    }

    return result;
}

QImage ImageProcessor::applyKernel(const QImage& src, const std::vector<std::vector<double>>& kernel) {
    int kernelHeight = kernel.size();
    int kernelWidth = kernel[0].size();
    int radiusY = kernelHeight / 2;
    int radiusX = kernelWidth / 2;

    int w = src.width();
    int h = src.height();
    QImage result = src.copy();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double rSum = 0, gSum = 0, bSum = 0;

            for (int ky = -radiusY; ky <= radiusY; ++ky) {
                for (int kx = -radiusX; kx <= radiusX; ++kx) {

                    int nx = std::clamp(x + kx, 0, w - 1);
                    int ny = std::clamp(y + ky, 0, h - 1);

                    QRgb px = src.pixel(nx, ny);

                    double weight = kernel[ky + radiusY][kx + radiusX];

                    rSum += qRed(px) * weight;
                    gSum += qGreen(px) * weight;
                    bSum += qBlue(px) * weight;
                }
            }

            result.setPixel(x, y, qRgb(
                std::clamp(static_cast<int>(rSum), 0, 255),
                std::clamp(static_cast<int>(gSum), 0, 255),
                std::clamp(static_cast<int>(bSum), 0, 255)
            ));
        }
    }
    return result;
}

QImage ImageProcessor::applyGlassEffect(const QImage& src, int radius) {
    QImage result = src;
    int w = src.width();
    int h = src.height();

    std::uniform_int_distribution<int> dist(-radius, radius);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int dx = dist(rng);
            int dy = dist(rng);

            int sx = std::clamp(x + dx, 0, w - 1);
            int sy = std::clamp(y + dy, 0, h - 1);

            result.setPixel(x, y, src.pixel(sx, sy));
        }
    }

    return result;
}














