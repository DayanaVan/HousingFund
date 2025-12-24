#include "AuthDialog.h"
#include "./ui_AuthDialog.h"
#include "../utils/HashUtils.h"
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDebug>

using namespace std;

AuthDialog::AuthDialog(DatabaseManager* dbManager, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuthDialog)
    , dbManager(dbManager)
    , currentUserLogin("")
    , userIsAdmin(false)
{
    ui->setupUi(this);
    
    setWindowTitle("Авторизация");
    setModal(true);
    
    setFixedSize(350, 200);

    QPushButton* registerButton = new QPushButton("Регистрация", this);
    ui->horizontalLayout->addWidget(registerButton);
    
    // Подключаем сигналы
    connect(ui->btnLogin, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &AuthDialog::onCancelClicked);
    connect(registerButton, &QPushButton::clicked, this, &AuthDialog::onRegisterClicked);
    
    connect(ui->txtLogin, &QLineEdit::returnPressed, this, &AuthDialog::onLoginClicked);
    connect(ui->txtPassword, &QLineEdit::returnPressed, this, &AuthDialog::onLoginClicked);
}

AuthDialog::~AuthDialog() {
    delete ui;
}

bool AuthDialog::checkDatabaseManager() const {
    if (!dbManager) {
        qDebug() << "ОШИБКА: DatabaseManager не инициализирован!";
        return false;
    }
    return true;
}

void AuthDialog::onLoginClicked() {
    qDebug() << "Нажата кнопка Войти";
    
    if (!checkDatabaseManager()) {
        QMessageBox::critical(this, "Ошибка", "Ошибка базы данных");
        return;
    }
    
    if (!validateInput()) {
        return;
    }
    
    // Получаем значения из полей 
    string login = ui->txtLogin->text().trimmed().toStdString();
    string password = ui->txtPassword->text().toStdString();
    
    qDebug() << "Попытка входа для пользователя:" << login.c_str();
    
    // Получаем пользователя из БД
    User user = dbManager->getUserByLogin(login);
    if (user.id == 0) {
        QMessageBox::warning(this, "Ошибка", "Пользователь не найден");
        return;
    }
    
    // Проверяем пароль
    string passwordHash = HashUtils::hashPassword(password, user.salt);
    if (passwordHash != user.passwordHash) {
        QMessageBox::warning(this, "Ошибка", "Неверный пароль");
        return;
    }
    
    currentUserLogin = login;
    userIsAdmin = false;
    
    qDebug() << "Успешный вход. Пользователь:" << login.c_str();
    
    accept(); 
}

void AuthDialog::onCancelClicked() {
    qDebug() << "Нажата кнопка Отмена";
    reject(); 
}

