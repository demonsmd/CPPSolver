#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "exceptions.h"
#include "ui_cppparameters.h"
#include "ui_alogorithmparametrs.h"
#include "ui_topogeneratordialog.h"
#include "CPService.h"

#include <QSettings>
#include <QProgressBar>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    static MainWindow* instance;

public:
    explicit MainWindow(QWidget *parent = 0);
    static MainWindow* getInstance(){return instance;}
    ~MainWindow();

private slots:
    void on_DirChooseButton_clicked();	///слот обработки нажатия кнопки выбора директории
    void on_InFileChooseButton_clicked();	///слот обработки нажатия кнопки выбора graphml файла
    void on_OutFileChooseButton_clicked();	///слот обработки нажатия кнопки выбора csv файла
    void on_topoGeneratorPB_clicked();	///слот обработки нажатия кнопки генерации топологий
    void on_StartButton_clicked();		///слот обработки нажатия кнопки старт
    void on_StopButton_clicked();		///слот обработки нажатия кнопки стоп
    void on_algorithmSettingsPB_clicked();
    void on_CPPSettingsPB_clicked();
    void on_AlgorithmChooseComboBox_currentIndexChanged(int index);
    void on_graphvizCB_clicked(bool checked);
    void on_chooseDotPB_clicked();
    void on_chooseImgBP_clicked();
    void LmaxSpinBox_valueChanged(int arg1);
    void ControllerPerfomanceSpinBox_valueChanged(int arg1);
    void SwitchPerfomanceSpinBox_valueChanged(int arg1);
    void SwitchAVGPerfomanceSB_valueChanged(int arg1);
    void ControllerCostSpinBox_valueChanged(int arg1);
    void ConnectionCostSpinBox_valueChanged(int arg1);
    void fixedTimeSB_valueChanged(int arg1);
    void bSpinBox_valueChanged(int arg1);
    void aSpinBox_valueChanged(int arg1);
    void ControllerPerfomanceCheckBox_clicked(bool checked);
    void LmaxCheckBox_clicked(bool checked);
    void SwitchPerfomanceCheckBox_clicked(bool checked);
    void SwitchAVGPerfomanceCB_clicked(bool checked);
    void ControllerCostCheckBox_clicked(bool checked);
    void FixedConCostRB_clicked(bool checked);
    void HopsDepConCostRB_clicked(bool checked);
    void LatDepConCostRB_clicked(bool checked);
    void LinearRB_clicked();
    void constRB_clicked();

    void topoSizeSB_valueChanged(int arg1);
    void algoTimeSB_valueChanged(int arg1);
    void fixedConNumSB_valueChanged(int arg1);
    void incrementalConNumSB_valueChanged(int arg1);
    void TopoSizeDependentConNumFromSB_valueChanged(int arg1);
    void TopoSizeDependentConNumToSB_valueChanged(int arg1);
    void fixedConSB_clicked();
    void incrementalConSB_clicked();
    void TopoSizeDependentConSB_clicked();
    void GACrossPosSB_valueChanged(int arg1);
    void GAMutPosSB_valueChanged(int arg1);
    void GAIterationsSB_valueChanged(int arg1);
    void GAPopSizeSB_valueChanged(int arg1);
    void WTotalCostSB_valueChanged(int arg1);
    void WAvgLatSB_valueChanged(int arg1);
    void WDisballanceSB_valueChanged(int arg1);
    void WWCLatencySB_valueChanged(int arg1);
    void WOverloadSB_valueChanged(int arg1);

    void  TopoGeneratorsrcDirPB_clicked();
    void  TopoGeneratordstDirPB_clicked();
    void  TopoGeneratorOkClicked();

public slots:
    void toLog(const QString& text);	///вывод сообщения в лог
    void clearLog();
    void processing(int min, int max, int val, QString message);	///отображение progress bar


private:
    Ui::MainWindow *ui;				///пользоватедьский интерфейс главного окна
    Ui::CPPParameters* CPPParametersUi;		///интерфейс параметров решения задачи
    Ui::topoGeneratorDialog* topoGeneratorDialogUi;
    Ui::AlgorithmParameters* AlgoParamUi;
    QDialog* CPPParameters;
    QDialog* topoGeneratorDialog;
    QDialog* AlgoParam;
    ControllerPlacementService* CPService;		///сервис управления решением задачи
    QSettings* MWSettings;				///пользовательские настройки
    QProgressBar* progress;
    ControllerPlacementSettings* CPSettings;

    QDialog* progressDialog;
    QVBoxLayout* vProgressLayout;
    QProgressBar* ProgressPB;
    QLabel* ProgressLabel;
    QPushButton* ProgressCancelPB;

    void loadMWSettings();		///загрузка пользовательских настроек главного окна
    void saveMWSettings();		///сохранение пользовательских настроек главного окна
    void initConnections();		///инициализация соединений с сервисом размещения контроллеров
    void statusBarInit();
    void applySettings();

    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
