#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QFileInfo>
#include <topogenerator.h>

using namespace std;

MainWindow* MainWindow::instance=NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    CPService(new ControllerPlacementService())
{
    instance=this;
    ui->setupUi(this);
    CPPParametersUi = new Ui::CPPParameters;
    CPPParameters = new QDialog(this);
    CPPParameters->setWindowFlags(CPPParameters->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    CPPParametersUi->setupUi(CPPParameters);

    topoGeneratorDialogUi = new Ui::topoGeneratorDialog;
    topoGeneratorDialog = new QDialog(this);
    topoGeneratorDialog->setWindowFlags(topoGeneratorDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    topoGeneratorDialogUi->setupUi(topoGeneratorDialog);
    topoGeneratorDialog->setMinimumSize(1500,450);

    AlgoParamUi = new Ui::AlgorithmParameters;
    AlgoParam = new QDialog(this);
    AlgoParam->setWindowFlags(AlgoParam->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    AlgoParamUi->setupUi(AlgoParam);
    CPSettings = NULL;

    statusBarInit();
    initConnections();
    loadMWSettings();
    applySettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveMWSettings();
    QApplication::closeAllWindows();
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete CPPParametersUi;
    delete topoGeneratorDialogUi;
    delete AlgoParamUi;
    delete CPService;
    delete MWSettings;
    delete progress;
}

void MainWindow::statusBarInit()
{
    ui->statusBar->layout()->setSpacing(9);
    ui->statusBar->layout()->setMargin(2);

    progress = new QProgressBar();
    progress->setObjectName("progress");
    progress->setAlignment(Qt::AlignRight);
    progress->setFixedWidth(450);
    ui->statusBar->addPermanentWidget(progress);
}

void MainWindow::initConnections()
{
    //сигналы от MainWindow к Сервису
    connect(ui->LogTextBrowser, SIGNAL(anchorClicked(QUrl)),
        CPService, SLOT(networkNamePressed(QUrl)));

    //сигналы от сервиса к MainWindow
    connect(CPService, SIGNAL(toLog(QString)),
        this, SLOT(toLog(QString)));
    connect(CPService, SIGNAL(programFinnished()),
        ui->StopButton, SLOT(click()));
    connect(CPService, SIGNAL(processingTopo(int,int,int,QString)),
        this, SLOT(processing(int,int,int,QString)));

    //внутренние сигналы
    connect(CPPParametersUi->acept, SIGNAL(clicked(bool)),
        CPPParameters, SLOT(accept()));

    connect(CPPParametersUi->aSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(aSpinBox_valueChanged(int)));
    connect(CPPParametersUi->bSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(bSpinBox_valueChanged(int)));
    connect(CPPParametersUi->LmaxSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(LmaxSpinBox_valueChanged(int)));
    connect(CPPParametersUi->ControllerPerfomanceSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(ControllerPerfomanceSpinBox_valueChanged(int)));
    connect(CPPParametersUi->SwitchPerfomanceSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(SwitchPerfomanceSpinBox_valueChanged(int)));
    connect(CPPParametersUi->ControllerCostSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(ControllerCostSpinBox_valueChanged(int)));
    connect(CPPParametersUi->ConCostSB, SIGNAL(valueChanged(int)),
        this, SLOT(ConnectionCostSpinBox_valueChanged(int)));
    connect(CPPParametersUi->fixedTimeSB, SIGNAL(valueChanged(int)),
        this, SLOT(fixedTimeSB_valueChanged(int)));
    connect(CPPParametersUi->ControllerPerfomanceCheckBox, SIGNAL(clicked(bool)),
        this, SLOT(ControllerPerfomanceCheckBox_clicked(bool)));
    connect(CPPParametersUi->LmaxCB, SIGNAL(clicked(bool)),
        this, SLOT(LmaxCheckBox_clicked(bool)));
    connect(CPPParametersUi->SwitchPerfomanceCheckBox, SIGNAL(clicked(bool)),
        this, SLOT(SwitchPerfomanceCheckBox_clicked(bool)));
    connect(CPPParametersUi->ControllerCostCheckBox, SIGNAL(clicked(bool)),
        this, SLOT(ControllerCostCheckBox_clicked(bool)));
    connect(CPPParametersUi->FixedConCostRB, SIGNAL(clicked(bool)),
        this, SLOT(FixedConCostRB_clicked(bool)));
    connect(CPPParametersUi->HopsDepConCostRB, SIGNAL(clicked(bool)),
        this, SLOT(HopsDepConCostRB_clicked(bool)));
    connect(CPPParametersUi->LatDepConCostRB, SIGNAL(clicked(bool)),
        this, SLOT(LatDepConCostRB_clicked(bool)));
    connect(CPPParametersUi->LinearRB, SIGNAL(clicked()),
        this, SLOT(LinearRB_clicked()));
    connect(CPPParametersUi->constRB, SIGNAL(clicked()),
        this, SLOT(constRB_clicked()));

    connect(AlgoParamUi->topoSizeSB, SIGNAL(valueChanged(int)),
        this, SLOT(topoSizeSB_valueChanged(int)));
    connect(AlgoParamUi->algoTimeSB, SIGNAL(valueChanged(int)),
        this, SLOT(algoTimeSB_valueChanged(int)));
    connect(AlgoParamUi->fixedConNumSB, SIGNAL(valueChanged(int)),
        this, SLOT(fixedConNumSB_valueChanged(int)));
    connect(AlgoParamUi->incrementalConNumSB, SIGNAL(valueChanged(int)),
        this, SLOT(incrementalConNumSB_valueChanged(int)));
    connect(AlgoParamUi->TopoSizeDependentConNumFromSB, SIGNAL(valueChanged(int)),
        this, SLOT(TopoSizeDependentConNumFromSB_valueChanged(int)));
    connect(AlgoParamUi->TopoSizeDependentConNumToSB, SIGNAL(valueChanged(int)),
        this, SLOT(TopoSizeDependentConNumToSB_valueChanged(int)));
    connect(AlgoParamUi->FixedConNumRB, SIGNAL(clicked()),
        this, SLOT(fixedConSB_clicked()));
    connect(AlgoParamUi->incrementalConNumRB, SIGNAL(clicked()),
        this, SLOT(incrementalConSB_clicked()));
    connect(AlgoParamUi->TopoSizeDependentConNumRB, SIGNAL(clicked()),
        this, SLOT(TopoSizeDependentConSB_clicked()));

    connect(topoGeneratorDialogUi->okPB, SIGNAL(clicked(bool)),
        this, SLOT(TopoGeneratorOkClicked()));
    connect(topoGeneratorDialogUi->srcDirPB, SIGNAL(clicked(bool)),
        this, SLOT(TopoGeneratorsrcDirPB_clicked()));
    connect(topoGeneratorDialogUi->dstDirPB, SIGNAL(clicked(bool)),
        this, SLOT(TopoGeneratordstDirPB_clicked()));
    connect(topoGeneratorDialogUi->cancelPB, SIGNAL(clicked(bool)),
        topoGeneratorDialog, SLOT(reject()));

}

void MainWindow::applySettings()
{
    try
    {
        ensureExp(CPSettings, "не заданы настройки основного окна");
        ui->AlgorithmChooseComboBox->setCurrentIndex(CPSettings->algorithm);
        QFileInfo finfo(CPSettings->inFileName);
        ensureExp(finfo.isDir() || finfo.isFile(), "неверный входной файл");
        if (finfo.isFile())
        {
            ui->InFileLabel->setText(finfo.fileName());
            ui->DirNameLabel->setText("Каталог не выбран");
        }
        else
        {
            ui->DirNameLabel->setText(finfo.fileName());
            ui->InFileLabel->setText("Файл не выбран");
        }
        finfo.setFile(CPSettings->outFileName);
        ui->OutFileNameLabel->setText(finfo.fileName());
        ui->graphvizCB->setChecked(CPSettings->useGraphviz);
        finfo.setFile(CPSettings->pathToGraphviz);
        if (finfo.baseName()=="dot")
            ui->chooseDotLabel->setText(finfo.fileName());
        else
            ui->chooseDotLabel->setText("dot файл не задан");
        finfo.setFile(CPSettings->pathToImgFolder);
        ui->chooseImgLabel->setText(finfo.baseName());

        CPPParametersUi->LmaxSpinBox->setValue(CPSettings->Lmax);
        CPPParametersUi->ControllerPerfomanceSpinBox->setValue(CPSettings->CPerfomance);
        CPPParametersUi->SwitchPerfomanceSpinBox->setValue(CPSettings->SPerfomance);
        CPPParametersUi->ConCostSB->setValue(CPSettings->SCCost);
        CPPParametersUi->ControllerCostSpinBox->setValue(CPSettings->CCost);
        CPPParametersUi->aSpinBox->setValue(CPSettings->SCTF_a);
        CPPParametersUi->bSpinBox->setValue(CPSettings->SCTF_b);
        CPPParametersUi->fixedTimeSB->setValue(CPSettings->constST);
        CPPParametersUi->FixedConCostRB->setChecked(CPSettings->FixedSCC);
        CPPParametersUi->ControllerCostCheckBox->setChecked(CPSettings->FixedCC);
        CPPParametersUi->ControllerPerfomanceCheckBox->setChecked(CPSettings->FixedCP);
        CPPParametersUi->LmaxCB->setChecked(CPSettings->LmaxMultiplier);
        CPPParametersUi->SwitchPerfomanceCheckBox->setChecked(CPSettings->FixedSP);
        CPPParametersUi->HopsDepConCostRB->setChecked(CPSettings->HopsDepSCC);
        CPPParametersUi->LatDepConCostRB->setChecked(CPSettings->LatDepSCC);
        CPPParametersUi->constRB->setChecked(CPSettings->constST);
        CPPParametersUi->LinearRB->setChecked(!CPSettings->constST);

        AlgoParamUi->algoTimeSB->setValue(CPSettings->algoTime);
        AlgoParamUi->topoSizeSB->setValue(CPSettings->maxTopoSize);
        AlgoParamUi->fixedConNumSB->setValue(CPSettings->FixedConNum);
        AlgoParamUi->FixedConNumRB->setChecked(CPSettings->FixedCon);
        AlgoParamUi->incrementalConNumSB->setValue(CPSettings->IncrementalConNum);
        AlgoParamUi->incrementalConNumRB->setChecked(CPSettings->IncrementalCon);
        AlgoParamUi->TopoSizeDependentConNumFromSB->setValue(CPSettings->PercentageConNumFrom);
        AlgoParamUi->TopoSizeDependentConNumToSB->setValue(CPSettings->PercentageConNumTo);
        AlgoParamUi->TopoSizeDependentConNumRB->setChecked(CPSettings->PercentageCon);

        topoGeneratorDialogUi->ConstMaxSwitchLoadSB->setValue(MWSettings->value("topoGenerator/ConstMaxSwitchLoadSB","1000").toInt());
        topoGeneratorDialogUi->EMaxSwitchLoadSB->setValue(MWSettings->value("topoGenerator/EMaxSwitchLoadSB","1000").toInt());
        topoGeneratorDialogUi->DMaxSwitchLoadSB->setValue(MWSettings->value("topoGenerator/DMaxSwitchLoadSB","100").toInt());
        topoGeneratorDialogUi->RMaxSwitchLoadSB->setValue(MWSettings->value("topoGenerator/RMaxSwitchLoadSB","500").toInt());
        topoGeneratorDialogUi->ConstConLoadSB->setValue(MWSettings->value("topoGenerator/ConstConLoadSB","10000").toInt());
        topoGeneratorDialogUi->ConstAvgSwitchLoadSB->setValue(MWSettings->value("topoGenerator/ConstAvgSwitchLoadSB","500").toInt());
        topoGeneratorDialogUi->EAvgSwitchLoadSB->setValue(MWSettings->value("topoGenerator/EAvgSwitchLoadSB","500").toInt());
        topoGeneratorDialogUi->DAvgSwitchLoadSB->setValue(MWSettings->value("topoGenerator/ConstMaxSwitchLoadSB","50").toInt());
        topoGeneratorDialogUi->RAvgSwitchLoadSB->setValue(MWSettings->value("topoGenerator/RAvgSwitchLoadSB","200").toInt());
        topoGeneratorDialogUi->MaxDepAvgSwitchLoadSB->setValue(MWSettings->value("topoGenerator/MaxDepAvgSwitchLoadSB","50").toInt());
        topoGeneratorDialogUi->ConstConCostSB->setValue(MWSettings->value("topoGenerator/ConstConCostSB","10").toInt());
        topoGeneratorDialogUi->LatDepConCostSB->setValue(MWSettings->value("topoGenerator/LatDepConCostSB","10").toInt());
        topoGeneratorDialogUi->ConstContCostSB->setValue(MWSettings->value("topoGenerator/ConstContCostSB","100").toInt());
        topoGeneratorDialogUi->EContCostSB->setValue(MWSettings->value("topoGenerator/EContCostSB","100").toInt());
        topoGeneratorDialogUi->DContCostSB->setValue(MWSettings->value("topoGenerator/DContCostSB","10").toInt());
        topoGeneratorDialogUi->RContCostSB->setValue(MWSettings->value("topoGenerator/RContCostSB","50").toInt());

        finfo.setFile(MWSettings->value("topoGenerator/srcDirChoosePath","../").toString());
        topoGeneratorDialogUi->srcDirLabel->setText(finfo.fileName());
        finfo.setFile(MWSettings->value("topoGenerator/dstDirChoosePath","../").toString());
        topoGeneratorDialogUi->dstDirLabel->setText(finfo.fileName());

        topoGeneratorDialogUi->MaxSwitchLoadTabWidget->setCurrentIndex(MWSettings->value("topoGenerator/MaxSwitchLoadTab","0").toInt());
        topoGeneratorDialogUi->ConLoadTabWidget->setCurrentIndex(MWSettings->value("topoGenerator/ConLoadTab","0").toInt());
        topoGeneratorDialogUi->AvgSwitchLoadTabWidget->setCurrentIndex(MWSettings->value("topoGenerator/AvgSwitchLoadTab","0").toInt());
        topoGeneratorDialogUi->ConnectionCostTabWidget->setCurrentIndex(MWSettings->value("topoGenerator/ConnectionCostTab","0").toInt());
        topoGeneratorDialogUi->ConCostTabWidget->setCurrentIndex(MWSettings->value("topoGenerator/ConCostTab","0").toInt());
    }
    catch (Exceptions ex)
    {
        cerr<<"Произошла фатальная ошибка при загрузке настроек: "<<ex.getText().toStdString().c_str()<<endl;
        exit(-1);
    }
}

void MainWindow::processing(int min, int max, int val, QString message)
{
    ui->statusBar->showMessage(message);
    if (max==0)
    {
        progress->reset();
        return;
    }

    if (min != progress->minimum() || max != progress->maximum())
    {
        progress->setMinimum(min);
        progress->setMaximum(max);
    }
    progress->setValue(val);
}

void MainWindow::loadMWSettings()
{
    MWSettings = new QSettings("demon_smd", "CPPsolver");
    resize(MWSettings->value("windowWidth", 1000).toInt(),
           MWSettings->value("windowHeight", 600).toInt());

    if (MWSettings->value("isMaximized", false).toBool())
        showMaximized();

    MWSettings->beginGroup("CPService/");
    for (int i=0;i<CPService->ALGORITHMS.size();i++)
        ui->AlgorithmChooseComboBox->addItem(CPService->ALGORITHMS[i]);

    CPSettings = CPService->loadSettings(MWSettings);
    MWSettings->endGroup();
}

void MainWindow::saveMWSettings()
{
    if (!isMaximized())
    {
        MWSettings->setValue("windowHeight", height());
        MWSettings->setValue("windowWidth", width());
    }
    MWSettings->setValue("isMaximized", isMaximized());
    MWSettings->beginGroup("CPService/");
    CPService->saveSettings(MWSettings);
    MWSettings->endGroup();
}

void MainWindow::toLog(const QString &text)
{
    ui->LogTextBrowser->append(text);
}

void MainWindow::clearLog()
{
    ui->LogTextBrowser->clear();
}

//=====================================================================
//========================== SLOTS ====================================
//=====================================================================

void MainWindow::on_DirChooseButton_clicked()
{
    //нажали выбор входного Каталога
    QString dirChoosePath = MWSettings->value("dirChoosePath","../").toString();
    QString dirName = QFileDialog::getExistingDirectory(this, "Открыть каталог с топологиями", dirChoosePath);
    if (!dirName.isEmpty())
    {
        QFileInfo finfo(dirName);
        MWSettings->setValue("dirChoosePath", dirName);
        ui->DirNameLabel->setText(finfo.fileName());
        ui->InFileLabel->setText("Файл не выбран");
        CPSettings->inFileName=dirName;
    }
}

void MainWindow::on_InFileChooseButton_clicked()
{
    //нажали выбор входного файла
    QString dirChoosePath = MWSettings->value("dirChoosePath","../").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл с топологиями", dirChoosePath, "*.graphml");
    if (!fileName.isEmpty())
    {
        QFileInfo finfo(fileName);
        MWSettings->setValue("dirChoosePath", finfo.path());
        ui->InFileLabel->setText(finfo.fileName());
        ui->DirNameLabel->setText("Каталог не выбран");
        CPSettings->inFileName=fileName;
    }
}

void MainWindow::on_OutFileChooseButton_clicked()
{
    //нажали выбор выходного файла
    QString outChoosePath = MWSettings->value("outChoosePath","../").toString();
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить результаты в файлы", outChoosePath, "*.csv");
    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".csv"))
            fileName.append(".csv");
        QFileInfo finfo(fileName);
        MWSettings->setValue("outChoosePath", finfo.path());
        ui->OutFileNameLabel->setText(finfo.fileName());
        CPSettings->outFileName=fileName;
    }
}

