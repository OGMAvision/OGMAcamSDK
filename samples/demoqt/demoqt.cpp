#include <QApplication>
#include "demoqt.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_hcam(nullptr), m_count(0)
    , m_timer(new QTimer(this))
    , m_imgWidth(0), m_imgHeight(0), m_pData(nullptr)
    , m_res(0), m_temp(OGMACAM_TEMP_DEF), m_tint(OGMACAM_TINT_DEF)
{
    setMinimumSize(1024, 768);

    QGroupBox* gbox_res = new QGroupBox("Resolution");
    m_cmb_res = new QComboBox();
    m_cmb_res->setEnabled(false);
    QVBoxLayout* vlyt_res = new QVBoxLayout();
    vlyt_res->addWidget(m_cmb_res);
    gbox_res->setLayout(vlyt_res);
    connect(m_cmb_res, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
    {
        if (m_hcam) //step 1: stop camera
            Ogmacam_Stop(m_hcam);

        m_res = index;
        m_imgWidth = m_cur.model->res[index].width;
        m_imgHeight = m_cur.model->res[index].height;

        if (m_hcam) //step 2: restart camera
        {
            Ogmacam_put_eSize(m_hcam, static_cast<unsigned>(m_res));
            startCamera();
        }
    });

    QGroupBox* gbox_exp = new QGroupBox("Exposure");
    m_cbox_auto = new QCheckBox();
    m_cbox_auto->setEnabled(false);
    QLabel* lbl_auto = new QLabel("Auto exposure");
    QHBoxLayout* hlyt_auto = new QHBoxLayout();
    hlyt_auto->addWidget(m_cbox_auto);
    hlyt_auto->addWidget(lbl_auto);
    hlyt_auto->addStretch();
    QLabel* lbl_time = new QLabel("Time(us):");
    QLabel* lbl_gain = new QLabel("Gain(%):");
    m_lbl_expoTime = new QLabel("0");
    m_lbl_expoGain = new QLabel("0");
    m_slider_expoTime = new QSlider(Qt::Horizontal);
    m_slider_expoGain = new QSlider(Qt::Horizontal);
    m_slider_expoTime->setEnabled(false);
    m_slider_expoGain->setEnabled(false);
    QVBoxLayout* vlyt_exp = new QVBoxLayout();
    vlyt_exp->addLayout(hlyt_auto);
    vlyt_exp->addLayout(makeLayout(lbl_time, m_slider_expoTime, m_lbl_expoTime, lbl_gain, m_slider_expoGain, m_lbl_expoGain));
    gbox_exp->setLayout(vlyt_exp);
    connect(m_cbox_auto, &QCheckBox::stateChanged, this, [this](bool state)
    {
        if (m_hcam)
        {
            Ogmacam_put_AutoExpoEnable(m_hcam, state ? 1 : 0);
            m_slider_expoTime->setEnabled(!state);
            m_slider_expoGain->setEnabled(!state);
        }
    });
    connect(m_slider_expoTime, &QSlider::valueChanged, this, [this](int value)
    {
        if (m_hcam)
        {
            m_lbl_expoTime->setText(QString::number(value));
            if (!m_cbox_auto->isChecked())
               Ogmacam_put_ExpoTime(m_hcam, value);
        }
    });
    connect(m_slider_expoGain, &QSlider::valueChanged, this, [this](int value)
    {
        if (m_hcam)
        {
            m_lbl_expoGain->setText(QString::number(value));
            if (!m_cbox_auto->isChecked())
                Ogmacam_put_ExpoAGain(m_hcam, value);
        }
    });

    QGroupBox* gbox_wb = new QGroupBox("White balance");
    m_btn_autoWB = new QPushButton("White balance");
    m_btn_autoWB->setEnabled(false);
    connect(m_btn_autoWB, &QPushButton::clicked, this, [this]()
    {
        Ogmacam_AwbOnce(m_hcam, nullptr, nullptr);
    });
    QLabel* lbl_temp = new QLabel("Temperature:");
    QLabel* lbl_tint = new QLabel("Tint:");
    m_lbl_temp = new QLabel(QString::number(OGMACAM_TEMP_DEF));
    m_lbl_tint = new QLabel(QString::number(OGMACAM_TINT_DEF));
    m_slider_temp = new QSlider(Qt::Horizontal);
    m_slider_tint = new QSlider(Qt::Horizontal);
    m_slider_temp->setRange(OGMACAM_TEMP_MIN, OGMACAM_TEMP_MAX);
    m_slider_temp->setValue(OGMACAM_TEMP_DEF);
    m_slider_tint->setRange(OGMACAM_TINT_MIN, OGMACAM_TINT_MAX);
    m_slider_tint->setValue(OGMACAM_TINT_DEF);
    m_slider_temp->setEnabled(false);
    m_slider_tint->setEnabled(false);
    QVBoxLayout* vlyt_wb = new QVBoxLayout();
    vlyt_wb->addLayout(makeLayout(lbl_temp, m_slider_temp, m_lbl_temp, lbl_tint, m_slider_tint, m_lbl_tint));
    vlyt_wb->addWidget(m_btn_autoWB);
    gbox_wb->setLayout(vlyt_wb);
    connect(m_slider_temp, &QSlider::valueChanged, this, [this](int value)
    {
        m_temp = value;
        if (m_hcam)
            Ogmacam_put_TempTint(m_hcam, m_temp, m_tint);
        m_lbl_temp->setText(QString::number(value));
    });
    connect(m_slider_tint, &QSlider::valueChanged, this, [this](int value)
    {
        m_tint = value;
        if (m_hcam)
            Ogmacam_put_TempTint(m_hcam, m_temp, m_tint);
        m_lbl_tint->setText(QString::number(value));
    });

    m_btn_open = new QPushButton("Open");
    connect(m_btn_open, &QPushButton::clicked, this, &MainWindow::onBtnOpen);
    m_btn_snap = new QPushButton("Snap");
    m_btn_snap->setEnabled(false);
    connect(m_btn_snap, &QPushButton::clicked, this, &MainWindow::onBtnSnap);

    QVBoxLayout* vlyt_ctrl = new QVBoxLayout();
    vlyt_ctrl->addWidget(gbox_res);
    vlyt_ctrl->addWidget(gbox_exp);
    vlyt_ctrl->addWidget(gbox_wb);
    vlyt_ctrl->addWidget(m_btn_open);
    vlyt_ctrl->addWidget(m_btn_snap);
    vlyt_ctrl->addStretch();
    QWidget* wg_ctrl = new QWidget();
    wg_ctrl->setLayout(vlyt_ctrl);

    m_lbl_frame = new QLabel();
    m_lbl_video = new QLabel();
    QVBoxLayout* vlyt_show = new QVBoxLayout();
    vlyt_show->addWidget(m_lbl_video, 1);
    vlyt_show->addWidget(m_lbl_frame);
    QWidget* wg_show = new QWidget();
    wg_show->setLayout(vlyt_show);

    QGridLayout* grid_main = new QGridLayout();
    grid_main->setColumnStretch(0, 1);
    grid_main->setColumnStretch(1, 4);
    grid_main->addWidget(wg_ctrl);
    grid_main->addWidget(wg_show);
    QWidget* w_main = new QWidget();
    w_main->setLayout(grid_main);
    setCentralWidget(w_main);

    connect(this, &MainWindow::evtCallback, this, [this](unsigned nEvent)
    {
        /* this run in the UI thread */
        if (m_hcam)
        {
            if (OGMACAM_EVENT_IMAGE == nEvent)
                handleImageEvent();
            else if (OGMACAM_EVENT_EXPOSURE == nEvent)
                handleExpoEvent();
            else if (OGMACAM_EVENT_TEMPTINT == nEvent)
                handleTempTintEvent();
            else if (OGMACAM_EVENT_STILLIMAGE == nEvent)
                handleStillImageEvent();
            else if (OGMACAM_EVENT_ERROR == nEvent)
            {
                closeCamera();
                QMessageBox::warning(this, "Warning", "Generic error.");
            }
            else if (OGMACAM_EVENT_DISCONNECTED == nEvent)
            {
                closeCamera();
                QMessageBox::warning(this, "Warning", "Camera disconnect.");
            }
        }
    });

    connect(m_timer, &QTimer::timeout, this, [this]()
    {
        unsigned nFrame = 0, nTime = 0, nTotalFrame = 0;
        if (m_hcam && SUCCEEDED(Ogmacam_get_FrameRate(m_hcam, &nFrame, &nTime, &nTotalFrame)) && (nTime > 0))
            m_lbl_frame->setText(QString::asprintf("%u, fps = %.1f", nTotalFrame, nFrame * 1000.0 / nTime));
    });
}

