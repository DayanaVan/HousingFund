#include "ExportDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QGridLayout>

ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent) {
    setupUI();
    setWindowTitle("Экспорт данных");
    setFixedSize(450, 400); 
}

ExportDialog::~ExportDialog() {}

void ExportDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Выбор файла
    QGroupBox* fileGroup = new QGroupBox("Файл для экспорта", this);
    QHBoxLayout* fileLayout = new QHBoxLayout(fileGroup);
    
    filePathEdit = new QLineEdit(this);
    filePathEdit->setPlaceholderText("Выберите файл для сохранения...");
    
    browseButton = new QPushButton("Обзор...", this);
    
    fileLayout->addWidget(filePathEdit);
    fileLayout->addWidget(browseButton);
    fileGroup->setLayout(fileLayout);

    QGroupBox* fieldsGroup = new QGroupBox("Выберите поля для экспорта", this);
    QGridLayout* fieldsLayout = new QGridLayout(fieldsGroup);
    
    idCheck = new QCheckBox("ID", this);
    addressCheck = new QCheckBox("Адрес", this);
    apartmentsCheck = new QCheckBox("Количество квартир", this);
    areaCheck = new QCheckBox("Общая площадь", this);
    yearCheck = new QCheckBox("Год постройки", this);
    floorsCheck = new QCheckBox("Этажность", this);
    ageCheck = new QCheckBox("Возраст дома", this);
    
    // По умолчанию выбираем все поля
    idCheck->setChecked(true);
    addressCheck->setChecked(true);
    apartmentsCheck->setChecked(true);
    areaCheck->setChecked(true);
    yearCheck->setChecked(true);
    floorsCheck->setChecked(true);
    ageCheck->setChecked(true);
    
    fieldsLayout->addWidget(idCheck, 0, 0);
    fieldsLayout->addWidget(addressCheck, 0, 1);
    fieldsLayout->addWidget(apartmentsCheck, 1, 0);
    fieldsLayout->addWidget(areaCheck, 1, 1);
    fieldsLayout->addWidget(yearCheck, 2, 0);
    fieldsLayout->addWidget(floorsCheck, 2, 1);
    fieldsLayout->addWidget(ageCheck, 3, 0);
    
    fieldsGroup->setLayout(fieldsLayout);
    
    QGroupBox* delimiterGroupBox = new QGroupBox("Разделитель для TXT файла", this);
    QHBoxLayout* delimiterLayout = new QHBoxLayout(delimiterGroupBox);
    
    delimiterGroup = new QButtonGroup(this);
    QRadioButton* tabRadio = new QRadioButton("Табуляция", this);
    QRadioButton* semicolonRadio = new QRadioButton("Точка с запятой", this);
    QRadioButton* commaRadio = new QRadioButton("Запятая", this);
    
    delimiterGroup->addButton(tabRadio, 0);
    delimiterGroup->addButton(semicolonRadio, 1);
    delimiterGroup->addButton(commaRadio, 2);
    tabRadio->setChecked(true);
    
    delimiterLayout->addWidget(tabRadio);
    delimiterLayout->addWidget(semicolonRadio);
    delimiterLayout->addWidget(commaRadio);
    delimiterGroupBox->setLayout(delimiterLayout);

    QLabel* formatInfo = new QLabel(
        "Формат экспорта: Текстовый файл (.txt)\n"
        "Все данные экспортируются в формате TXT с выбранным разделителем",
        this
    );
    formatInfo->setWordWrap(true);
    formatInfo->setStyleSheet("QLabel { color: #666666; font-size: 10px; background-color: #f0f0f0; padding: 5px; }");
    
    // Дополнительные опции
    headerCheck = new QCheckBox("Включать заголовок", this);
    headerCheck->setChecked(true);
    
    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    exportButton = new QPushButton("Экспорт в TXT", this);
    cancelButton = new QPushButton("Отмена", this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(exportButton);
    buttonLayout->addWidget(cancelButton);
  
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(fieldsGroup);
    mainLayout->addWidget(delimiterGroupBox);
    mainLayout->addWidget(formatInfo);
    mainLayout->addWidget(headerCheck);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
    
    // Подключаем сигналы
    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::onBrowseClicked);
    connect(exportButton, &QPushButton::clicked, this, &ExportDialog::onExportClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(filePathEdit, &QLineEdit::textChanged, this, &ExportDialog::updateExportButton);
    
    updateExportButton();
}

void ExportDialog::onBrowseClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Сохранить как текстовый файл (TXT)", 
        "houses_export.txt", 
        "Текстовые файлы (*.txt);;Все файлы (*)"
    );
    
    if (!fileName.isEmpty()) {
        if (!fileName.contains('.')) {
            fileName += ".txt";
        }
        filePathEdit->setText(fileName);
    }
}

void ExportDialog::onExportClicked() {
    if (getSelectedFields().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы одно поле для экспорта");
        return;
    }
    
    // Проверяем расширение файла - должно быть .txt
    QString filePath = filePathEdit->text();
    if (!filePath.endsWith(".txt", Qt::CaseInsensitive)) {
        int answer = QMessageBox::question(this, 
            "Подтверждение", 
            "Рекомендуется использовать расширение .txt для текстовых файлов.\n"
            "Хотите добавить расширение .txt к имени файла?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (answer == QMessageBox::Yes) {
            filePathEdit->setText(filePath + ".txt");
        }
    }
    
    accept();
}

void ExportDialog::updateExportButton() {
    exportButton->setEnabled(!filePathEdit->text().isEmpty());
}

QString ExportDialog::getFilePath() const {
    return filePathEdit->text();
}

QStringList ExportDialog::getSelectedFields() const {
    QStringList fields;
    
    if (idCheck->isChecked()) fields << "id";
    if (addressCheck->isChecked()) fields << "address";
    if (apartmentsCheck->isChecked()) fields << "apartments";
    if (areaCheck->isChecked()) fields << "total_area";
    if (yearCheck->isChecked()) fields << "build_year";
    if (floorsCheck->isChecked()) fields << "floors";
    if (ageCheck->isChecked()) fields << "age";
    
    return fields;
}

QString ExportDialog::getDelimiter() const {
    switch (delimiterGroup->checkedId()) {
        case 0: return "\t";           
        case 1: return ";";           
        case 2: return ",";           
        default: return "\t";        
    }
}

bool ExportDialog::includeHeader() const {
    return headerCheck->isChecked();
}