void MainWindow::on_topoGeneratorPB_clicked(){
    topoGeneratorDialog->setWindowTitle("Параметры генерации топологий");
    topoGeneratorDialog->exec();
}

void MainWindow::on_StartButton_clicked()
{
    ui->AlgorithmChooseComboBox->setEnabled(false);
    ui->InFileChooseButton->setEnabled(false);
    ui->DirChooseButton->setEnabled(false);
    ui->OutFileChooseButton->setEnabled(false);
    ui->topoGeneratorPB->setEnabled(false);
    ui->StopButton->setEnabled(true);
    ui->StartButton->setEnabled(false);
    ui->CPPSettingsPB->setEnabled(false);
    ui->algorithmSettingsPB->setEnabled(false);
    ui->graphvizCB->setEnabled(false);
    ui->chooseDotPB->setEnabled(false);
    ui->chooseImgBP->setEnabled(false);
    clearLog();
    CPService->startButtonPressed();
}

void MainWindow::on_StopButton_clicked()
{
    ui->AlgorithmChooseComboBox->setEnabled(true);
    ui->InFileChooseButton->setEnabled(true);
    ui->DirChooseButton->setEnabled(true);
    ui->OutFileChooseButton->setEnabled(true);
    ui->topoGeneratorPB->setEnabled(true);
    ui->StopButton->setEnabled(false);
    ui->StartButton->setEnabled(true);
    ui->CPPSettingsPB->setEnabled(true);
    ui->algorithmSettingsPB->setEnabled(true);
    ui->graphvizCB->setEnabled(true);
    ui->chooseDotPB->setEnabled(true);
    ui->chooseImgBP->setEnabled(true);
    progress->reset();
    CPService->stopButtonPressed();
}

