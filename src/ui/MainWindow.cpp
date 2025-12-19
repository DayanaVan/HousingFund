#include "MainWindow.h"
#include <QIcon>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QTabWidget>
#include "./ui_MainWindow.h"
#include "../models/House.h"
#include "AddEditDialog.h"
#include "ExportDialog.h"
#include "AuthDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QHeaderView>
#include <QDebug>
#include <QDateTime>
#include <algorithm>
#include <QInputDialog>
#include <QSet>

using namespace std;

MainWindow::MainWindow(DatabaseManager* dbManager, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dbManager(dbManager)
    , sortColumns()
{
    ui->setupUi(this);
    setWindowTitle("Управление жилым фондом");
    setWindowIcon(QIcon::fromTheme("building"));
    
    setupUI();
    setupTable();
    loadHouses();
    updateStatusBar();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    // Создаем центральный виджет
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Основная панель инструментов
    QToolBar* toolbar = new QToolBar("Основные действия", this);
    toolbar->addAction(QIcon::fromTheme("list-add"), "Добавить дом", this, &MainWindow::onAddHouse);
    toolbar->addAction(QIcon::fromTheme("document-edit"), "Редактировать", this, &MainWindow::onEditHouse);
    toolbar->addAction(QIcon::fromTheme("edit-delete"), "Удалить", this, &MainWindow::onDeleteHouses);
    toolbar->addSeparator();
    toolbar->addAction(QIcon::fromTheme("view-filter"), "Фильтры", this, &MainWindow::onFilter);
    toolbar->addAction(QIcon::fromTheme("edit-clear"), "Очистить фильтры", this, &MainWindow::onClearFilters);
    toolbar->addAction(QIcon::fromTheme("document-export"), "Экспорт", this, &MainWindow::onExport);
    toolbar->addAction(QIcon::fromTheme("view-refresh"), "Обновить", this, &MainWindow::onRefresh);
    toolbar->addSeparator();
    toolbar->addAction(QIcon::fromTheme("system-log-out"), "Выход", this, &MainWindow::onExit);
    
    addToolBar(toolbar);
    
    // Панель сортировки
    QWidget* sortPanel = new QWidget();
    QHBoxLayout* sortLayout = new QHBoxLayout(sortPanel);
    
    QLabel* sortLabel = new QLabel("Сортировка:", sortPanel);
    QComboBox* sortCombo1 = new QComboBox(sortPanel);
    sortCombo1->addItem("Без сортировки", -1);
    sortCombo1->addItem("Адрес", 1);
    sortCombo1->addItem("Квартиры", 2);
    sortCombo1->addItem("Площадь", 3);
    sortCombo1->addItem("Год постройки", 4);
    sortCombo1->addItem("Этажность", 5);
    
    QComboBox* orderCombo1 = new QComboBox(sortPanel);
    orderCombo1->addItem("По возрастанию", true);
    orderCombo1->addItem("По убыванию", false);
    
    QComboBox* sortCombo2 = new QComboBox(sortPanel);
    sortCombo2->addItem("Без сортировки", -1);
    sortCombo2->addItem("Адрес", 1);
    sortCombo2->addItem("Квартиры", 2);
    sortCombo2->addItem("Площадь", 3);
    sortCombo2->addItem("Год постройки", 4);
    sortCombo2->addItem("Этажность", 5);
    
    QComboBox* orderCombo2 = new QComboBox(sortPanel);
    orderCombo2->addItem("По возрастанию", true);
    orderCombo2->addItem("По убыванию", false);
    
    QPushButton* applySortBtn = new QPushButton("Применить", sortPanel);
    
    sortLayout->addWidget(sortLabel);
    sortLayout->addWidget(sortCombo1);
    sortLayout->addWidget(orderCombo1);
    sortLayout->addWidget(new QLabel("затем", sortPanel));
    sortLayout->addWidget(sortCombo2);
    sortLayout->addWidget(orderCombo2);
    sortLayout->addWidget(applySortBtn);
    sortLayout->addStretch();
    
    // Панель поиска
    QWidget* searchPanel = new QWidget();
    QHBoxLayout* searchLayout = new QHBoxLayout(searchPanel);
    
    QLabel* searchLabel = new QLabel("Поиск по адресу:", searchPanel);
    QLineEdit* searchEdit = new QLineEdit(searchPanel);
    searchEdit->setPlaceholderText("Введите часть адреса...");
    QPushButton* searchBtn = new QPushButton("Найти", searchPanel);
    QPushButton* clearSearchBtn = new QPushButton("Очистить", searchPanel);
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchBtn);
    searchLayout->addWidget(clearSearchBtn);
    searchLayout->addStretch();
    
    // Таблица
    QTableWidget* table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID", "Адрес", "Квартир", "Площадь", "Год постройки", "Этажей"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Собираем layout
    mainLayout->addWidget(sortPanel);
    mainLayout->addWidget(searchPanel);
    mainLayout->addWidget(table);
    
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    
    // Статус бар
    statusBar()->showMessage("Готово");
    
    // Подключаем сигналы
    connect(searchBtn, &QPushButton::clicked, this, [this, searchEdit]() {
        currentFilters.addressFilter = searchEdit->text();
        loadHouses();
        statusBar()->showMessage(QString("Применен фильтр по адресу: %1").arg(searchEdit->text()));
    });
    
    connect(clearSearchBtn, &QPushButton::clicked, this, [this, searchEdit]() {
        searchEdit->clear();
        currentFilters.addressFilter.clear();
        loadHouses();
        statusBar()->showMessage("Поиск очищен");
    });
    
    connect(applySortBtn, &QPushButton::clicked, this, [this, sortCombo1, orderCombo1, sortCombo2, orderCombo2]() {
        sortColumns.clear();
        
        // Первый уровень сортировки
        int col1 = sortCombo1->currentData().toInt();
        if (col1 != -1) {
            SortColumn sc;
            sc.column = col1;
            sc.ascending = orderCombo1->currentData().toBool();
            sortColumns.append(sc);
        }
        
        // Второй уровень сортировки
        int col2 = sortCombo2->currentData().toInt();
        if (col2 != -1) {
            SortColumn sc;
            sc.column = col2;
            sc.ascending = orderCombo2->currentData().toBool();
            sortColumns.append(sc);
        }
        
        loadHouses();
        statusBar()->showMessage("Сортировка применена");
    });
    
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        onEditHouse();
    });
}

