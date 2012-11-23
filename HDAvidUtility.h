#ifndef HDAVIDUTILITY_H
#define HDAVIDUTILITY_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QCloseEvent>
#include <QSettings>
#include <QUuid>
#include <QDate>
#include <QDateTime>
#include <QTextStream>

namespace Ui {
    class HDAvidUtility;
}

class HDAvidUtility : public QMainWindow
{
    Q_OBJECT

public:
    explicit HDAvidUtility(QWidget *parent = 0);
    ~HDAvidUtility();

private slots:
    bool checkTime();

    void on_startButton_clicked();

    void on_stopButton_clicked();

    void on_inputBrowse_clicked();

    void on_outputBrowse_clicked();

    void on_actionOpenMainWindow_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionQuit_triggered();

    void trayIconClicked(QSystemTrayIcon::ActivationReason);

    void renameTranscodedFiles();

private:
    Ui::HDAvidUtility *ui;

    QProcess *transcode;

    QFileInfoList fileList;
    int counter;

    QSettings *options;
    QString defaultInputFolder;
    QString defaultOutputFolder;
    QString systemID;
    QString licenseID;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    void closeEvent(QCloseEvent *);

    void createTrayIcon();
    void getFileList();
    QStringList getCmdArgs();
    void startTranscode();
    void loadSettings();
    void saveSettings();
    void selfDestroy();

    QTimer *transcodeTimer;
    QTimer *licenseTimer;
};

#endif // HDAVIDUTILITY_H