void MainWindow::on_algorithmSettingsPB_clicked()
{
    AlgoParam->setWindowTitle("Параметры алгоритма");
    AlgoParam->exec();
}

void MainWindow::on_CPPSettingsPB_clicked()
{
    CPPParameters->setWindowTitle("Параметры размещения");
    CPPParameters->exec();
}

void MainWindow::on_AlgorithmChooseComboBox_currentIndexChanged(int index)
{
    if (CPSettings)
        CPSettings->algorithm=index;
}

void MainWindow::on_graphvizCB_clicked(bool checked)
{
    CPSettings->useGraphviz=checked;
}

void MainWindow::on_chooseDotPB_clicked()
{
    QString dotChoosePath = MWSettings->value("dotChoosePath","../").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Выбор dot файла", dotChoosePath);
    if (!fileName.isEmpty())
    {
        QFileInfo finfo(fileName);
        if (finfo.isFile() && finfo.baseName()=="dot")
        {
            MWSettings->setValue("dotChoosePath", finfo.path());
            ui->chooseDotLabel->setText(finfo.fileName());
            CPSettings->pathToGraphviz=fileName;
        }
        else
        {
            QMessageBox mb;
            mb.setWindowTitle("Ошибка выбора визуализатора");
            mb.setText("Ошибка: выбранный файл не является dot файлом");
            mb.exec();
        }
    }
}

