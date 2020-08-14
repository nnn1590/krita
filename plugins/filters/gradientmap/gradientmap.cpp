/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "QObject"
#include "gradientmap.h"
#include <kpluginfactory.h>
#include <filter/kis_filter_registry.h>
#include "krita_filter_gradient_map.h"
#include "KoResourceServerProvider.h"
#include "kis_config_widget.h"
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoGradientBackground.h>
#include <KisGlobalResourcesInterface.h>

#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaGradientMapFactory, "kritagradientmap.json", registerPlugin<KritaGradientMap>();)

KritaGradientMapConfigWidget::KritaGradientMapConfigWidget(QWidget *parent, KisPaintDeviceSP dev, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
{
    Q_UNUSED(dev);
    m_page = new WdgGradientMap(this);
    QHBoxLayout *l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);
    l->addWidget(m_page);
    l->setContentsMargins(0, 0, 0, 0);

    m_gradientChangedCompressor = new KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE);

    m_gradientPopUp = new KoResourcePopupAction(ResourceType::Gradients, 0, m_page->btnGradientChooser);

    m_activeGradient = KoStopGradient::fromQGradient(m_gradientPopUp->currentResource().dynamicCast<KoAbstractGradient>()->toQGradient());
    m_page->gradientEditor->setGradient(m_activeGradient);
    m_page->gradientEditor->setCompactMode(true);
    m_page->gradientEditor->setEnabled(true);
    m_page->btnGradientChooser->setDefaultAction(m_gradientPopUp);
    m_page->btnGradientChooser->setPopupMode(QToolButton::InstantPopup);
    connect(m_gradientPopUp, SIGNAL(resourceSelected(QSharedPointer<KoShapeBackground>)), this, SLOT(setAbstractGradientToEditor()));
    connect(m_page->gradientEditor, SIGNAL(sigGradientChanged()), m_gradientChangedCompressor, SLOT(start()));
    connect(m_gradientChangedCompressor, SIGNAL(timeout()), this, SIGNAL(sigConfigurationItemChanged()));

    QObject::connect(m_page->colorModeComboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);
    QObject::connect(m_page->ditherWidget, &KisDitherWidget::sigConfigurationItemChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
}

KritaGradientMapConfigWidget::~KritaGradientMapConfigWidget()
{
    delete m_page;
}

void KritaGradientMapConfigWidget::setAbstractGradientToEditor()
{
    QSharedPointer<KoGradientBackground> bg =
        qSharedPointerDynamicCast<KoGradientBackground>(
            m_gradientPopUp->currentBackground());
    m_activeGradient = KoStopGradient::fromQGradient(bg->gradient());
    m_page->gradientEditor->setGradient(m_activeGradient);

}

KisPropertiesConfigurationSP KritaGradientMapConfigWidget::configuration() const
{
    KisFilterSP filter = KisFilterRegistry::instance()->get("gradientmap");
    KisFilterConfigurationSP cfg = filter->factoryConfiguration(KisGlobalResourcesInterface::instance());
    if (m_activeGradient) {
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        m_activeGradient->toXML(doc, elt);
        doc.appendChild(elt);
        cfg->setProperty("gradientXML", doc.toString());
    }

    cfg->setProperty("colorMode", m_page->colorModeComboBox->currentIndex());
    m_page->ditherWidget->configuration(*cfg, "dither/");

    return cfg;
}

//-----------------------------

void KritaGradientMapConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    Q_ASSERT(config);
    QDomDocument doc;
    if (config->hasProperty("gradientXML")) {
        doc.setContent(config->getString("gradientXML", ""));
        KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());
        if (gradient.stops().size() > 0) {
            m_activeGradient->setStops(gradient.stops());
            m_activeGradient->updateVariableColors(m_page->gradientEditor->canvasResourcesInterface());
        }
        m_page->gradientEditor->setGradient(m_activeGradient);
    }

    m_page->colorModeComboBox->setCurrentIndex(config->getInt("colorMode"));

    const KisFilterConfiguration *filterConfig = dynamic_cast<const KisFilterConfiguration*>(config.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);
    m_page->ditherWidget->setConfiguration(*filterConfig, "dither/");
}

void KritaGradientMapConfigWidget::setView(KisViewManager *view)
{
    m_page->gradientEditor->setCanvasResourcesInterface(view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    m_gradientPopUp->setCanvasResourcesInterface(view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    m_activeGradient->updateVariableColors(m_page->gradientEditor->canvasResourcesInterface());
}
//------------------------------
KritaGradientMap::KritaGradientMap(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KritaFilterGradientMap()));
}

KritaGradientMap::~KritaGradientMap()
{
}

//-----------------------------



#include "gradientmap.moc"
