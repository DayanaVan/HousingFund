/********************************************************************************
** Form generated from reading UI file 'AuthDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTHDIALOG_H
#define UI_AUTHDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AuthDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelLogin;
    QLineEdit *txtLogin;
    QLabel *labelPassword;
    QLineEdit *txtPassword;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnLogin;
    QPushButton *btnCancel;

    void setupUi(QDialog *AuthDialog)
    {
        if (AuthDialog->objectName().isEmpty())
            AuthDialog->setObjectName("AuthDialog");
        AuthDialog->resize(350, 200);
        verticalLayout = new QVBoxLayout(AuthDialog);
        verticalLayout->setObjectName("verticalLayout");
        labelLogin = new QLabel(AuthDialog);
        labelLogin->setObjectName("labelLogin");

        verticalLayout->addWidget(labelLogin);

        txtLogin = new QLineEdit(AuthDialog);
        txtLogin->setObjectName("txtLogin");

        verticalLayout->addWidget(txtLogin);

        labelPassword = new QLabel(AuthDialog);
        labelPassword->setObjectName("labelPassword");

        verticalLayout->addWidget(labelPassword);

        txtPassword = new QLineEdit(AuthDialog);
        txtPassword->setObjectName("txtPassword");
        txtPassword->setEchoMode(QLineEdit::Password);

        verticalLayout->addWidget(txtPassword);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        btnLogin = new QPushButton(AuthDialog);
        btnLogin->setObjectName("btnLogin");

        horizontalLayout->addWidget(btnLogin);

        btnCancel = new QPushButton(AuthDialog);
        btnCancel->setObjectName("btnCancel");

        horizontalLayout->addWidget(btnCancel);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(AuthDialog);

        QMetaObject::connectSlotsByName(AuthDialog);
    } // setupUi

    void retranslateUi(QDialog *AuthDialog)
    {
        AuthDialog->setWindowTitle(QCoreApplication::translate("AuthDialog", "\320\220\320\262\321\202\320\276\321\200\320\270\320\267\320\260\321\206\320\270\321\217", nullptr));
        labelLogin->setText(QCoreApplication::translate("AuthDialog", "\320\233\320\276\320\263\320\270\320\275:", nullptr));
        txtLogin->setPlaceholderText(QCoreApplication::translate("AuthDialog", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\273\320\276\320\263\320\270\320\275", nullptr));
        labelPassword->setText(QCoreApplication::translate("AuthDialog", "\320\237\320\260\321\200\320\276\320\273\321\214:", nullptr));
        txtPassword->setPlaceholderText(QCoreApplication::translate("AuthDialog", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\277\320\260\321\200\320\276\320\273\321\214", nullptr));
        btnLogin->setText(QCoreApplication::translate("AuthDialog", "\320\222\320\276\320\271\321\202\320\270", nullptr));
        btnCancel->setText(QCoreApplication::translate("AuthDialog", "\320\236\321\202\320\274\320\265\320\275\320\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AuthDialog: public Ui_AuthDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTHDIALOG_H