void MainWindow::setupTable() {
    QTableWidget* table = findChild<QTableWidget*>();
    if (!table) return;
    
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void MainWindow::loadHouses() {
    auto houses = dbManager->getAllHouses();
    
    // Применяем фильтры
    if (currentFilters.isActive()) {
        applyFilters(houses);
    }
    
    // Применяем сортировку
    if (!sortColumns.isEmpty()) {
        applySorting(houses);
    }
    
    loadHouses(houses);
}

void MainWindow::loadHouses(const vector<House>& houses) {
    QTableWidget* table = findChild<QTableWidget*>();
    if (!table) return;
    
    table->setRowCount(0);
    table->setRowCount(houses.size());
    
    for (size_t i = 0; i < houses.size(); ++i) {
        const House& house = houses[i];
        
        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(house.id));
        idItem->setData(Qt::UserRole, house.id);
        table->setItem(i, 0, idItem);
        
        // Адрес
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(house.address)));
        
        // Квартиры
        table->setItem(i, 2, new QTableWidgetItem(QString::number(house.apartments)));
        
        // Площадь
        table->setItem(i, 3, new QTableWidgetItem(QString::number(house.totalArea, 'f', 2)));
        
        // Год постройки
        QTableWidgetItem* yearItem = new QTableWidgetItem(QString::number(house.buildYear));
        table->setItem(i, 4, yearItem);
        
        // Этажи
        table->setItem(i, 5, new QTableWidgetItem(QString::number(house.floors)));
    }
    
    updateStatusBar();
}

void MainWindow::applyFilters(vector<House>& houses) {
    houses.erase(remove_if(houses.begin(), houses.end(),
        [this](const House& house) {
            return !matchesFilters(house);
        }),
        houses.end());
}

