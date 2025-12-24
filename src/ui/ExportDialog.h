#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QButtonGroup>
#include <QLineEdit>      
#include <QPushButton>    
#include <QGroupBox>      
#include <QRadioButton>   
#include <QLabel>         
#include <QGridLayout>    

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);
    ~ExportDialog();
    
    QString getFilePath() const;
    QStringList getSelectedFields() const;
    QString getDelimiter() const;
    bool includeHeader() const;

private slots:
    void onBrowseClicked();
    void onExportClicked();
    void updateExportButton();

private:
    QLineEdit* filePathEdit;
    QPushButton* browseButton;
    QPushButton* exportButton;
    QPushButton* cancelButton;
    
    QCheckBox* idCheck;
    QCheckBox* addressCheck;
    QCheckBox* apartmentsCheck;
    QCheckBox* areaCheck;
    QCheckBox* yearCheck;
    QCheckBox* floorsCheck;
    QCheckBox* ageCheck;
    QCheckBox* headerCheck;
    
    QButtonGroup* delimiterGroup;
    
    void setupUI();
};

#endif 
