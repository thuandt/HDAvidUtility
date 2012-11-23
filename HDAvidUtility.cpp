#include "HDAvidUtility.h"
#include "ui_HDAvidUtility.h"

HDAvidUtility::HDAvidUtility(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HDAvidUtility)
{
    options = new QSettings("HDAvidUtility.ini", QSettings::IniFormat);
    loadSettings();
    if(checkTime())
    {
        transcode = new QProcess(this);
        transcodeTimer = new QTimer(this);
        licenseTimer = new QTimer(this);
        transcodeTimer->start(30000);
        licenseTimer->start(600000);

        ui->setupUi(this);

        ui->startButton->setEnabled(true);
        ui->stopButton->setDisabled(true);

        ui->actionStart->setEnabled(true);
        ui->actionStop->setDisabled(true);

        ui->inputLineEdit->setText(defaultInputFolder);
        ui->outputLineEdit->setText(defaultOutputFolder);

        createTrayIcon();

        connect(transcode, SIGNAL(finished(int)), this, SLOT(renameTranscodedFiles()));
        connect(transcodeTimer, SIGNAL(timeout()), this, SLOT(on_startButton_clicked()));
        connect(licenseTimer, SIGNAL(timeout()), this, SLOT(checkTime()));

        on_startButton_clicked();
    }
    else
    {
        selfDestroy();
        exit(-1);
    }
}

HDAvidUtility::~HDAvidUtility()
{
    delete ui;
}

void HDAvidUtility::createTrayIcon()
{
    trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(ui->actionOpenMainWindow);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionStart);
    trayIconMenu->addAction(ui->actionStop);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionQuit);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/icons/mxf.png"));

    trayIcon->setVisible(true);

    connect(trayIcon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));
}

void HDAvidUtility::on_startButton_clicked()
{
    if(transcode->state() == QProcess::NotRunning)
    {
        // UI Display
        ui->startButton->setDisabled(true);
        ui->stopButton->setEnabled(true);
        ui->actionStart->setDisabled(true);
        ui->actionStop->setEnabled(true);

        getFileList();

        if(!fileList.isEmpty())
        {
            transcode->blockSignals(false);
            startTranscode();
        }
    }
}

void HDAvidUtility::on_stopButton_clicked()
{
    // UI Display
    ui->startButton->setEnabled(true);
    ui->stopButton->setDisabled(true);
    ui->actionStart->setEnabled(true);
    ui->actionStop->setDisabled(true);

    // Kill transcode process
    transcode->blockSignals(true);
    transcode->kill();
}

void HDAvidUtility::closeEvent(QCloseEvent *event)
{
    if(trayIcon->isVisible())
    {
        trayIcon->showMessage(trUtf8("HD Avid Utility"),
                              trUtf8("Ch\306\260\306\241ng tr\303\254nh v\341\272\253n \304\221ang ch\341\272\241y.\n\304\220\341\273\203 tho\303\241t ch\306\260\306\241ng tr\303\254nh, chu\341\273\231t ph\341\272\243i v\303\240o bi\341\273\203u t\306\260\341\273\243ng c\341\273\247a ch\306\260\306\241ng tr\303\254nh v\303\240 ch\341\273\215n \"Tho\303\241t\"."),
                              QSystemTrayIcon::Information, 500);
        this->hide();
        event->ignore();
    }
    else
    {
        saveSettings();
        event->accept();
    }
}

void HDAvidUtility::on_actionOpenMainWindow_triggered()
{
    this->show();
}

void HDAvidUtility::on_actionStart_triggered()
{
    on_startButton_clicked();
}

void HDAvidUtility::on_actionStop_triggered()
{
    on_stopButton_clicked();
}

void HDAvidUtility::on_actionQuit_triggered()
{
    if(transcode->state() == QProcess::Running)
    {
        int answer = QMessageBox::question(this,
                                           trUtf8("C\341\272\243nh b\303\241o !"),
                                           trUtf8("Ch\306\260\306\241ng tr\303\254nh \304\221ang ho\341\272\241t \304\221\341\273\231ng.\nB\341\272\241n c\303\263 ch\341\272\257c ch\304\203n mu\341\273\221n tho\303\241t kh\303\264ng ?"),
                                           QMessageBox::Yes,
                                           QMessageBox::No);
        if(answer == QMessageBox::Yes)
        {
            on_stopButton_clicked();
            trayIcon->setVisible(false);
            this->close();
        }
    }
    else
    {
        trayIcon->setVisible(false);
        this->close();
    }
}