bool MainWindow::matchesFilters(const House& house) const {
    // Проверка года постройки
    if (house.buildYear < currentFilters.minYear || house.buildYear > currentFilters.maxYear) {
        return false;
    }
    
    // Проверка количества квартир
    if (house.apartments < currentFilters.minApartments || house.apartments > currentFilters.maxApartments) {
        return false;
    }
    
    // Проверка площади
    if (house.totalArea < currentFilters.minArea || house.totalArea > currentFilters.maxArea) {
        return false;
    }
    
    // Проверка адреса
    if (!currentFilters.addressFilter.isEmpty()) {
        QString address = QString::fromStdString(house.address);
        if (!address.contains(currentFilters.addressFilter, Qt::CaseInsensitive)) {
            return false;
        }
    }
    
    // Проверка этажности
    if (house.floors < currentFilters.minFloors || house.floors > currentFilters.maxFloors) {
        return false;
    }
    
    return true;
}

void MainWindow::applySorting(vector<House>& houses) {
    sort(houses.begin(), houses.end(),
        [this](const House& a, const House& b) {
            return compareHouses(a, b);
        });
}

bool MainWindow::compareHouses(const House& a, const House& b) const {
    for (const SortColumn& sc : sortColumns) {
        int comparison = 0;
        
        switch (sc.column) {
            case 1: // Адрес
                comparison = QString::fromStdString(a.address).compare(QString::fromStdString(b.address));
                break;
            case 2: // Квартиры
                comparison = a.apartments - b.apartments;
                break;
            case 3: // Площадь
                comparison = (a.totalArea < b.totalArea) ? -1 : (a.totalArea > b.totalArea) ? 1 : 0;
                break;
            case 4: // Год постройки
                comparison = a.buildYear - b.buildYear;
                break;
            case 5: // Этажность
                comparison = a.floors - b.floors;
                break;
            default:
                continue;
        }
        
        if (comparison != 0) {
            return sc.ascending ? (comparison < 0) : (comparison > 0);
        }
    }
    
    return false; // Все поля равны
}

// === ОСНОВНЫЕ СЛОТЫ ===

void MainWindow::onAddHouse() {
    AddEditDialog dialog(this);
    dialog.setWindowTitle("Добавить новый дом");
    dialog.setDatabaseManager(dbManager);
    dialog.setEditingMode(false);
    
    if (dialog.exec() == QDialog::Accepted) {
        House house = dialog.getHouse();
        if (dbManager->addHouse(house)) {
            showInfo("Дом успешно добавлен");
            loadHouses();
        } else {
            showError("Ошибка при добавлении дома");
        }
    }
}

void MainWindow::onEditHouse() {
    QTableWidget* table = findChild<QTableWidget*>();
    if (!table || table->currentRow() < 0) {
        showError("Выберите дом для редактирования");
        return;
    }
    
    int houseId = table->item(table->currentRow(), 0)->data(Qt::UserRole).toInt();
    
    // Получаем все дома и находим нужный
    auto houses = dbManager->getAllHouses();
    House* selectedHouse = nullptr;
    
    for (auto& house : houses) {
        if (house.id == houseId) {
            selectedHouse = &house;
            break;
        }
    }
    
    if (!selectedHouse) {
        showError("Дом не найден");
        return;
    }
    
    AddEditDialog dialog(this);
    dialog.setWindowTitle("Редактировать дом");
    dialog.setDatabaseManager(dbManager);
    dialog.setEditingMode(true, houseId);
    dialog.setHouse(*selectedHouse);
    
    if (dialog.exec() == QDialog::Accepted) {
        House updatedHouse = dialog.getHouse();
        updatedHouse.id = houseId;
        
        if (dbManager->updateHouse(updatedHouse)) {
            showInfo("Изменения сохранены");
            loadHouses();
        } else {
            showError("Ошибка при сохранении изменений");
        }
    }
}