void MainWindow::on_chooseImgBP_clicked()
{
    QString imgChoosePath = MWSettings->value("imgChoosePath","../").toString();
    QString dirName = QFileDialog::getExistingDirectory(this, "Выбрать каталог с изабражениями", imgChoosePath);
    if (!dirName.isEmpty())
    {
        QFileInfo finfo(dirName);
        MWSettings->setValue("imgChoosePath", finfo.path());
        ui->chooseImgLabel->setText(finfo.fileName());
        CPSettings->pathToImgFolder=dirName;

    }
}

void MainWindow::LmaxSpinBox_valueChanged(int arg1)
{
    CPSettings->Lmax=arg1;
}

void MainWindow::ControllerPerfomanceSpinBox_valueChanged(int arg1)
{
    CPSettings->CPerfomance=arg1;
}

void MainWindow::SwitchPerfomanceSpinBox_valueChanged(int arg1)
{
    CPSettings->SPerfomance=arg1;
}

void MainWindow::ControllerCostSpinBox_valueChanged(int arg1)
{
    CPSettings->CCost=arg1;
}

void MainWindow::ConnectionCostSpinBox_valueChanged(int arg1)
{
    CPSettings->SCCost=arg1;
}

void MainWindow::fixedTimeSB_valueChanged(int arg1)
{
    CPSettings->constST=arg1;
}