void HDAvidUtility::trayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        this->show();
    }
}

void HDAvidUtility::on_inputBrowse_clicked()
{
    QString inputFolderBrowse = QFileDialog::getExistingDirectory(
    this,
    trUtf8("H\303\243y ch\341\273\215n th\306\260 m\341\273\245c ngu\341\273\223n"),
    "C:\\Users\\MrTux\\Desktop\\Input",
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!inputFolderBrowse.isEmpty())
    {
        ui->inputLineEdit->setText(inputFolderBrowse);
    }
}

void HDAvidUtility::on_outputBrowse_clicked()
{
    QString outputFolderBrowse = QFileDialog::getExistingDirectory(
    this,
    trUtf8("H\303\243y ch\341\273\215n th\306\260 m\341\273\245c \304\221\303\255ch"),
    "C:\\Users\\MrTux\\Desktop\\Output",
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!outputFolderBrowse.isEmpty())
    {
        ui->outputLineEdit->setText(outputFolderBrowse);
    }
}

void HDAvidUtility::getFileList()
{
    // Get all file from input folder recursive
    QDirIterator directory_walker(ui->inputLineEdit->text(),
                                  QDir::Files | QDir::NoSymLinks,
                                  QDirIterator::Subdirectories);
    QRegExp rx("^HDVietNam_");

    fileList.clear();
    while(directory_walker.hasNext())
    {
        directory_walker.next();
        if((directory_walker.fileInfo().suffix().toLower() == "mxf"  ||
            directory_walker.fileInfo().suffix().toLower() == "avi"  ||
            directory_walker.fileInfo().suffix().toLower() == "mov"  ||
            directory_walker.fileInfo().suffix().toLower() == "mpg"  ||
            directory_walker.fileInfo().suffix().toLower() == "mpeg" ||
            directory_walker.fileInfo().suffix().toLower() == "dv"   ||
            directory_walker.fileInfo().suffix().toLower() == "mp4"  ||
            directory_walker.fileInfo().suffix().toLower() == "mkv"  ||
            directory_walker.fileInfo().suffix().toLower() == "mts"  ||
            directory_walker.fileInfo().suffix().toLower() == "m2ts" ||
            directory_walker.fileInfo().suffix().toLower() == "aaf"  ||
            directory_walker.fileInfo().suffix().toLower() == "mpeg4"||
            directory_walker.fileInfo().suffix().toLower() == "wmv") &&
           !directory_walker.fileInfo().fileName().contains(rx))
        {
            fileList.append(directory_walker.fileInfo());
        }
    }

    if(!fileList.isEmpty())
    {
        counter = 0;
    }
}

QStringList HDAvidUtility::getCmdArgs()
{
    QStringList args;
    QString bitrate;
    QString rc_init;
    QString qmax;

    if(ui->outputFormatComboBox->currentText() == "IMX30")
    {
        bitrate = "30000k";
        rc_init = "1200000";
        qmax = "8";
    }
    else if(ui->outputFormatComboBox->currentText() == "IMX40")
    {
        bitrate = "40000k";
        rc_init = "1600000";
        qmax = "3";
    }
    else if(ui->outputFormatComboBox->currentText() == "IMX50")
    {
        bitrate = "50000k";
        rc_init = "2000000";
        qmax = "3";
    }

    args<< "-i" << fileList.at(counter).absoluteFilePath()
        << "-vf" << "[in]scale=702:576,pad=720:608:9:32:black[out]"
        << "-r" << "25"
        << "-top" << "1"
        << "-aspect" << "4:3"
        << "-pix_fmt" << "yuv422p"
        << "-vcodec" << "mpeg2video"
        << "-minrate" << bitrate
        << "-maxrate" << bitrate
        << "-b" << bitrate
        << "-bufsize" << rc_init
        << "-rc_init_occupancy" << rc_init
        << "-flags" << "+ildct+ilme+low_delay"
        << "-flags2" << "+ivlc+non_linear_q"
        << "-qscale" << "1"
        << "-ps" << "1"
        << "-qmin" << "1"
        << "-qmax" << qmax
        << "-rc_max_vbv_use" << "1"
        << "-rc_min_vbv_use" << "1"
        << "-dc" << "10"
        << "-intra"
        << "-acodec" << "pcm_s16le"
        << "-ar" << "48000"
        << "-ac" << "2"
        << "-f" << "mxf_d10"
        << "-y" << fileList.at(counter).baseName().append(".mxf");

    return args;
}