void MainWindow::closeCamera()
{
    if (m_hcam)
    {
        Ogmacam_Close(m_hcam);
        m_hcam = nullptr;
    }
    delete[] m_pData;
    m_pData = nullptr;

    m_btn_open->setText("Open");
    m_timer->stop();
    m_lbl_frame->clear();
    m_cbox_auto->setEnabled(false);
    m_slider_expoGain->setEnabled(false);
    m_slider_expoTime->setEnabled(false);
    m_btn_autoWB->setEnabled(false);
    m_slider_temp->setEnabled(false);
    m_slider_tint->setEnabled(false);
    m_btn_snap->setEnabled(false);
    m_cmb_res->setEnabled(false);
    m_cmb_res->clear();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    closeCamera();
}

void MainWindow::startCamera()
{
    if (m_pData)
    {
        delete[] m_pData;
        m_pData = nullptr;
    }
    m_pData = new uchar[TDIBWIDTHBYTES(m_imgWidth * 24) * m_imgHeight];
    unsigned uimax = 0, uimin = 0, uidef = 0;
    unsigned short usmax = 0, usmin = 0, usdef = 0;
    Ogmacam_get_ExpTimeRange(m_hcam, &uimin, &uimax, &uidef);
    m_slider_expoTime->setRange(uimin, uimax);
    Ogmacam_get_ExpoAGainRange(m_hcam, &usmin, &usmax, &usdef);
    m_slider_expoGain->setRange(usmin, usmax);
    if (0 == (m_cur.model->flag & OGMACAM_FLAG_MONO))
        handleTempTintEvent();
    handleExpoEvent();
    if (SUCCEEDED(Ogmacam_StartPullModeWithCallback(m_hcam, eventCallBack, this)))
    {
        m_cmb_res->setEnabled(true);
        m_cbox_auto->setEnabled(true);
        m_btn_autoWB->setEnabled(true);
        m_slider_temp->setEnabled(0 == (m_cur.model->flag & OGMACAM_FLAG_MONO));
        m_slider_tint->setEnabled(0 == (m_cur.model->flag & OGMACAM_FLAG_MONO));
        m_btn_open->setText("Close");
        m_btn_snap->setEnabled(true);

        int bAuto = 0;
        Ogmacam_get_AutoExpoEnable(m_hcam, &bAuto);
        m_cbox_auto->setChecked(1 == bAuto);
        
        m_timer->start(1000);
    }
    else
    {
        closeCamera();
        QMessageBox::warning(this, "Warning", "Failed to start camera.");
    }
}