void MainWindow::bSpinBox_valueChanged(int arg1)
{
    CPSettings->SCTF_b=arg1;
}

void MainWindow::aSpinBox_valueChanged(int arg1)
{
    CPSettings->SCTF_a=arg1;
}

void MainWindow::ControllerPerfomanceCheckBox_clicked(bool checked)
{
    CPSettings->FixedCP=checked;
}

void MainWindow::LmaxCheckBox_clicked(bool checked)
{
    CPSettings->LmaxMultiplier=checked;
}

void MainWindow::SwitchPerfomanceCheckBox_clicked(bool checked)
{
    CPSettings->FixedSP=checked;
}

void MainWindow::ControllerCostCheckBox_clicked(bool checked)
{
    CPSettings->FixedCC=checked;
}

void MainWindow::FixedConCostRB_clicked(bool checked)
{
    CPSettings->FixedSCC=checked;
    CPSettings->HopsDepSCC=!checked;
    CPSettings->LatDepSCC=!checked;
    CPPParametersUi->ConCostSB->setMinimum(0);
}

void MainWindow::HopsDepConCostRB_clicked(bool checked)
{
    CPSettings->HopsDepSCC=checked;
    CPSettings->FixedSCC=!checked;
    CPSettings->LatDepSCC=!checked;
    CPPParametersUi->ConCostSB->setMinimum(0);
}

