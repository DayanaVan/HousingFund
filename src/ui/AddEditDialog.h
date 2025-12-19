#ifndef ADDEDITDIALOG_H
#define ADDEDITDIALOG_H

#include <QDialog>
#include "../models/House.h"

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class DatabaseManager;

class AddEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddEditDialog(QWidget* parent = nullptr);
    ~AddEditDialog();
    
    House getHouse() const;
    void setHouse(const House& house);
    void setDatabaseManager(DatabaseManager* dbManager);
    void setEditingMode(bool editing, int houseId = 0);
    
private slots:
    void onSaveClicked();
    void onCancelClicked();
    void validateInput();

private:
    QLineEdit* addressEdit;
    QSpinBox* apartmentsSpin;
    QDoubleSpinBox* areaSpin;
    QSpinBox* yearSpin;
    QSpinBox* floorsSpin;
    
    House currentHouse;
    DatabaseManager* dbManager;
    int currentHouseId;
    bool isEditingMode;
    
    void setupUI();
    bool validateData();
    bool checkForDuplicates() const;
};

#endif // ADDEDITDIALOG_H
