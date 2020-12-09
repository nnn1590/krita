/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_dlg_options_tiff.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QSlider>
#include <QStackedWidget>
#include <QApplication>

#include <kcombobox.h>
#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>

KisTIFFOptionsWidget::KisTIFFOptionsWidget(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);
    activated(0);
    connect(kComboBoxCompressionType, SIGNAL(activated(int)), this, SLOT(activated(int)));
    connect(flatten, SIGNAL(toggled(bool)), this, SLOT(flattenToggled(bool)));
    QApplication::restoreOverrideCursor();
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
}

void KisTIFFOptionsWidget::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    kComboBoxCompressionType->setCurrentIndex(cfg->getInt("compressiontype", 0));
    activated(kComboBoxCompressionType->currentIndex());
    kComboBoxPredictor->setCurrentIndex(cfg->getInt("predictor", 0));
    alpha->setChecked(cfg->getBool("alpha", true));
    flatten->setChecked(cfg->getBool("flatten", true));
    flattenToggled(flatten->isChecked());
    qualityLevel->setValue(cfg->getInt("quality", 80));
    compressionLevelDeflate->setValue(cfg->getInt("deflate", 6));
    compressionLevelPixarLog->setValue(cfg->getInt("pixarlog", 6));
    chkSaveProfile->setChecked(cfg->getBool("saveProfile", true));

    if (cfg->getInt("type", -1) == KoChannelInfo::FLOAT16 || cfg->getInt("type", -1) == KoChannelInfo::FLOAT32) {
        kComboBoxPredictor->removeItem(1);
    } else {
        kComboBoxPredictor->removeItem(2);
    }

    if (cfg->getBool("isCMYK")) {
        alpha->setChecked(false);
        alpha->setEnabled(false);
    }


}

KisPropertiesConfigurationSP KisTIFFOptionsWidget::configuration() const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    cfg->setProperty("compressiontype", kComboBoxCompressionType->currentIndex());
    cfg->setProperty("predictor", kComboBoxPredictor->currentIndex());
    cfg->setProperty("alpha", alpha->isChecked());
    cfg->setProperty("flatten", flatten->isChecked());
    cfg->setProperty("quality", qualityLevel->value());
    cfg->setProperty("deflate", compressionLevelDeflate->value());
    cfg->setProperty("pixarlog", compressionLevelPixarLog->value());
    cfg->setProperty("saveProfile", chkSaveProfile->isChecked());

    return cfg;
}

void KisTIFFOptionsWidget::activated(int index)
{
    switch (index) {
    case 1:
        codecsOptionsStack->setCurrentIndex(1);
        break;
    case 2:
        codecsOptionsStack->setCurrentIndex(2);
        break;
    case 4:
        codecsOptionsStack->setCurrentIndex(4);
        break;
    default:
        codecsOptionsStack->setCurrentIndex(0);
    }
}

void KisTIFFOptionsWidget::flattenToggled(bool t)
{
    alpha->setEnabled(t);
    if (!t) {
        alpha->setChecked(true);
    }
}