void MainWindow::LatDepConCostRB_clicked(bool checked)
{
    CPSettings->LatDepSCC=checked;
    CPSettings->HopsDepSCC=!checked;
    CPSettings->FixedSCC=!checked;
    CPPParametersUi->ConCostSB->setMinimum(1);
    if (CPSettings->SCCost==0)
    {
        CPSettings->SCCost=1;
        CPPParametersUi->ConCostSB->setValue(1);
    }
}

void MainWindow::LinearRB_clicked()
{
    CPSettings->constST=false;
    CPPParametersUi->LinearRB->setChecked(true);
    CPPParametersUi->constRB->setChecked(false);
}

void MainWindow::constRB_clicked()
{
    CPSettings->constST=true;
    CPPParametersUi->LinearRB->setChecked(false);
    CPPParametersUi->constRB->setChecked(true);
}

void MainWindow::topoSizeSB_valueChanged(int arg1)
{
    CPSettings->maxTopoSize=arg1;
}

void MainWindow::algoTimeSB_valueChanged(int arg1)
{
    CPSettings->algoTime=arg1;
}

void MainWindow::fixedConNumSB_valueChanged(int arg1)
{
    CPSettings->FixedConNum=arg1;
}

void MainWindow::incrementalConNumSB_valueChanged(int arg1)
{
    CPSettings->IncrementalConNum=arg1;
}

void MainWindow::TopoSizeDependentConNumFromSB_valueChanged(int arg1)
{
    CPSettings->PercentageConNumFrom=arg1;
}

void MainWindow::TopoSizeDependentConNumToSB_valueChanged(int arg1)
{
    CPSettings->PercentageConNumTo=arg1;
}

void MainWindow::fixedConSB_clicked()
{
    CPSettings->FixedCon=true;
    CPSettings->IncrementalCon=false;
    CPSettings->PercentageCon=false;
    AlgoParamUi->incrementalConNumRB->setChecked(false);
    AlgoParamUi->TopoSizeDependentConNumRB->setChecked(false);
}

void MainWindow::incrementalConSB_clicked()
{
    CPSettings->FixedCon=false;
    CPSettings->IncrementalCon=true;
    CPSettings->PercentageCon=false;
    AlgoParamUi->FixedConNumRB->setChecked(false);
    AlgoParamUi->TopoSizeDependentConNumRB->setChecked(false);
}

void MainWindow::TopoSizeDependentConSB_clicked()
{
    CPSettings->FixedCon=false;
    CPSettings->IncrementalCon=false;
    CPSettings->PercentageCon=true;
    AlgoParamUi->FixedConNumRB->setChecked(false);
    AlgoParamUi->incrementalConNumRB->setChecked(false);
}