void MainWindow::onDeleteHouses() {
    QDialog deleteDialog(this);
    deleteDialog.setWindowTitle("Удаление домов");
    deleteDialog.setFixedSize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(&deleteDialog);
    
    QLabel* titleLabel = new QLabel("Выберите критерий удаления:", &deleteDialog);
    layout->addWidget(titleLabel);
    
    QComboBox* deleteCombo = new QComboBox(&deleteDialog);
    deleteCombo->addItem("Удалить выбранные дома");
    deleteCombo->addItem("Удалить по году постройки");
    deleteCombo->addItem("Удалить по количеству квартир");
    deleteCombo->addItem("Удалить по площади");
    deleteCombo->addItem("Удалить по адресу (улице)");
    layout->addWidget(deleteCombo);
    
    QWidget* paramWidget = new QWidget(&deleteDialog);
    QVBoxLayout* paramLayout = new QVBoxLayout(paramWidget);
    
    QLineEdit* yearEdit = new QLineEdit(paramWidget);
    yearEdit->setPlaceholderText("Год постройки");
    yearEdit->setVisible(false);
    
    QLineEdit* apartmentsEdit = new QLineEdit(paramWidget);
    apartmentsEdit->setPlaceholderText("Количество квартир (можно диапазон: 10-50)");
    apartmentsEdit->setVisible(false);
    
    QLineEdit* areaEdit = new QLineEdit(paramWidget);
    areaEdit->setPlaceholderText("Площадь (можно диапазон: 500-1000)");
    areaEdit->setVisible(false);
    
    QLineEdit* addressEdit = new QLineEdit(paramWidget);
    addressEdit->setPlaceholderText("Часть адреса или улицы");
    addressEdit->setVisible(false);
    
    paramLayout->addWidget(yearEdit);
    paramLayout->addWidget(apartmentsEdit);
    paramLayout->addWidget(areaEdit);
    paramLayout->addWidget(addressEdit);
    paramWidget->setLayout(paramLayout);
    layout->addWidget(paramWidget);
    
    // Показываем соответствующий параметр
    connect(deleteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index) {
            yearEdit->setVisible(index == 1);
            apartmentsEdit->setVisible(index == 2);
            areaEdit->setVisible(index == 3);
            addressEdit->setVisible(index == 4);
        });
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, 
                                                      Qt::Horizontal, &deleteDialog);
    layout->addWidget(buttonBox);
    
    deleteDialog.setLayout(layout);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &deleteDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &deleteDialog, &QDialog::reject);
    
    if (deleteDialog.exec() == QDialog::Accepted) {
        int option = deleteCombo->currentIndex();
        
        switch (option) {
            case 0: { // Удалить выбранные дома
                QTableWidget* table = findChild<QTableWidget*>();
                if (!table) return;
                
                QList<QTableWidgetItem*> selectedItems = table->selectedItems();
                QSet<int> selectedRows;
                
                for (const auto& item : selectedItems) {
                    selectedRows.insert(item->row());
                }
                
                if (selectedRows.isEmpty()) {
                    showError("Выберите дома для удаления");
                    return;
                }
                
                if (!confirmAction("Удаление", 
                    QString("Удалить выбранные %1 домов?\nЭто действие нельзя отменить.").arg(selectedRows.size()))) {
                    return;
                }
                
                bool allDeleted = true;
                for (int row : selectedRows) {
                    int houseId = table->item(row, 0)->data(Qt::UserRole).toInt();
                    if (!dbManager->deleteHouse(houseId)) {
                        allDeleted = false;
                    }
                }
                
                if (allDeleted) {
                    showInfo(QString("Удалено %1 домов").arg(selectedRows.size()));
                    loadHouses();
                } else {
                    showError("Ошибка при удалении некоторых домов");
                }
                break;
            }
            
            case 1: { // Удалить по году
                QString yearText = yearEdit->text();
                if (yearText.isEmpty()) {
                    showError("Введите год постройки");
                    return;
                }
                
                bool ok;
                int year = yearText.toInt(&ok);
                if (!ok || year < 1500 || year > QDate::currentDate().year()) {
                    showError("Некорректный год");
                    return;
                }
                
                // Проверяем сколько домов будет удалено
                auto allHouses = dbManager->getAllHouses();
                int countToDelete = 0;
                for (const auto& house : allHouses) {
                    if (house.buildYear == year) {
                        countToDelete++;
                    }
                }
                
                if (countToDelete == 0) {
                    showInfo(QString("Домов %1 года не найдено").arg(year));
                    return;
                }
                
                if (!confirmAction("Удаление", 
                    QString("Удалить %1 домов постройки %2 года?\nЭто действие нельзя отменить.").arg(countToDelete).arg(year))) {
                    return;
                }
                
                if (dbManager->deleteHousesByYear(year)) {
                    showInfo(QString("Дома %1 года удалены (%2 шт.)").arg(year).arg(countToDelete));
                    loadHouses();
                } else {
                    showError("Ошибка при удалении");
                }
                break;
            }
            
            case 2: { // Удалить по количеству квартир
                QString aptText = apartmentsEdit->text();
                if (aptText.isEmpty()) {
                    showError("Введите количество квартир");
                    return;
                }
                
                int minApt = 0, maxApt = 0;
                if (aptText.contains("-")) {
                    QStringList parts = aptText.split("-");
                    if (parts.size() == 2) {
                        minApt = parts[0].toInt();
                        maxApt = parts[1].toInt();
                    }
                } else {
                    minApt = maxApt = aptText.toInt();
                }
                
                if (minApt <= 0 || maxApt <= 0 || minApt > maxApt) {
                    showError("Некорректный диапазон квартир");
                    return;
                }
                
                auto allHouses = dbManager->getAllHouses();
                int countToDelete = 0;
                for (const auto& house : allHouses) {
                    if (house.apartments >= minApt && house.apartments <= maxApt) {
                        countToDelete++;
                    }
                }
                
                if (countToDelete == 0) {
                    showInfo(QString("Домов с количеством квартир %1-%2 не найдено").arg(minApt).arg(maxApt));
                    return;
                }
                
                if (!confirmAction("Удаление", 
                    QString("Удалить %1 домов с количеством квартир %2-%3?\nЭто действие нельзя отменить.").arg(countToDelete).arg(minApt).arg(maxApt))) {
                    return;
                }
                
                if (dbManager->deleteHousesByApartments(minApt, maxApt)) {
                    showInfo(QString("Дома удалены (%1 шт.)").arg(countToDelete));
                    loadHouses();
                } else {
                    showError("Ошибка при удалении");
                }
                break;
            }
            
            case 3: { // Удалить по площади
                QString areaText = areaEdit->text();
                if (areaText.isEmpty()) {
                    showError("Введите площадь");
                    return;
                }
                
                double minArea = 0, maxArea = 0;
                if (areaText.contains("-")) {
                    QStringList parts = areaText.split("-");
                    if (parts.size() == 2) {
                        minArea = parts[0].toDouble();
                        maxArea = parts[1].toDouble();
                    }
                } else {
                    minArea = maxArea = areaText.toDouble();
                }
                
                if (minArea <= 0 || maxArea <= 0 || minArea > maxArea) {
                    showError("Некорректный диапазон площади");
                    return;
                }
                
                auto allHouses = dbManager->getAllHouses();
                int countToDelete = 0;
                for (const auto& house : allHouses) {
                    if (house.totalArea >= minArea && house.totalArea <= maxArea) {
                        countToDelete++;
                    }
                }
                
                if (countToDelete == 0) {
                    showInfo(QString("Домов с площадью %1-%2 м² не найдено").arg(minArea, 0, 'f', 2).arg(maxArea, 0, 'f', 2));
                    return;
                }
                
                if (!confirmAction("Удаление", 
                    QString("Удалить %1 домов с площадью %2-%3 м²?\nЭто действие нельзя отменить.").arg(countToDelete).arg(minArea, 0, 'f', 2).arg(maxArea, 0, 'f', 2))) {
                    return;
                }
                
                if (dbManager->deleteHousesByArea(minArea, maxArea)) {
                    showInfo(QString("Дома удалены (%1 шт.)").arg(countToDelete));
                    loadHouses();
                } else {
                    showError("Ошибка при удалении");
                }
                break;
            }
            
            case 4: { // Удалить по адресу
                QString addressText = addressEdit->text();
                if (addressText.isEmpty()) {
                    showError("Введите часть адреса");
                    return;
                }
                
                auto allHouses = dbManager->getAllHouses();
                int countToDelete = 0;
                for (const auto& house : allHouses) {
                    if (QString::fromStdString(house.address).contains(addressText, Qt::CaseInsensitive)) {
                        countToDelete++;
                    }
                }
                
                if (countToDelete == 0) {
                    showInfo(QString("Домов с адресом содержащим '%1' не найдено").arg(addressText));
                    return;
                }
                
                if (!confirmAction("Удаление", 
                    QString("Удалить %1 домов с адресом содержащим '%2'?\nЭто действие нельзя отменить.").arg(countToDelete).arg(addressText))) {
                    return;
                }
                
                if (dbManager->deleteHousesByAddress(addressText.toStdString())) {
                    showInfo(QString("Дома удалены (%1 шт.)").arg(countToDelete));
                    loadHouses();
                } else {
                    showError("Ошибка при удалении");
                }
                break;
            }
        }
    }
}

