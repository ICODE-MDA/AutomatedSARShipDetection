/********************************************************************************
** Form generated from reading UI file 'MultiImageDialog.ui'
**
** Created: Mon Jan 7 10:23:42 2013
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MULTIIMAGEDIALOG_H
#define UI_MULTIIMAGEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MultiImageDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QTabWidget *m_tabWidget;
    QWidget *m_imageSummaryTab;
    QTableWidget *m_imageTable;
    QWidget *m_pointEditorTab;
    QTableWidget *m_pointTable;
    QPushButton *m_addPointButton;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *m_currentPointID;
    QFrame *frame_2;
    QWidget *m_RegistrationTab;
    QPushButton *m_registerButton;
    QTextBrowser *m_regResultsBrowser;
    QWidget *m_pointPositionTab;
    QPushButton *m_dropButton;
    QLabel *m_notCertified;
    QTextBrowser *m_pointPositionBrowser;
    QWidget *m_mensurationTab;
    QLabel *m_notCertified_3;
    QHBoxLayout *m_botPanel;
    QPushButton *m_resetButton;
    QSpacerItem *horizontalSpacer_1;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *m_hideButton;

    void setupUi(QDialog *MultiImageDialog)
    {
        if (MultiImageDialog->objectName().isEmpty())
            MultiImageDialog->setObjectName(QString::fromUtf8("MultiImageDialog"));
        MultiImageDialog->resize(710, 359);
        verticalLayout_3 = new QVBoxLayout(MultiImageDialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_tabWidget = new QTabWidget(MultiImageDialog);
        m_tabWidget->setObjectName(QString::fromUtf8("m_tabWidget"));
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        m_tabWidget->setAutoFillBackground(false);
        m_tabWidget->setTabPosition(QTabWidget::North);
        m_tabWidget->setTabShape(QTabWidget::Rounded);
        m_imageSummaryTab = new QWidget();
        m_imageSummaryTab->setObjectName(QString::fromUtf8("m_imageSummaryTab"));
        m_imageSummaryTab->setEnabled(true);
        m_imageSummaryTab->setAutoFillBackground(true);
        m_imageTable = new QTableWidget(m_imageSummaryTab);
        m_imageTable->setObjectName(QString::fromUtf8("m_imageTable"));
        m_imageTable->setEnabled(true);
        m_imageTable->setGeometry(QRect(20, 20, 640, 220));
        m_imageTable->viewport()->setProperty("cursor", QVariant(QCursor(Qt::ArrowCursor)));
        m_imageTable->setFocusPolicy(Qt::NoFocus);
        m_imageTable->setFrameShape(QFrame::StyledPanel);
        m_imageTable->setFrameShadow(QFrame::Raised);
        m_imageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_imageTable->setTabKeyNavigation(false);
        m_imageTable->setProperty("showDropIndicator", QVariant(false));
        m_imageTable->setDragDropOverwriteMode(false);
        m_imageTable->setAlternatingRowColors(true);
        m_imageTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        m_imageTable->setWordWrap(false);
        m_imageTable->setCornerButtonEnabled(false);
        m_imageTable->horizontalHeader()->setHighlightSections(false);
        m_imageTable->verticalHeader()->setHighlightSections(false);
        m_tabWidget->addTab(m_imageSummaryTab, QString());
        m_pointEditorTab = new QWidget();
        m_pointEditorTab->setObjectName(QString::fromUtf8("m_pointEditorTab"));
        m_pointEditorTab->setEnabled(true);
        m_pointTable = new QTableWidget(m_pointEditorTab);
        m_pointTable->setObjectName(QString::fromUtf8("m_pointTable"));
        m_pointTable->setGeometry(QRect(23, 30, 551, 211));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_pointTable->sizePolicy().hasHeightForWidth());
        m_pointTable->setSizePolicy(sizePolicy);
        m_pointTable->setLayoutDirection(Qt::LeftToRight);
        m_pointTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_pointTable->setTabKeyNavigation(false);
        m_pointTable->setProperty("showDropIndicator", QVariant(false));
        m_pointTable->setDragDropOverwriteMode(false);
        m_pointTable->setAlternatingRowColors(true);
        m_pointTable->setWordWrap(false);
        m_pointTable->setCornerButtonEnabled(false);
        m_addPointButton = new QPushButton(m_pointEditorTab);
        m_addPointButton->setObjectName(QString::fromUtf8("m_addPointButton"));
        m_addPointButton->setGeometry(QRect(577, 30, 91, 32));
        label_4 = new QLabel(m_pointEditorTab);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(2, 95, 16, 81));
        label_4->setAlignment(Qt::AlignCenter);
        label_5 = new QLabel(m_pointEditorTab);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(273, 10, 51, 16));
        m_currentPointID = new QLabel(m_pointEditorTab);
        m_currentPointID->setObjectName(QString::fromUtf8("m_currentPointID"));
        m_currentPointID->setGeometry(QRect(590, 78, 62, 16));
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        font.setWeight(75);
        m_currentPointID->setFont(font);
        m_currentPointID->setAlignment(Qt::AlignCenter);
        frame_2 = new QFrame(m_pointEditorTab);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setGeometry(QRect(590, 70, 61, 31));
        frame_2->setFrameShape(QFrame::Box);
        frame_2->setFrameShadow(QFrame::Raised);
        m_tabWidget->addTab(m_pointEditorTab, QString());
        frame_2->raise();
        m_pointTable->raise();
        m_addPointButton->raise();
        label_4->raise();
        label_5->raise();
        m_currentPointID->raise();
        m_RegistrationTab = new QWidget();
        m_RegistrationTab->setObjectName(QString::fromUtf8("m_RegistrationTab"));
        m_registerButton = new QPushButton(m_RegistrationTab);
        m_registerButton->setObjectName(QString::fromUtf8("m_registerButton"));
        m_registerButton->setGeometry(QRect(570, 10, 94, 32));
        m_regResultsBrowser = new QTextBrowser(m_RegistrationTab);
        m_regResultsBrowser->setObjectName(QString::fromUtf8("m_regResultsBrowser"));
        m_regResultsBrowser->setGeometry(QRect(10, 10, 551, 231));
        m_tabWidget->addTab(m_RegistrationTab, QString());
        m_pointPositionTab = new QWidget();
        m_pointPositionTab->setObjectName(QString::fromUtf8("m_pointPositionTab"));
        m_dropButton = new QPushButton(m_pointPositionTab);
        m_dropButton->setObjectName(QString::fromUtf8("m_dropButton"));
        m_dropButton->setGeometry(QRect(550, 10, 114, 32));
        m_notCertified = new QLabel(m_pointPositionTab);
        m_notCertified->setObjectName(QString::fromUtf8("m_notCertified"));
        m_notCertified->setGeometry(QRect(198, 206, 281, 20));
        QFont font1;
        font1.setPointSize(18);
        font1.setBold(false);
        font1.setWeight(50);
        m_notCertified->setFont(font1);
        m_pointPositionBrowser = new QTextBrowser(m_pointPositionTab);
        m_pointPositionBrowser->setObjectName(QString::fromUtf8("m_pointPositionBrowser"));
        m_pointPositionBrowser->setGeometry(QRect(20, 0, 521, 192));
        m_pointPositionBrowser->setFrameShadow(QFrame::Sunken);
        m_tabWidget->addTab(m_pointPositionTab, QString());
        m_mensurationTab = new QWidget();
        m_mensurationTab->setObjectName(QString::fromUtf8("m_mensurationTab"));
        m_notCertified_3 = new QLabel(m_mensurationTab);
        m_notCertified_3->setObjectName(QString::fromUtf8("m_notCertified_3"));
        m_notCertified_3->setGeometry(QRect(245, 100, 191, 20));
        m_notCertified_3->setFont(font1);
        m_tabWidget->addTab(m_mensurationTab, QString());

        horizontalLayout->addWidget(m_tabWidget);


        verticalLayout_3->addLayout(horizontalLayout);

        m_botPanel = new QHBoxLayout();
        m_botPanel->setObjectName(QString::fromUtf8("m_botPanel"));
        m_botPanel->setSizeConstraint(QLayout::SetNoConstraint);
        m_resetButton = new QPushButton(MultiImageDialog);
        m_resetButton->setObjectName(QString::fromUtf8("m_resetButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_resetButton->sizePolicy().hasHeightForWidth());
        m_resetButton->setSizePolicy(sizePolicy1);

        m_botPanel->addWidget(m_resetButton);

        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_botPanel->addItem(horizontalSpacer_1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_botPanel->addItem(horizontalSpacer_2);

        m_hideButton = new QPushButton(MultiImageDialog);
        m_hideButton->setObjectName(QString::fromUtf8("m_hideButton"));
        sizePolicy1.setHeightForWidth(m_hideButton->sizePolicy().hasHeightForWidth());
        m_hideButton->setSizePolicy(sizePolicy1);

        m_botPanel->addWidget(m_hideButton);


        verticalLayout_3->addLayout(m_botPanel);


        retranslateUi(MultiImageDialog);

        m_tabWidget->setCurrentIndex(4);


        QMetaObject::connectSlotsByName(MultiImageDialog);
    } // setupUi

    void retranslateUi(QDialog *MultiImageDialog)
    {
        MultiImageDialog->setWindowTitle(QApplication::translate("MultiImageDialog", "Metric Exploitation", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(m_imageSummaryTab), QApplication::translate("MultiImageDialog", "Image Summary", 0, QApplication::UnicodeUTF8));
        m_addPointButton->setText(QApplication::translate("MultiImageDialog", "New Point", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MultiImageDialog", "I\n"
"m\n"
"a\n"
"g\n"
"e", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MultiImageDialog", "P o i n t", 0, QApplication::UnicodeUTF8));
        m_currentPointID->setText(QApplication::translate("MultiImageDialog", "-", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(m_pointEditorTab), QApplication::translate("MultiImageDialog", "Point Editor", 0, QApplication::UnicodeUTF8));
        m_registerButton->setText(QApplication::translate("MultiImageDialog", "Register", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(m_RegistrationTab), QApplication::translate("MultiImageDialog", "Registration", 0, QApplication::UnicodeUTF8));
        m_dropButton->setText(QApplication::translate("MultiImageDialog", "Drop Point", 0, QApplication::UnicodeUTF8));
        m_notCertified->setText(QApplication::translate("MultiImageDialog", "<html><head/><body><p><span style=\" font-style:italic; color:#0000ff;\">NOT CERTIFIED FOR TARGETING</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(m_pointPositionTab), QApplication::translate("MultiImageDialog", "Point Position", 0, QApplication::UnicodeUTF8));
        m_notCertified_3->setText(QApplication::translate("MultiImageDialog", "<html><head/><body><p>To Be Implemented...</p></body></html>", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(m_mensurationTab), QApplication::translate("MultiImageDialog", "Mensuration", 0, QApplication::UnicodeUTF8));
        m_resetButton->setText(QApplication::translate("MultiImageDialog", "Reset", 0, QApplication::UnicodeUTF8));
        m_hideButton->setText(QApplication::translate("MultiImageDialog", "Hide", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MultiImageDialog: public Ui_MultiImageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MULTIIMAGEDIALOG_H
