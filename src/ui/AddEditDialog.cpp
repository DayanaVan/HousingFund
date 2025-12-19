#include "AddEditDialog.h"
#include "../database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

using namespace std;

AddEditDialog::AddEditDialog(QWidget* parent)
    : QDialog(parent)
    , addressEdit(nullptr)
    , apartmentsSpin(nullptr)
    , areaSpin(nullptr)
    , yearSpin(nullptr)
    , floorsSpin(nullptr)
    , dbManager(nullptr)
    , currentHouseId(0)
    , isEditingMode(false)
{
    setupUI();
    setWindowTitle("Добавить дом");
    setModal(true);
    setFixedSize(400, 300);
}

AddEditDialog::~AddEditDialog() {}

void AddEditDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Адрес
    QLabel* addressLabel = new QLabel("Адрес дома:", this);
    addressEdit = new QLineEdit(this);
    addressEdit->setPlaceholderText("ул. Примерная, д. 10");
    addressEdit->setMaxLength(200);
    
    // Количество квартир
    QLabel* apartmentsLabel = new QLabel("Количество квартир:", this);
    apartmentsSpin = new QSpinBox(this);
    apartmentsSpin->setRange(1, 1000);
    apartmentsSpin->setValue(24);
    
    // Площадь
    QLabel* areaLabel = new QLabel("Общая площадь (кв.м):", this);
    areaSpin = new QDoubleSpinBox(this);
    areaSpin->setRange(1.0, 100000.0);
    areaSpin->setDecimals(2);
    areaSpin->setValue(1000.0);
    
    // Год постройки
    QLabel* yearLabel = new QLabel("Год постройки:", this);
    yearSpin = new QSpinBox(this);
    int currentYear = QDate::currentDate().year();
    yearSpin->setRange(1500, currentYear);
    yearSpin->setValue(2000);
    
    // Этажность
    QLabel* floorsLabel = new QLabel("Этажность:", this);
    floorsSpin = new QSpinBox(this);
    floorsSpin->setRange(1, 100);
    floorsSpin->setValue(5);
    
    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* saveButton = new QPushButton("Сохранить", this);
    QPushButton* cancelButton = new QPushButton("Отмена", this);
    
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    // Собираем layout
    mainLayout->addWidget(addressLabel);
    mainLayout->addWidget(addressEdit);
    mainLayout->addWidget(apartmentsLabel);
    mainLayout->addWidget(apartmentsSpin);
    mainLayout->addWidget(areaLabel);
    mainLayout->addWidget(areaSpin);
    mainLayout->addWidget(yearLabel);
    mainLayout->addWidget(yearSpin);
    mainLayout->addWidget(floorsLabel);
    mainLayout->addWidget(floorsSpin);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
    
    // Подключаем сигналы
    connect(saveButton, &QPushButton::clicked, this, &AddEditDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &AddEditDialog::onCancelClicked);
    
    // Валидация при изменении
    connect(addressEdit, &QLineEdit::textChanged, this, &AddEditDialog::validateInput);
    connect(apartmentsSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AddEditDialog::validateInput);
}

void AddEditDialog::onSaveClicked() {
    if (!validateData()) {
        return;
    }
    
    // Проверка на дубликаты
    if (!checkForDuplicates()) {
        return;
    }
    
    // Заполняем структуру House
    currentHouse.address = addressEdit->text().trimmed().toStdString();
    currentHouse.apartments = apartmentsSpin->value();
    currentHouse.totalArea = areaSpin->value();
    currentHouse.buildYear = yearSpin->value();
    currentHouse.floors = floorsSpin->value();
    currentHouse.id = currentHouseId;
    
    accept();
}

void AddEditDialog::onCancelClicked() {
    reject();
}

void AddEditDialog::validateInput() {
    bool valid = !addressEdit->text().trimmed().isEmpty() &&
                 apartmentsSpin->value() > 0 &&
                 areaSpin->value() > 0 &&
                 yearSpin->value() >= 1500 &&
                 yearSpin->value() <= QDate::currentDate().year() &&
                 floorsSpin->value() > 0 &&
                 floorsSpin->value() <= 100;
    
    QPushButton* saveButton = findChild<QPushButton*>("Сохранить");
    if (saveButton) {
        saveButton->setEnabled(valid);
    }
}

