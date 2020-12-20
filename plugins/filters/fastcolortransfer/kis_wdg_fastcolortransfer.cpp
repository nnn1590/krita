/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_fastcolortransfer.h"

#include <QLayout>

#include <KisImportExportFilter.h>
#include <KisImportExportManager.h>

#include <filter/kis_filter_configuration.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_paint_device.h>
#include <kundo2command.h>
#include <KoColorSpaceRegistry.h>
#include <kis_file_name_requester.h>
#include "ui_wdgfastcolortransfer.h"
#include <KisGlobalResourcesInterface.h>

KisWdgFastColorTransfer::KisWdgFastColorTransfer(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgFastColorTransfer();
    m_widget->setupUi(this);
    m_widget->fileNameURLRequester->setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    connect(m_widget->fileNameURLRequester, SIGNAL(textChanged(QString)), this, SIGNAL(sigConfigurationItemChanged()));
}


KisWdgFastColorTransfer::~KisWdgFastColorTransfer()
{
    delete m_widget;
}

void KisWdgFastColorTransfer::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("filename", value)) {
        widget()->fileNameURLRequester->setFileName(value.toString());
    }

}

KisPropertiesConfigurationSP KisWdgFastColorTransfer::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("colortransfer", 1, KisGlobalResourcesInterface::instance());
    QString fileName = this->widget()->fileNameURLRequester->fileName();

    if (fileName.isEmpty()) return config;

    KisPaintDeviceSP ref;

    dbgPlugins << "Use as reference file : " << fileName;

    KisDocument *d = KisPart::instance()->createDocument();

    KisImportExportManager manager(d);
    KisImportExportErrorCode status = manager.importDocument(fileName, QString());
    dbgPlugins << "import returned status" << status.errorMessage();
    KisImageWSP importedImage = d->image();

    if (importedImage) {
        ref = importedImage->projection();
    }
    if (!ref) {
        dbgPlugins << "No reference image was specified.";
        delete d;
        return config;
    }

    // Convert ref to LAB
    const KoColorSpace* labCS = KoColorSpaceRegistry::instance()->lab16();
    if (!labCS) {
        dbgPlugins << "The LAB colorspace is not available.";
        delete d;
        return config;
    }

    dbgPlugins << "convert ref to lab";
    ref->convertTo(labCS, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    // Compute the means and sigmas of ref
    double meanL_ref = 0., meanA_ref = 0., meanB_ref = 0.;
    double sigmaL_ref = 0., sigmaA_ref = 0., sigmaB_ref = 0.;

    KisSequentialConstIterator refIt(ref, importedImage->bounds());
    while (refIt.nextPixel()) {
        const quint16* data = reinterpret_cast<const quint16*>(refIt.oldRawData());

        quint32 L = data[0];
        quint32 A = data[1];
        quint32 B = data[2];

        meanL_ref += L;
        meanA_ref += A;
        meanB_ref += B;

        sigmaL_ref += L * L;
        sigmaA_ref += A * A;
        sigmaB_ref += B * B;

    }

    double totalSize = 1. / (importedImage->width() * importedImage->height());

    meanL_ref *= totalSize;
    meanA_ref *= totalSize;
    meanB_ref *= totalSize;
    sigmaL_ref *= totalSize;
    sigmaA_ref *= totalSize;
    sigmaB_ref *= totalSize;

    dbgPlugins << totalSize << "" << meanL_ref << "" << meanA_ref << "" << meanB_ref << "" << sigmaL_ref << "" << sigmaA_ref << "" << sigmaB_ref;

    config->setProperty("filename", fileName);
    config->setProperty("meanL", meanL_ref);
    config->setProperty("meanA", meanA_ref);
    config->setProperty("meanB", meanB_ref);
    config->setProperty("sigmaL", sigmaL_ref);
    config->setProperty("sigmaA", sigmaA_ref);
    config->setProperty("sigmaB", sigmaB_ref);

    delete d;

    return config;
}