void MainWindow::onFilter() {
    QDialog filterDialog(this);
    filterDialog.setWindowTitle("Фильтры");
    filterDialog.setFixedSize(500, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&filterDialog);
    
    QTabWidget* tabWidget = new QTabWidget(&filterDialog);
    
    // Вкладка "Год постройки"
    QWidget* yearTab = new QWidget();
    QFormLayout* yearLayout = new QFormLayout(yearTab);
    
    QSpinBox* minYearSpin = new QSpinBox(yearTab);
    minYearSpin->setRange(1500, QDate::currentDate().year());
    minYearSpin->setValue(currentFilters.minYear);
    
    QSpinBox* maxYearSpin = new QSpinBox(yearTab);
    maxYearSpin->setRange(1500, QDate::currentDate().year());
    maxYearSpin->setValue(currentFilters.maxYear);
    
    yearLayout->addRow("Год от:", minYearSpin);
    yearLayout->addRow("Год до:", maxYearSpin);
    yearTab->setLayout(yearLayout);
    tabWidget->addTab(yearTab, "Год постройки");
    
    // Вкладка "Квартиры"
    QWidget* aptTab = new QWidget();
    QFormLayout* aptLayout = new QFormLayout(aptTab);
    
    QSpinBox* minAptSpin = new QSpinBox(aptTab);
    minAptSpin->setRange(0, 1000);
    minAptSpin->setValue(currentFilters.minApartments);
    
    QSpinBox* maxAptSpin = new QSpinBox(aptTab);
    maxAptSpin->setRange(0, 1000);
    maxAptSpin->setValue(currentFilters.maxApartments);
    
    aptLayout->addRow("Квартир от:", minAptSpin);
    aptLayout->addRow("Квартир до:", maxAptSpin);
    aptTab->setLayout(aptLayout);
    tabWidget->addTab(aptTab, "Квартиры");
    
    // Вкладка "Площадь"
    QWidget* areaTab = new QWidget();
    QFormLayout* areaLayout = new QFormLayout(areaTab);
    
    QDoubleSpinBox* minAreaSpin = new QDoubleSpinBox(areaTab);
    minAreaSpin->setRange(0.0, 100000.0);
    minAreaSpin->setValue(currentFilters.minArea);
    minAreaSpin->setDecimals(2);
    
    QDoubleSpinBox* maxAreaSpin = new QDoubleSpinBox(areaTab);
    maxAreaSpin->setRange(0.0, 100000.0);
    maxAreaSpin->setValue(currentFilters.maxArea);
    maxAreaSpin->setDecimals(2);
    
    areaLayout->addRow("Площадь от:", minAreaSpin);
    areaLayout->addRow("Площадь до:", maxAreaSpin);
    areaTab->setLayout(areaLayout);
    tabWidget->addTab(areaTab, "Площадь");
    
    // Вкладка "Этажность"
    QWidget* floorsTab = new QWidget();
    QFormLayout* floorsLayout = new QFormLayout(floorsTab);
    
    QSpinBox* minFloorsSpin = new QSpinBox(floorsTab);
    minFloorsSpin->setRange(0, 100);
    minFloorsSpin->setValue(currentFilters.minFloors);
    
    QSpinBox* maxFloorsSpin = new QSpinBox(floorsTab);
    maxFloorsSpin->setRange(0, 100);
    maxFloorsSpin->setValue(currentFilters.maxFloors);
    
    floorsLayout->addRow("Этажей от:", minFloorsSpin);
    floorsLayout->addRow("Этажей до:", maxFloorsSpin);
    floorsTab->setLayout(floorsLayout);
    tabWidget->addTab(floorsTab, "Этажность");
    
    mainLayout->addWidget(tabWidget);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | 
                                                      QDialogButtonBox::Reset, Qt::Horizontal, &filterDialog);
    mainLayout->addWidget(buttonBox);
    
    filterDialog.setLayout(mainLayout);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &filterDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &filterDialog, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
        [=]() {
            minYearSpin->setValue(1500);
            maxYearSpin->setValue(QDate::currentDate().year());
            minAptSpin->setValue(0);
            maxAptSpin->setValue(1000);
            minAreaSpin->setValue(0.0);
            maxAreaSpin->setValue(100000.0);
            minFloorsSpin->setValue(0);
            maxFloorsSpin->setValue(100);
        });
    
    if (filterDialog.exec() == QDialog::Accepted) {
        currentFilters.minYear = minYearSpin->value();
        currentFilters.maxYear = maxYearSpin->value();
        currentFilters.minApartments = minAptSpin->value();
        currentFilters.maxApartments = maxAptSpin->value();
        currentFilters.minArea = minAreaSpin->value();
        currentFilters.maxArea = maxAreaSpin->value();
        currentFilters.minFloors = minFloorsSpin->value();
        currentFilters.maxFloors = maxFloorsSpin->value();
        
        loadHouses();
        statusBar()->showMessage("Фильтры применены");
    }
}