void HDAvidUtility::startTranscode()
{
    if(counter >=0 && counter < fileList.size())
    {
        QStringList cmdArgs = getCmdArgs();

        transcode->setWorkingDirectory(ui->outputLineEdit->text());
        transcode->start("hdffmbc", cmdArgs);
    }
    else
    {
        trayIcon->showMessage(trUtf8("HD Avid Utility"),
                              trUtf8("T\341\272\245t c\341\272\243 c\303\241c t\341\273\207p tin \304\221\303\243 \304\221\306\260\341\273\243c chuy\341\273\203n \304\221\341\273\225i."),
                              QSystemTrayIcon::Information,
                              500);
    }
}

void HDAvidUtility::renameTranscodedFiles()
{
    if(counter >=0 && counter < fileList.size() && transcode->exitCode() == 0)
    {
        QFileInfo f = fileList.at(counter);
        QFile file(f.absoluteFilePath());
        file.rename(f.absolutePath() + QString("/HDVietNam_") + f.fileName());

        trayIcon->showMessage(tr("HD Avid Utility"),
                              trUtf8("\304\220\303\243 chuy\341\273\203n \304\221\341\273\225i xong t\341\273\207p tin ") + QString(fileList.at(counter).fileName()),
                              QSystemTrayIcon::Information,
                              10);
        counter++;
    }
    startTranscode();
}

bool HDAvidUtility::checkTime()
{
    QDate lastDate = QDate::fromString(systemID, "ddMMyyyy");
    if (lastDate.isValid())
    {
        QDate today = QDate::currentDate();
        int timeAmount = licenseID.toInt();
        if(lastDate != today)
        {
            systemID = today.toString("ddMMyyyy");
            timeAmount--;
            if(timeAmount < 10)
            {
                licenseID = "0" + QString::number(timeAmount);
            }
            else
            {
                licenseID = QString::number(timeAmount);
            }
        }
        if(timeAmount > 0 && timeAmount <= 60)
        {
            return true;
        }
    }
    selfDestroy();
    return false;
}

void HDAvidUtility::loadSettings()
{
    options->beginGroup("DefaultFolder");
    defaultInputFolder = options->value("InputFolder").toString();
    defaultOutputFolder = options->value("OutputFolder").toString();
    options->endGroup();

    QDir dif;
    QDir dof;

    if(!dif.exists(defaultInputFolder))
    {
        dif.mkdir(defaultInputFolder);
    }

    if(!dof.exists(defaultOutputFolder))
    {
        dof.mkdir(defaultOutputFolder);
    }

    options->beginGroup("License");
    QString encryptSystemID = options->value("SystemID").toString();
    QString encryptLicenseID = options->value("LicenseID").toString();
    options->endGroup();

    systemID += encryptSystemID[3];
    systemID += encryptSystemID[4];
    systemID += encryptSystemID[16];
    systemID += encryptSystemID[17];
    systemID += encryptSystemID[28];
    systemID += encryptSystemID[29];
    systemID += encryptSystemID[34];
    systemID += encryptSystemID[35];

    licenseID += encryptLicenseID[11];
    licenseID += encryptLicenseID[30];
}

void HDAvidUtility::saveSettings()
{
    QString encryptSystemID = QUuid::createUuid();
    QString encryptLicenseID = QUuid::createUuid();

    encryptSystemID[3]  = systemID[0];
    encryptSystemID[4]  = systemID[1];
    encryptSystemID[16] = systemID[2];
    encryptSystemID[17] = systemID[3];
    encryptSystemID[28] = systemID[4];
    encryptSystemID[29] = systemID[5];
    encryptSystemID[34] = systemID[6];
    encryptSystemID[35] = systemID[7];

    encryptLicenseID[11] = licenseID[0];
    encryptLicenseID[30] = licenseID[1];

    options->beginGroup("License");
    options->setValue("SystemID", encryptSystemID);
    options->setValue("LicenseID", encryptLicenseID);
    options->endGroup();
}

void HDAvidUtility::selfDestroy()
{
    QFile file("selfDestroy.bat");
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&file);
    out << "taskkill /f /t /im HDAvidUtility.exe" << endl;
    out << "del /f /s /q \"%~dp0\"" << endl;

    file.close();

    QProcess *d;
    d->startDetached("cmd", QStringList() << "/c" << "selfDestroy.bat");
    exit(-1);
}
