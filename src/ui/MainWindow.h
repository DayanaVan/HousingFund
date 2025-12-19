#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDate>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QInputDialog>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QSortFilterProxyModel>
#include "../database/DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Структура для хранения фильтров
struct FilterSettings {
    int minYear = 1500;
    int maxYear = QDate::currentDate().year();
    int minApartments = 0;
    int maxApartments = 1000;
    double minArea = 0.0;
    double maxArea = 100000.0;
    QString addressFilter;
    int minFloors = 0;
    int maxFloors = 100;
    
    bool isActive() const {
        return (minYear > 1500 || maxYear < QDate::currentDate().year() ||
                minApartments > 0 || maxApartments < 1000 ||
                minArea > 0.0 || maxArea < 100000.0 ||
                !addressFilter.isEmpty() ||
                minFloors > 0 || maxFloors < 100);
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddHouse();
    void onEditHouse();
    void onDeleteHouses();
    void onSearch();
    void onExport();
    void onFilter();
    void onClearFilters();
    void onRefresh();
    void onAbout();
    void onExit();
    
    void updateStatusBar();
    void onHouseSelected(int row, int column);

private:
    Ui::MainWindow* ui;
    DatabaseManager* dbManager;
    
    // Фильтры
    FilterSettings currentFilters;
    
    // Сортировка
    struct SortColumn {
        int column;
        bool ascending;
    };
    QList<SortColumn> sortColumns;
    
    void setupUI();
    void setupTable();
    void loadHouses();
    void loadHouses(const vector<House>& houses);
    void applyFilters(vector<House>& houses);
    void applySorting(vector<House>& houses);
    void showFilterDialog();
    void showAdvancedDeleteDialog();
    
    bool confirmAction(const QString& title, const QString& message);
    void showError(const QString& message);
    void showInfo(const QString& message);
    
    // Вспомогательные методы
    bool matchesFilters(const House& house) const;
    bool compareHouses(const House& a, const House& b) const;
};

#endif