bool AddEditDialog::validateData() {
    if (addressEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите адрес дома");
        addressEdit->setFocus();
        return false;
    }
    
    if (apartmentsSpin->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Количество квартир должно быть больше 0");
        apartmentsSpin->setFocus();
        return false;
    }
    
    if (areaSpin->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Площадь должна быть больше 0");
        areaSpin->setFocus();
        return false;
    }
    
    int currentYear = QDate::currentDate().year();
    if (yearSpin->value() < 1500 || yearSpin->value() > currentYear) {
        QMessageBox::warning(this, "Ошибка", 
            QString("Год постройки должен быть от 1500 до %1").arg(currentYear));
        yearSpin->setFocus();
        return false;
    }
    
    if (floorsSpin->value() < 1 || floorsSpin->value() > 100) {
        QMessageBox::warning(this, "Ошибка", "Этажность должна быть от 1 до 100");
        floorsSpin->setFocus();
        return false;
    }
    
    return true;
}

bool AddEditDialog::checkForDuplicates() const {
    if (!dbManager) {
        QMessageBox::warning(const_cast<AddEditDialog*>(this), "Ошибка", 
                            "Не подключен менеджер базы данных");
        return false;
    }
    
    QString address = addressEdit->text().trimmed();
    if (address.isEmpty()) {
        return true;
    }
    
    // Создаем временный объект House для проверки
    House tempHouse;
    tempHouse.address = address.toStdString();
    tempHouse.id = currentHouseId;
    tempHouse.apartments = apartmentsSpin->value();
    tempHouse.totalArea = areaSpin->value();
    tempHouse.buildYear = yearSpin->value();
    tempHouse.floors = floorsSpin->value();
    
    // 1. Проверка точного совпадения адреса
    bool exactDuplicateExists;
    if (isEditingMode) {
        exactDuplicateExists = dbManager->houseExistsWithDifferentId(tempHouse);
    } else {
        exactDuplicateExists = dbManager->houseExists(tempHouse);
    }
    
    if (exactDuplicateExists) {
        QMessageBox::warning(const_cast<AddEditDialog*>(this), "Ошибка", 
                            QString("Дом с адресом '%1' уже существует в базе данных!")
                            .arg(address));
        addressEdit->setFocus();
        addressEdit->selectAll();
        return false;
    }
    
    // 2. Проверка похожих домов (расширенная проверка)
    vector<House> similarHouses;
    if (dbManager->findSimilarHouses(tempHouse, similarHouses, 0.7)) {
        if (!similarHouses.empty()) {
            QString message = QString("Найдены похожие дома:\n\n");
            
            for (const auto& similar : similarHouses) {
                message += QString("Адрес: %1\nКвартир: %2, Площадь: %3 м², Год: %4, Этажей: %5\n\n")
                    .arg(QString::fromStdString(similar.address))
                    .arg(similar.apartments)
                    .arg(similar.totalArea, 0, 'f', 2)
                    .arg(similar.buildYear)
                    .arg(similar.floors);
            }
            
            message += "Вы уверены, что хотите продолжить?";
            
            QMessageBox msgBox(const_cast<AddEditDialog*>(this));
            msgBox.setWindowTitle("Внимание: найдены похожие дома");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            
            if (msgBox.exec() != QMessageBox::Yes) {
                return false;
            }
        }
    }
    
    return true;
}

House AddEditDialog::getHouse() const {
    return currentHouse;
}

void AddEditDialog::setHouse(const House& house) {
    currentHouse = house;
    
    if (addressEdit) addressEdit->setText(QString::fromStdString(house.address));
    if (apartmentsSpin) apartmentsSpin->setValue(house.apartments);
    if (areaSpin) areaSpin->setValue(house.totalArea);
    if (yearSpin) yearSpin->setValue(house.buildYear);
    if (floorsSpin) floorsSpin->setValue(house.floors);
    
    currentHouseId = house.id;
}

void AddEditDialog::setDatabaseManager(DatabaseManager* manager) {
    dbManager = manager;
}

void AddEditDialog::setEditingMode(bool editing, int houseId) {
    isEditingMode = editing;
    currentHouseId = houseId;
    if (editing) {
        setWindowTitle("Редактировать дом");
    } else {
        setWindowTitle("Добавить дом");
    }
}