void AuthDialog::onRegisterClicked() {
    qDebug() << "Нажата кнопка Регистрация";
    
    if (!checkDatabaseManager()) {
        QMessageBox::critical(this, "Ошибка", "Ошибка базы данных");
        return;
    }
    
    // Создаем диалог регистрации
    QDialog registerDialog(this);
    registerDialog.setWindowTitle("Регистрация нового пользователя");
    
    registerDialog.setFixedSize(450, 350);
    
    QVBoxLayout* layout = new QVBoxLayout(&registerDialog);
    
    // Создаем виджеты
    QLabel* loginLabel = new QLabel("Логин:", &registerDialog);
    QLineEdit* loginEdit = new QLineEdit(&registerDialog);
    loginEdit->setPlaceholderText("Введите логин (3-50 символов)");
    
    QLabel* passwordLabel = new QLabel("Пароль:", &registerDialog);
    QLineEdit* passwordEdit = new QLineEdit(&registerDialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Введите пароль");

    QLabel* confirmLabel = new QLabel("Подтвердите пароль:", &registerDialog);
    QLineEdit* confirmEdit = new QLineEdit(&registerDialog);
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setPlaceholderText("Повторите пароль");
    
    QLabel* passwordRequirements = new QLabel(
        "Требования к паролю:\n"
        "1. Минимум 8 символов\n"
        "2. Хотя бы одна заглавная буква\n"
        "3. Хотя бы одна строчная буква\n"
        "4. Хотя бы одна цифра\n"
        "5. Хотя бы один специальный символ",
        &registerDialog
    );
    passwordRequirements->setWordWrap(true);
    passwordRequirements->setStyleSheet("QLabel { "
        "color: #666666; "
        "font-size: 13px; "
        "background-color: #f8f8f8; "
        "padding: 8px; "
        "border: 1px solid #ddd; "
        "border-radius: 4px; "
        "margin-top: 5px; "
        "margin-bottom: 10px;"
    "}");
    
    QPushButton* okButton = new QPushButton("Создать", &registerDialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &registerDialog);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(loginLabel);
    layout->addWidget(loginEdit);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit);
    layout->addWidget(passwordRequirements);
    layout->addWidget(confirmLabel);
    layout->addWidget(confirmEdit);
    layout->addLayout(buttonLayout);
    
    layout->addStretch();
    
    connect(okButton, &QPushButton::clicked, &registerDialog, 
        [&registerDialog, loginEdit, passwordEdit, confirmEdit, passwordRequirements, this]() {
        
        qDebug() << "Нажата кнопка 'Создать' в диалоге регистрации";
        
        // Получаем данные из полей
        string login = loginEdit->text().trimmed().toStdString();
        string password = passwordEdit->text().toStdString();
        string confirm = confirmEdit->text().toStdString();
        
        qDebug() << "Регистрация. Логин:" << login.c_str();
        
        // 1. Проверка на пустые поля
        if (login.empty() || password.empty() || confirm.empty()) {
            QMessageBox::warning(&registerDialog, "Ошибка", "Заполните все поля");
            loginEdit->setFocus();
            return;
        }
        
        // 2. Проверка длины логина
        if (login.length() < 3 || login.length() > 50) {
            QMessageBox::warning(&registerDialog, "Ошибка", 
                "Логин должен содержать от 3 до 50 символов");
            loginEdit->setFocus();
            loginEdit->selectAll();
            return;
        }
        
        // 3. Проверка сложности пароля
        if (!HashUtils::isPasswordStrong(password)) {
            QMessageBox::warning(&registerDialog, "Ошибка",
                "Пароль не соответствует требованиям безопасности!");
            passwordEdit->setFocus();
            passwordEdit->selectAll();
            return;
        }
        
        // 4. Проверка совпадения паролей
        if (password != confirm) {
            QMessageBox::warning(&registerDialog, "Ошибка", "Пароли не совпадают");
            confirmEdit->setFocus();
            confirmEdit->selectAll();
            return;
        }
        
        // 5. Проверка существования пользователя
        if (dbManager->userExists(login)) {
            QMessageBox::warning(&registerDialog, "Ошибка", 
                "Пользователь с логином '" + QString::fromStdString(login) + "' уже существует");
            loginEdit->setFocus();
            loginEdit->selectAll();
            return;
        }
        
        // 6. Создание пользователя
        string salt = HashUtils::generateSalt();
        string passwordHash = HashUtils::hashPassword(password, salt);
        
        User newUser;
        newUser.login = login;
        newUser.passwordHash = passwordHash;
        newUser.salt = salt;
        
        // 7. Сохранение в БД
        if (dbManager->addUser(newUser)) {
            qDebug() << "Пользователь успешно создан:" << login.c_str();
            QMessageBox::information(&registerDialog, "Успех", 
                "Пользователь '" + QString::fromStdString(login) + "' успешно создан!\n\n"
                "Теперь вы можете войти в систему с этим логином и паролем.");
            registerDialog.accept();
        } else {
            qDebug() << "Ошибка при создании пользователя:" << login.c_str();
            QMessageBox::critical(&registerDialog, "Ошибка", 
                "Не удалось создать пользователя\n"
                "Возможно, проблемы с подключением к базе данных.");
        }
    });
    
    // Подключаем кнопку "Отмена"
    connect(cancelButton, &QPushButton::clicked, &registerDialog, &QDialog::reject);
    
// Подключаем проверку пароля в реальном времени
connect(passwordEdit, &QLineEdit::textChanged, passwordRequirements, 
    [passwordRequirements, passwordEdit]() {
    QString password = passwordEdit->text();
    bool isStrong = HashUtils::isPasswordStrong(password.toStdString());
    
    if (password.isEmpty()) {
        passwordRequirements->setText(
            "Требования к паролю:\n"
            "1. Минимум 8 символов\n"
            "2. Хотя бы одна заглавная буква\n"
            "3. Хотя бы одна строчная буква\n"
            "4. Хотя бы одна цифра\n"
            "5. Хотя бы один специальный символ"
        );
        passwordRequirements->setStyleSheet("QLabel { "
            "color: #666666; "
            "font-size: 13px; "
            "background-color: #f8f8f8; "
            "padding: 8px; "
            "border: 1px solid #ddd; "
            "border-radius: 4px; "
            "margin-top: 5px; "
            "margin-bottom: 10px;"
        "}");
    } else if (isStrong) {
        // Проверяем каждый критерий для детального отображения
        bool hasUpper = false;
        bool hasLower = false;
        bool hasDigit = false;
        bool hasSpecial = false;
        
        for (QChar c : password) {
            ushort unicode = c.unicode();
            
            // Русские заглавные
            if ((unicode >= 0x0410 && unicode <= 0x042F) || unicode == 0x0401) {
                hasUpper = true;
            }
            // Русские строчные
            else if ((unicode >= 0x0430 && unicode <= 0x044F) || unicode == 0x0451) {
                hasLower = true;
            }
            // Латинские заглавные
            else if (c.isUpper()) {
                hasUpper = true;
            }
            // Латинские строчные
            else if (c.isLower()) {
                hasLower = true;
            }
            // Цифры
            else if (c.isDigit()) {
                hasDigit = true;
            }
            // Специальные символы
            else if (!c.isSpace() && c.category() != QChar::Other_Control) {
                hasSpecial = true;
            }
        }
        
        QString details = "Пароль соответствует всем требованиям безопасности\n";
        details += QString("1. Длина: %1 символов\n").arg(password.length());
        details += QString("2. Заглавная буква: %1\n").arg(hasUpper ? "есть" : "нет");
        details += QString("3. Строчная буква: %1\n").arg(hasLower ? "есть" : "нет");
        details += QString("4. Цифра: %1\n").arg(hasDigit ? "есть" : "нет");
        details += QString("5. Специальный символ: %1").arg(hasSpecial ? "есть" : "нет");
        
        passwordRequirements->setText(details);
        passwordRequirements->setStyleSheet("QLabel { "
            "color: #008000; "
            "font-size: 13px; "
            "background-color: #f0fff0; "
            "padding: 8px; "
            "border: 2px solid #4CAF50; "
            "border-radius: 4px; "
            "margin-top: 5px; "
            "margin-bottom: 10px;"
        "}");
    } else {
        bool hasUpper = false;
        bool hasLower = false;
        bool hasDigit = false;
        bool hasSpecial = false;
        
        for (QChar c : password) {
            ushort unicode = c.unicode();
            
            if ((unicode >= 0x0410 && unicode <= 0x042F) || unicode == 0x0401) {
                hasUpper = true;
            }
            else if ((unicode >= 0x0430 && unicode <= 0x044F) || unicode == 0x0451) {
                hasLower = true;
            }
            else if (c.isUpper()) {
                hasUpper = true;
            }
            else if (c.isLower()) {
                hasLower = true;
            }
            else if (c.isDigit()) {
                hasDigit = true;
            }
            else if (!c.isSpace() && c.category() != QChar::Other_Control) {
                hasSpecial = true;
            }
        }
        
        QString requirementsText = "Пароль не соответствует требованиям:\n";
        
        requirementsText += QString("1. Минимум 8 символов: %1\n")
            .arg(password.length() >= 8 ? "есть" : QString("нет (%1/8)").arg(password.length()));
        
        requirementsText += QString("2. Заглавная буква: %1\n")
            .arg(hasUpper ? "есть" : "нет");
        
        requirementsText += QString("3. Строчная буква: %1\n")
            .arg(hasLower ? "есть" : "нет");
        
        requirementsText += QString("4. Цифра: %1\n")
            .arg(hasDigit ? "есть" : "нет");
        
        requirementsText += QString("5. Специальный символ: %1")
            .arg(hasSpecial ? "есть" : "нет");
        
        passwordRequirements->setText(requirementsText);
        passwordRequirements->setStyleSheet("QLabel { "
            "color: #FF0000; "
            "font-size: 13px; "
            "background-color: #fff0f0; "
            "padding: 8px; "
            "border: 2px solid #f44336; "
            "border-radius: 4px; "
            "margin-top: 5px; "
            "margin-bottom: 10px;"
        "}");
    }
});
    
    // Показываем диалог
    int result = registerDialog.exec();
    qDebug() << "Диалог регистрации закрыт с кодом:" << result;
}

bool AuthDialog::validateInput() {
    if (ui->txtLogin->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите логин");
        ui->txtLogin->setFocus();
        return false;
    }
    
    if (ui->txtPassword->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите пароль");
        ui->txtPassword->setFocus();
        return false;
    }
    
    return true;
}

string AuthDialog::getCurrentUserLogin() const {
    return currentUserLogin;
}

bool AuthDialog::isAdmin() const {
    return false;
}