void MainWindow::onClearFilters() {
    currentFilters = FilterSettings();
    loadHouses();
    statusBar()->showMessage("Все фильтры очищены");
}

void MainWindow::onRefresh() {
    loadHouses();
    statusBar()->showMessage("Данные обновлены");
}

void MainWindow::onExport() {
    ExportDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString fileName = dialog.getFilePath();
        QStringList fields = dialog.getSelectedFields();
        QString delimiter = dialog.getDelimiter();
        bool includeHeader = dialog.includeHeader();
        
        // Конвертируем QStringList в vector<string>
        vector<string> stdFields;
        for (const QString& field : fields) {
            stdFields.push_back(field.toStdString());
        }
        
        if (dbManager->exportToFile(fileName.toStdString(), stdFields, 
                                   delimiter.toStdString(), includeHeader)) {
            showInfo(QString("Данные экспортированы в текстовый файл:\n%1\n"
                           "Формат: TXT\n"
                           "Разделитель: %2")
                    .arg(fileName)
                    .arg(delimiter == "\t" ? "Табуляция" : 
                         delimiter == ";" ? "Точка с запятой" : "Запятая"));
        } else {
            showError("Ошибка при экспорте данных в TXT файл");
        }
    }
}

void MainWindow::onExit() {
    if (confirmAction("Выход", "Вы уверены, что хотите выйти из программы?")) {
        qApp->quit();
    }
}