void MainWindow::TopoGeneratorOkClicked(){
    //нажали "ок" для генерации фходных топологий
    QString srcDir = MWSettings->value("topoGenerator/srcDirChoosePath","../").toString();
    QString dstDir = MWSettings->value("topoGenerator/dstDirChoosePath","../").toString();

    ControllerDeploymentCostGeneator* CDCG;
    ConnectionCostGeneator* CCG;
    MaxLoadGeneator* MLG;
    AvgLoadGeneator* ALG;
    ControllerLoadGeneator* CLG;


    //Максимальная производительность коммутаторов
    int tab = topoGeneratorDialogUi->MaxSwitchLoadTabWidget->currentIndex();
    MWSettings->setValue("topoGenerator/MaxSwitchLoadTab",tab);
    switch (tab) {
        case 0: { MLG = new ConstMaxLoadGeneator(topoGeneratorDialogUi->ConstMaxSwitchLoadSB->value());}
        case 1: { MLG = new NormalDistributedMaxLoadGeneator(topoGeneratorDialogUi->EMaxSwitchLoadSB->value(), topoGeneratorDialogUi->DMaxSwitchLoadSB->value(), topoGeneratorDialogUi->RMaxSwitchLoadSB->value());}
    }

    //Производительность контроллеров
    tab = topoGeneratorDialogUi->ConLoadTabWidget->currentIndex();
    MWSettings->setValue("topoGenerator/ConLoadTab",tab);
    switch (tab) {
        case 0: { CLG = new ConstControllerLoadGeneator(topoGeneratorDialogUi->ConstConLoadSB->value());}
    }

    //Средняя производительность коммутаторов
    tab = topoGeneratorDialogUi->AvgSwitchLoadTabWidget->currentIndex();
    MWSettings->setValue("topoGenerator/AvgSwitchLoadTab",tab);
    switch (tab) {
        case 0: { ALG = new ConstAvgLoadGeneator(topoGeneratorDialogUi->ConstAvgSwitchLoadSB->value());}
        case 1: { ALG = new NormalDistributedAvgLoadGeneator(topoGeneratorDialogUi->EAvgSwitchLoadSB->value(), topoGeneratorDialogUi->DAvgSwitchLoadSB->value(), topoGeneratorDialogUi->RAvgSwitchLoadSB->value());}
        case 2: { ALG = new MaxLoadLinearDependedAvgLoadGeneator(MLG, topoGeneratorDialogUi->MaxDepAvgSwitchLoadSB->value());}
    }

    //Стоимость управляющих соединений
    tab = topoGeneratorDialogUi->ConnectionCostTabWidget->currentIndex();
    MWSettings->setValue("topoGenerator/ConnectionCostTab",tab);
    switch (tab) {
        case 0: { CCG = new ConstConnectionCostGeneator(topoGeneratorDialogUi->ConstConCostSB->value());}
        case 1: { CCG = new LatencyDependentConnectionCostGeneator(topoGeneratorDialogUi->LatDepConCostSB->value());}
    }

    //Стоимость размещения контроллеров
    tab = topoGeneratorDialogUi->ConCostTabWidget->currentIndex();
    MWSettings->setValue("topoGenerator/ConCostTab",tab);
    switch (tab) {
        case 0: { CDCG = new ConstControllerDeploymentCostGeneator(topoGeneratorDialogUi->ConstContCostSB->value());}
        case 1: { CDCG = new NormalDistributedControllerDeploymentCostGeneator(topoGeneratorDialogUi->EContCostSB->value(), topoGeneratorDialogUi->DContCostSB->value(), topoGeneratorDialogUi->RContCostSB->value());}
    }

    MWSettings->setValue("topoGenerator/ConstMaxSwitchLoadSB",topoGeneratorDialogUi->ConstMaxSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/EMaxSwitchLoadSB",topoGeneratorDialogUi->EMaxSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/DMaxSwitchLoadSB",topoGeneratorDialogUi->DMaxSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/RMaxSwitchLoadSB",topoGeneratorDialogUi->RMaxSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/ConstConLoadSB",topoGeneratorDialogUi->ConstConLoadSB->value());
    MWSettings->setValue("topoGenerator/ConstAvgSwitchLoadSB",topoGeneratorDialogUi->ConstAvgSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/EAvgSwitchLoadSB",topoGeneratorDialogUi->EAvgSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/DAvgSwitchLoadSB",topoGeneratorDialogUi->DAvgSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/RAvgSwitchLoadSB",topoGeneratorDialogUi->RAvgSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/MaxDepAvgSwitchLoadSB",topoGeneratorDialogUi->MaxDepAvgSwitchLoadSB->value());
    MWSettings->setValue("topoGenerator/ConstConCostSB",topoGeneratorDialogUi->ConstConCostSB->value());
    MWSettings->setValue("topoGenerator/LatDepConCostSB",topoGeneratorDialogUi->LatDepConCostSB->value());
    MWSettings->setValue("topoGenerator/ConstContCostSB",topoGeneratorDialogUi->ConstContCostSB->value());
    MWSettings->setValue("topoGenerator/EContCostSB",topoGeneratorDialogUi->EContCostSB->value());
    MWSettings->setValue("topoGenerator/RContCostSB",topoGeneratorDialogUi->RContCostSB->value());
    MWSettings->setValue("topoGenerator/DContCostSB",topoGeneratorDialogUi->DContCostSB->value());

    topoGenerator* generator = new topoGenerator(srcDir, dstDir, CDCG, MLG, ALG, CCG, CLG, CPSettings);

    connect(generator, SIGNAL(maxProgressChanged(int)), topoGeneratorDialogUi->progressBar, SLOT(setMaximum(int)));
    connect(generator, SIGNAL(minProgressChanged(int)), topoGeneratorDialogUi->progressBar, SLOT(setMinimum(int)));
    connect(generator, SIGNAL(valProgressChanged(int)), topoGeneratorDialogUi->progressBar, SLOT(setValue(int)));
    connect(generator, SIGNAL(done()), topoGeneratorDialog, SLOT(accept()));
    connect(generator, SIGNAL(reject()), topoGeneratorDialog, SLOT(reject()));

    topoGeneratorDialogUi->srcDirPB->setEnabled(false);
    topoGeneratorDialogUi->dstDirPB->setEnabled(false);
    topoGeneratorDialogUi->MSLGB->setEnabled(false);
    topoGeneratorDialogUi->CLGB->setEnabled(false);
    topoGeneratorDialogUi->ASLGB->setEnabled(false);
    topoGeneratorDialogUi->CCGB->setEnabled(false);
    topoGeneratorDialogUi->ContCGB->setEnabled(false);
    topoGeneratorDialogUi->okPB->setEnabled(false);

    generator->generate();

    topoGeneratorDialogUi->srcDirPB->setEnabled(true);
    topoGeneratorDialogUi->dstDirPB->setEnabled(true);
    topoGeneratorDialogUi->MSLGB->setEnabled(true);
    topoGeneratorDialogUi->CLGB->setEnabled(true);
    topoGeneratorDialogUi->ASLGB->setEnabled(true);
    topoGeneratorDialogUi->CCGB->setEnabled(true);
    topoGeneratorDialogUi->ContCGB->setEnabled(true);
    topoGeneratorDialogUi->okPB->setEnabled(true);
    topoGeneratorDialogUi->progressBar->setValue(0);

    disconnect(generator, SIGNAL(maxProgressChanged(int)), topoGeneratorDialogUi->progressBar, SLOT(setMaximum(int)));
    disconnect(generator, SIGNAL(minProgressChanged(int)), topoGeneratorDialogUi->progressBar, SLOT(setMinimum(int)));
    disconnect(generator, SIGNAL(valProgressChanged(int)), topoGeneratorDialogUi->progressBar, SLOT(setValue(int)));
    disconnect(generator, SIGNAL(done()), topoGeneratorDialog, SLOT(accept()));
    disconnect(generator, SIGNAL(reject()), topoGeneratorDialog, SLOT(reject()));

    delete generator;
    delete CDCG;
    delete CCG;
    delete MLG;
    delete ALG;
    delete CLG;
}

void MainWindow::TopoGeneratorsrcDirPB_clicked(){
    //нажали выбор входного Каталога
    QString dirChoosePath = MWSettings->value("topoGenerator/srcDirChoosePath","../").toString();
    QString dirName = QFileDialog::getExistingDirectory(topoGeneratorDialog, "Открыть каталог с топологиями", dirChoosePath);
    if (!dirName.isEmpty())
    {
        QFileInfo finfo(dirName);
        MWSettings->setValue("topoGenerator/srcDirChoosePath", dirName);
        topoGeneratorDialogUi->srcDirLabel->setText(finfo.fileName());
    }
}

void MainWindow::TopoGeneratordstDirPB_clicked(){
    //нажали выбор входного Каталога
    QString dirChoosePath = MWSettings->value("topoGenerator/dstDirChoosePath","../").toString();
    QString dirName = QFileDialog::getExistingDirectory(topoGeneratorDialog, "Открыть каталог с топологиями", dirChoosePath);
    if (!dirName.isEmpty())
    {
        QFileInfo finfo(dirName);
        MWSettings->setValue("topoGenerator/dstDirChoosePath", dirName);
        topoGeneratorDialogUi->dstDirLabel->setText(finfo.fileName());
    }
}
