#ifndef HG_KIS_PAINTING_ASSISTANTS_DECORATION_TEST_H
#define HG_KIS_PAINTING_ASSISTANTS_DECORATION_TEST_H

#include <QTest>

class KisDocument;
class KisMainWindow;
class KisView;
class KisViewManager;

class KisPaintingAssistantsDecorationTest : public QObject {
    Q_OBJECT

    public:
        KisDocument* m_document;
        KisMainWindow* m_mainWindow;
        QPointer<KisView> m_view;
        KisViewManager* m_viewManager;

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testParallelRulerAdjustPosition();
        void testPerspectiveAssistant();

};

#endif /* HG_KIS_PAINTING_ASSISTANTS_DECORATION_TEST_H */