void MainWindow::updateStatusBar() {
    QTableWidget* table = findChild<QTableWidget*>();
    if (table) {
        int count = table->rowCount();
        QString filterInfo = currentFilters.isActive() ? " (с фильтрами)" : "";
        statusBar()->showMessage(QString("Всего домов: %1%2 | %3")
            .arg(count)
            .arg(filterInfo)
            .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm")));
    }
}

bool MainWindow::confirmAction(const QString& title, const QString& message) {
    return QMessageBox::question(this, title, message, 
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::showError(const QString& message) {
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::showInfo(const QString& message) {
    QMessageBox::information(this, "Информация", message);
}

void MainWindow::onSearch()
{
    // Реализовано в setupUI
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе", 
        "Housing Fund Management System\n"
        "Версия 2.1\n\n"
        "Новые возможности:\n"
        "• Расширенные фильтры по всем параметрам\n"
        "• Многоуровневая сортировка\n"
        "• Улучшенное удаление с выбором критериев\n"
        "• Проверка на дубликаты домов\n"
        "• Расширенный экспорт данных");
}

void MainWindow::onHouseSelected(int row, int column) {
    QTableWidget* table = findChild<QTableWidget*>();
    if (!table || row < 0) return;
    
    // Получаем ID дома
    QTableWidgetItem* item = table->item(row, 0);
    if (!item) return;
    
    int houseId = item->data(Qt::UserRole).toInt();
    
    // Получаем информацию о доме
    auto houses = dbManager->getAllHouses();
    for (const auto& house : houses) {
        if (house.id == houseId) {
            // Выводим информацию в статус бар
            QString info = QString("Выбран дом: %1 | Квартир: %2 | Площадь: %3 м² | Год: %4 | Этажей: %5")
                .arg(QString::fromStdString(house.address))
                .arg(house.apartments)
                .arg(house.totalArea, 0, 'f', 2)
                .arg(house.buildYear)
                .arg(house.floors);
            statusBar()->showMessage(info, 5000);
            break;
        }
    }
}
