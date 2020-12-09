/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_input_output_mapper.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>

KisInputOutputMapper::KisInputOutputMapper(KisImageWSP image, KisNodeSP activeNode):m_image(image),m_activeNode(activeNode)
{

}


KisNodeListSP KisInputOutputMapper::inputNodes(InputLayerMode inputMode)
{
/*
    ACTIVE_LAYER,
    ALL_LAYERS,
    ACTIVE_LAYER_BELOW_LAYER,
    ACTIVE_LAYER_ABOVE_LAYER,
    ALL_VISIBLE_LAYERS,
    ALL_INVISIBLE_LAYERS,
    ALL_VISIBLE_LAYERS_DECR,
    ALL_INVISIBLE_DECR,
    ALL_DECR
*/

    KisNodeListSP result(new QList< KisNodeSP >());
    switch (inputMode)
    {
        case ACTIVE_LAYER:
        {
            result->append(m_activeNode);
            break;// drop down in case of one more layer modes
        }
        case ACTIVE_LAYER_BELOW_LAYER:
        {
            result->append(m_activeNode);
            result->append(m_activeNode->prevSibling());
            break;
        }
        case ACTIVE_LAYER_ABOVE_LAYER:
        {
            result->append(m_activeNode);
            result->append(m_activeNode->nextSibling());
            break;
        }
        case NONE:
        case ALL_VISIBLE_LAYERS:
        case ALL_INVISIBLE_LAYERS:
        case ALL_VISIBLE_LAYERS_DECR:
        case ALL_INVISIBLE_DECR:
        {
            qWarning() << "Inputmode" << inputMode << "not implemented";
            break;
        }
        case ALL_LAYERS:
        {
            allLayers(result);
            break;
        }
        case ALL_DECR:
        {
            allInverseOrderedLayers(result);
            break;
        }
        default:
        {
            Q_ASSERT(false); // why here??
            break;
        }
    }
    return result;
}


void KisInputOutputMapper::allLayers(KisNodeListSP result)
{
    //TODO: hack ignores hierarchy introduced by group layers
    KisNodeSP root = m_image->rootLayer();
    KisNodeSP item = root->lastChild();
    while (item)
    {
        KisPaintLayer * paintLayer = dynamic_cast<KisPaintLayer*>(item.data());
        if (paintLayer)
        {
            result->append(item);
        }
        item = item->prevSibling();
    }
}


void KisInputOutputMapper::allInverseOrderedLayers(KisNodeListSP result)
{
    qWarning() << "allInverseOrderedLayers not implemented";
    Q_UNUSED(result);
}