void MainWindow::openCamera()
{
    m_hcam = Ogmacam_Open(m_cur.id);
    if (m_hcam)
    {
        Ogmacam_get_eSize(m_hcam, (unsigned*)&m_res);
        m_imgWidth = m_cur.model->res[m_res].width;
        m_imgHeight = m_cur.model->res[m_res].height;
        {
            const QSignalBlocker blocker(m_cmb_res);
            m_cmb_res->clear();
            for (unsigned i = 0; i < m_cur.model->preview; ++i)
                m_cmb_res->addItem(QString::asprintf("%u*%u", m_cur.model->res[i].width, m_cur.model->res[i].height));
            m_cmb_res->setCurrentIndex(m_res);
            m_cmb_res->setEnabled(true);
        }

        Ogmacam_put_Option(m_hcam, OGMACAM_OPTION_BYTEORDER, 0); //Qimage use RGB byte order
        Ogmacam_put_AutoExpoEnable(m_hcam, 1);
        startCamera();
    }
}

void MainWindow::onBtnOpen()
{
    if (m_hcam)
        closeCamera();
    else
    {
        OgmacamDeviceV2 arr[OGMACAM_MAX] = { 0 };
        unsigned count = Ogmacam_EnumV2(arr);
        if (0 == count)
            QMessageBox::warning(this, "Warning", "No camera found.");
        else if (1 == count)
        {
            m_cur = arr[0];
            openCamera();
        }
        else
        {
            QMenu menu;
            for (unsigned i = 0; i < count; ++i)
            {
                menu.addAction(
#if defined(_WIN32)
                            QString::fromWCharArray(arr[i].displayname)
#else
                            arr[i].displayname
#endif
                            , this, [this, i, arr](bool)
                {
                    m_cur = arr[i];
                    openCamera();
                });
            }
            menu.exec(mapToGlobal(m_btn_snap->pos()));
        }
    }
}

