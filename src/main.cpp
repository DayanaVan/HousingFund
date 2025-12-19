#include <QApplication>
#include <QMessageBox>
#include "database/DatabaseManager.h"
#include "ui/MainWindow.h"
#include "ui/AuthDialog.h"
#include "config/Config.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("HousingFund");
    app.setOrganizationName("NovosibirskStateTechUniversity");
    
    // Подключение к базе данных
    DatabaseManager dbManager(Config::getConnectionString());
    if (!dbManager.connect()) {
        QMessageBox::critical(nullptr, "Ошибка", 
            "Не удалось подключиться к базе данных!\n"
            "Проверьте:\n"
            "1. Запущен ли PostgreSQL\n"
            "2. Существует ли база 'housing_fund'\n"
            "3. Правильность настроек в Config.h");
        return 1;
    }
    
    // Окно авторизации
    AuthDialog authDialog(&dbManager);
    if (authDialog.exec() != QDialog::Accepted) {
        return 0; // Пользователь отменил вход
    }
    
    // Главное окно
    MainWindow mainWindow(&dbManager);
    mainWindow.show();
    
    return app.exec();
}
