#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include "../database/DatabaseManager.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
class AuthDialog;
}
QT_END_NAMESPACE

class AuthDialog : public QDialog {
    Q_OBJECT

public:
    explicit AuthDialog(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~AuthDialog();
    string getCurrentUserLogin() const;
    bool isAdmin() const;

private slots:
    void onLoginClicked();
    void onCancelClicked();
    void onRegisterClicked();

private:
    bool validateInput();
    bool checkDatabaseManager() const;

    Ui::AuthDialog* ui;
    DatabaseManager* dbManager;
    string currentUserLogin;
    bool userIsAdmin;
};

#endif 