void MainWindow::onBtnSnap()
{
    if (m_hcam)
    {
        if (0 == m_cur.model->still)    // not support still image capture
        {
            if (m_pData)
            {
                QImage image(m_pData, m_imgWidth, m_imgHeight, QImage::Format_RGB888);
                image.save(QString::asprintf("demoqt_%u.jpg", ++m_count));
            }
        }
        else
        {
            QMenu menu;
            for (unsigned i = 0; i < m_cur.model->still; ++i)
            {
                menu.addAction(QString::asprintf("%u*%u", m_cur.model->res[i].width, m_cur.model->res[i].height), this, [this, i](bool)
                {
                    Ogmacam_Snap(m_hcam, i);
                });
            }
            menu.exec(mapToGlobal(m_btn_snap->pos()));
        }
    }
}

void MainWindow::eventCallBack(unsigned nEvent, void* pCallbackCtx)
{
    MainWindow* pThis = reinterpret_cast<MainWindow*>(pCallbackCtx);
    emit pThis->evtCallback(nEvent);
}

void MainWindow::handleImageEvent()
{
    unsigned width = 0, height = 0;
    if (SUCCEEDED(Ogmacam_PullImage(m_hcam, m_pData, 24, &width, &height)))
    {
        QImage image(m_pData, width, height, QImage::Format_RGB888);
        QImage newimage = image.scaled(m_lbl_video->width(), m_lbl_video->height(), Qt::KeepAspectRatio, Qt::FastTransformation);
        m_lbl_video->setPixmap(QPixmap::fromImage(newimage));
    }
}

void MainWindow::handleExpoEvent()
{
    unsigned time = 0;
    unsigned short gain = 0;
    Ogmacam_get_ExpoTime(m_hcam, &time);
    Ogmacam_get_ExpoAGain(m_hcam, &gain);
    {
        const QSignalBlocker blocker(m_slider_expoTime);
        m_slider_expoTime->setValue(int(time));
    }
    {
        const QSignalBlocker blocker(m_slider_expoGain);
        m_slider_expoGain->setValue(int(gain));
    }
    m_lbl_expoTime->setText(QString::number(time));
    m_lbl_expoGain->setText(QString::number(gain));
}

void MainWindow::handleTempTintEvent()
{
    int nTemp = 0, nTint = 0;
    if (SUCCEEDED(Ogmacam_get_TempTint(m_hcam, &nTemp, &nTint)))
    {
        {
            const QSignalBlocker blocker(m_slider_temp);
            m_slider_temp->setValue(nTemp);
        }
        {
            const QSignalBlocker blocker(m_slider_tint);
            m_slider_tint->setValue(nTint);
        }
        m_lbl_temp->setText(QString::number(nTemp));
        m_lbl_tint->setText(QString::number(nTint));
    }
}

void MainWindow::handleStillImageEvent()
{
    unsigned width = 0, height = 0;
    if (SUCCEEDED(Ogmacam_PullStillImage(m_hcam, nullptr, 24, &width, &height))) // peek
    {
        std::vector<uchar> vec(TDIBWIDTHBYTES(width * 24) * height);
        if (SUCCEEDED(Ogmacam_PullStillImage(m_hcam, &vec[0], 24, &width, &height)))
        {
			QImage image(&vec[0], width, height, QImage::Format_RGB888);
			image.save(QString::asprintf("demoqt_%u.jpg", ++m_count));
		}
    }
}

QVBoxLayout* MainWindow::makeLayout(QLabel* lbl_1, QSlider* sli_1, QLabel* val_1, QLabel* lbl_2, QSlider* sli_2, QLabel* val_2)
{
    QHBoxLayout* hlyt_1 = new QHBoxLayout();
    hlyt_1->addWidget(lbl_1);
    hlyt_1->addStretch();
    hlyt_1->addWidget(val_1);
    QHBoxLayout* hlyt_2 = new QHBoxLayout();
    hlyt_2->addWidget(lbl_2);
    hlyt_2->addStretch();
    hlyt_2->addWidget(val_2);
    QVBoxLayout* vlyt = new QVBoxLayout();
    vlyt->addLayout(hlyt_1);
    vlyt->addWidget(sli_1);
    vlyt->addLayout(hlyt_2);
    vlyt->addWidget(sli_2);
    return vlyt;
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
