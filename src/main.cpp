#include <QApplication>
#include <QMessageBox>
#include "database/DatabaseManager.h"
#include "ui/MainWindow.h"
#include "ui/AuthDialog.h"
#include "config/Config.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Подключение к базе данных
    DatabaseManager dbManager(Config::getConnectionString());
    if (!dbManager.connect()) {
        QMessageBox::critical(nullptr, "Ошибка", 
            "Не удалось подключиться к базе данных");
        return 1;
    }
    
    // Окно авторизации
    AuthDialog authDialog(&dbManager);
    if (authDialog.exec() != QDialog::Accepted) {
        return 0; 
    }
    
    // Главное окно
    MainWindow mainWindow(&dbManager);
    mainWindow.show();
    
    return app.exec();
}
