#include <QApplication>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSoundEffect>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>
#include <QWheelEvent>
#include <QFontMetrics>
#include <algorithm>
#include <QIcon>

// ============================================================
// VERTICAL SCROLL PICKER
// ============================================================

class TimePicker : public QWidget
{
    Q_OBJECT

public:

    TimePicker(int minimum, int maximum, QWidget *parent = nullptr)
        : QWidget(parent),
          minValue(minimum),
          maxValue(maximum),
          currentValue(minimum)
    {
        setMinimumSize(100, 230);
        setMaximumHeight(250);

        setMouseTracking(true);
    }

    int value() const
    {
        return currentValue;
    }

    void setValue(int value)
    {
        value = std::clamp(value, minValue, maxValue);

        if (currentValue == value)
            return;

        currentValue = value;

        update();

        emit valueChanged(currentValue);
    }

signals:

    void valueChanged(int value);

protected:

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(
            rect(),
            QColor("#050505")
        );

        const int centerY = height() / 2;

        const int rowHeight = 46;

        // --------------------------------------------------------
        // Draw numbers
        // --------------------------------------------------------

        for (int offset = -2; offset <= 2; ++offset)
        {
            int number = currentValue + offset;

            // Wrap around
            if (number > maxValue)
                number = minValue + (number - maxValue - 1);

            if (number < minValue)
                number = maxValue - (minValue - number - 1);

            int y =
                centerY
                + offset * rowHeight;

            bool selected = (offset == 0);

            QFont font;

            if (selected)
            {
                font.setPixelSize(42);
                font.setWeight(QFont::DemiBold);

                painter.setPen(
                    QColor("#FFD400")
                );
            }
            else
            {
                font.setPixelSize(30);
                font.setWeight(QFont::Normal);

                painter.setPen(
                    QColor("#444444")
                );
            }

            painter.setFont(font);

            QString text =
                QString("%1")
                    .arg(
                        number,
                        2,
                        10,
                        QLatin1Char('0')
                    );

            QFontMetrics metrics(font);

            int textWidth =
                metrics.horizontalAdvance(text);

            int textHeight =
                metrics.height();

            painter.drawText(
                (width() - textWidth) / 2,
                y - textHeight / 2 + metrics.ascent(),
                text
            );
        }

        // --------------------------------------------------------
        // Selection lines
        // --------------------------------------------------------

        painter.setPen(
            QPen(
                QColor("#FFD400"),
                2
            )
        );

        painter.drawLine(
            8,
            centerY - 27,
            width() - 8,
            centerY - 27
        );

        painter.drawLine(
            8,
            centerY + 27,
            width() - 8,
            centerY + 27
        );
    }

    void wheelEvent(QWheelEvent *event) override
    {
        if (event->angleDelta().y() > 0)
        {
            setValueWithWrap(currentValue + 1);
        }
        else
        {
            setValueWithWrap(currentValue - 1);
        }

        event->accept();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            dragging = true;

            lastMouseY =
                event->position().y();

            event->accept();

            return;
        }

        QWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!dragging)
        {
            QWidget::mouseMoveEvent(event);

            return;
        }

        int currentY =
            static_cast<int>(
                event->position().y()
            );

        int difference =
            currentY - lastMouseY;

        if (std::abs(difference) >= 20)
        {
            if (difference < 0)
            {
                setValueWithWrap(
                    currentValue + 1
                );
            }
            else
            {
                setValueWithWrap(
                    currentValue - 1
                );
            }

            lastMouseY = currentY;
        }

        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            dragging = false;

            event->accept();

            return;
        }

        QWidget::mouseReleaseEvent(event);
    }

private:

    int minValue;
    int maxValue;
    int currentValue;

    bool dragging = false;

    int lastMouseY = 0;


    void setValueWithWrap(int value)
    {
        if (value > maxValue)
            value = minValue;

        if (value < minValue)
            value = maxValue;

        setValue(value);
    }
};


// ============================================================
// TIMER WINDOW
// ============================================================

class TimerWindow : public QWidget
{
    Q_OBJECT

public:

    TimerWindow()
    {
        setWindowTitle("Timer");

        setMinimumSize(500, 700);

        resize(540, 740);


        // ========================================================
        // MAIN LAYOUT
        // ========================================================

        auto *mainLayout =
            new QVBoxLayout(this);

        mainLayout->setContentsMargins(
            24,
            24,
            24,
            24
        );

        mainLayout->setSpacing(18);


        // ========================================================
        // CARD
        // ========================================================

        auto *card =
            new QFrame(this);

        card->setObjectName("card");


        auto *cardLayout =
            new QVBoxLayout(card);

        cardLayout->setContentsMargins(
            28,
            26,
            28,
            26
        );

        cardLayout->setSpacing(18);


        // ========================================================
        // TITLE
        // ========================================================

        auto *title =
            new QLabel("TIMER", card);

        title->setObjectName("title");

        title->setAlignment(
            Qt::AlignCenter
        );


        // ========================================================
        // DISPLAY
        // ========================================================

        display =
            new QLabel("00:00:00", card);

        display->setObjectName(
            "display"
        );

        display->setAlignment(
            Qt::AlignCenter
        );

        display->setMinimumHeight(
            100
        );


        // ========================================================
        // PICKERS
        // ========================================================

        auto *pickerLayout =
            new QHBoxLayout();

        pickerLayout->setSpacing(12);


        hoursPicker =
            new TimePicker(
                0,
                99,
                card
            );


        minutesPicker =
            new TimePicker(
                0,
                59,
                card
            );


        secondsPicker =
            new TimePicker(
                0,
                59,
                card
            );


        pickerLayout->addWidget(
            createPickerColumn(
                hoursPicker,
                "HOURS"
            )
        );


        pickerLayout->addWidget(
            createPickerColumn(
                minutesPicker,
                "MINUTES"
            )
        );


        pickerLayout->addWidget(
            createPickerColumn(
                secondsPicker,
                "SECONDS"
            )
        );


        // ========================================================
        // BUTTONS
        // ========================================================

        auto *buttonLayout =
            new QHBoxLayout();

        buttonLayout->setSpacing(12);


        playPause =
            new QPushButton(
                "▶ START",
                card
            );

        playPause->setObjectName(
            "primaryButton"
        );


        reset =
            new QPushButton(
                "RESET",
                card
            );

        reset->setObjectName(
            "resetButton"
        );


        buttonLayout->addWidget(
            playPause
        );

        buttonLayout->addWidget(
            reset
        );


        // ========================================================
        // HISTORY TITLE
        // ========================================================

        auto *historyTitle =
            new QLabel(
                "HISTORY",
                card
            );

        historyTitle->setObjectName(
            "historyTitle"
        );


        // ========================================================
        // HISTORY
        // ========================================================

        historyLayout =
            new QVBoxLayout();

        historyLayout->setSpacing(8);


        for (int i = 0; i < 3; ++i)
        {
            auto *item =
                new QLabel(
                    "--:--:--",
                    card
                );

            item->setObjectName(
                "historyItem"
            );

            item->setAlignment(
                Qt::AlignCenter
            );

            historyItems.append(item);

            historyLayout->addWidget(item);
        }


        // ========================================================
        // ADD TO CARD
        // ========================================================

        cardLayout->addWidget(title);

        cardLayout->addWidget(display);

        cardLayout->addLayout(
            pickerLayout
        );

        cardLayout->addLayout(
            buttonLayout
        );

        cardLayout->addWidget(
            historyTitle
        );

        cardLayout->addLayout(
            historyLayout
        );


        mainLayout->addWidget(card);


        // ========================================================
        // TIMER
        // ========================================================

        timer.setInterval(50);


        connect(
            &timer,
            &QTimer::timeout,
            this,
            [this]()
            {
                qint64 elapsedMs =
                    elapsed.elapsed();


                qint64 remainingMs =
                    targetMilliseconds
                    - elapsedMs;


                if (remainingMs <= 0)
                {
                    remainingMilliseconds = 0;

                    remainingSeconds = 0;

                    render();

                    finishTimer();

                    return;
                }


                remainingMilliseconds =
                    remainingMs;


                remainingSeconds =
                    static_cast<int>(
                        (remainingMs + 999) / 1000
                    );


                render();
            }
        );


        // ========================================================
        // START / PAUSE
        // ========================================================

        connect(
            playPause,
            &QPushButton::clicked,
            this,
            [this]()
            {
                if (running)
                {
                    pauseTimer();
                }
                else
                {
                    startTimer();
                }
            }
        );


        // ========================================================
        // RESET
        // ========================================================

        connect(
            reset,
            &QPushButton::clicked,
            this,
            [this]()
            {
                resetTimer();
            }
        );


        // ========================================================
        // PICKER CHANGES
        // ========================================================

        auto updateSelectedTime =
            [this]()
            {
                if (!running)
                {
                    remainingSeconds =
                        readSelectedTime();

                    remainingMilliseconds =
                        static_cast<qint64>(
                            remainingSeconds
                        ) * 1000;

                    // Keep selected time for history
                    originalDurationSeconds =
                        remainingSeconds;
                                        
                    render();
                }
            };


        connect(
            hoursPicker,
            &TimePicker::valueChanged,
            this,
            updateSelectedTime
        );


        connect(
            minutesPicker,
            &TimePicker::valueChanged,
            this,
            updateSelectedTime
        );


        connect(
            secondsPicker,
            &TimePicker::valueChanged,
            this,
            updateSelectedTime
        );


        // ========================================================
        // CUSTOM SOUND
        // ========================================================

        soundEffect =
            new QSoundEffect(this);


        QString soundPath =
            QCoreApplication::applicationDirPath()
            + "/../sounds/timer.wav";


        soundEffect->setSource(
            QUrl::fromLocalFile(
                soundPath
            )
        );


        soundEffect->setVolume(1.0);


        render();
    }


private:

    // ============================================================
    // UI
    // ============================================================

    QLabel *display = nullptr;

    QPushButton *playPause = nullptr;

    QPushButton *reset = nullptr;


    TimePicker *hoursPicker = nullptr;

    TimePicker *minutesPicker = nullptr;

    TimePicker *secondsPicker = nullptr;


    QVector<QLabel*> historyItems;

    QVBoxLayout *historyLayout = nullptr;


    // ============================================================
    // TIMER
    // ============================================================

    QTimer timer;

    QElapsedTimer elapsed;

    QSoundEffect *soundEffect = nullptr;


    bool running = false;


    qint64 targetMilliseconds = 0;

    qint64 remainingMilliseconds = 0;


    int remainingSeconds = 0;


    int originalDurationSeconds = 0;


    // ============================================================
    // PICKER COLUMN
    // ============================================================

    QWidget *createPickerColumn(
        TimePicker *picker,
        const QString &label
    )
    {
        auto *container =
            new QWidget(this);


        auto *layout =
            new QVBoxLayout(container);


        layout->setContentsMargins(
            0,
            0,
            0,
            0
        );


        layout->setSpacing(4);


        auto *labelWidget =
            new QLabel(
                label,
                container
            );


        labelWidget->setObjectName(
            "pickerLabel"
        );


        labelWidget->setAlignment(
            Qt::AlignCenter
        );


        layout->addWidget(
            picker
        );


        layout->addWidget(
            labelWidget
        );


        return container;
    }


    // ============================================================
    // READ SELECTED TIME
    // ============================================================

    int readSelectedTime() const
    {
        return
            hoursPicker->value() * 3600
            +
            minutesPicker->value() * 60
            +
            secondsPicker->value();
    }


    // ============================================================
    // FORMAT
    // ============================================================

    QString formatTime(
        int totalSeconds
    ) const
    {
        int hours =
            totalSeconds / 3600;


        int minutes =
            (totalSeconds % 3600) / 60;


        int seconds =
            totalSeconds % 60;


        return QString("%1:%2:%3")
            .arg(
                hours,
                2,
                10,
                QLatin1Char('0')
            )
            .arg(
                minutes,
                2,
                10,
                QLatin1Char('0')
            )
            .arg(
                seconds,
                2,
                10,
                QLatin1Char('0')
            );
    }


    // ============================================================
    // RENDER
    // ============================================================

    void render()
    {
        display->setText(
            formatTime(
                remainingSeconds
            )
        );
    }


    // ============================================================
    // START
    // ============================================================

    void startTimer()
    {
        // If timer is at zero, get value from picker

        if (remainingMilliseconds <= 0)
        {
            int selected =
                readSelectedTime();


            if (selected <= 0)
                return;


            remainingSeconds =
                selected;


            remainingMilliseconds =
                static_cast<qint64>(
                    selected
                ) * 1000;


            originalDurationSeconds =
                selected;
        }


        // Start / Resume

        targetMilliseconds =
            remainingMilliseconds;


        elapsed.restart();


        running = true;


        playPause->setText(
            "⏸ PAUSE"
        );


        timer.start();
    }


    // ============================================================
    // PAUSE
    // ============================================================

    void pauseTimer()
    {
        qint64 elapsedMs =
            elapsed.elapsed();


        remainingMilliseconds =
            std::max<qint64>(
                0,
                targetMilliseconds
                - elapsedMs
            );


        remainingSeconds =
            static_cast<int>(
                (remainingMilliseconds + 999)
                / 1000
            );


        timer.stop();


        running = false;


        playPause->setText(
            "▶ START"
        );


        render();
    }


    // ============================================================
    // FINISH
    // ============================================================

    void finishTimer()
    {
        timer.stop();


        running = false;


        remainingMilliseconds = 0;

        remainingSeconds = 0;


        playPause->setText(
            "▶ START"
        );


        render();


        // Add to history

        addHistory(
            originalDurationSeconds
        );


        // Play custom sound

        if (soundEffect)
        {
            soundEffect->play();

            QTimer::singleShot(5000, this, [this]()
            {
                if (soundEffect)
                {
                    soundEffect->stop();
                }
            });

        }
    }


    // ============================================================
    // RESET
    // ============================================================

    void resetTimer()
    {
        timer.stop();


        running = false;


        remainingMilliseconds = 0;

        remainingSeconds = 0;

        originalDurationSeconds = 0;


        // Reset pickers

        hoursPicker->setValue(0);

        minutesPicker->setValue(0);

        secondsPicker->setValue(0);


        playPause->setText(
            "▶ START"
        );


        render();


        // IMPORTANT:
        // History is NOT cleared.
    }


    // ============================================================
    // HISTORY
    // ============================================================

    void addHistory(
        int durationSeconds
    )
    {
        QString value =
            formatTime(
                durationSeconds
            );


        // Move old history down

        for (
            int i = historyItems.size() - 1;
            i > 0;
            --i
        )
        {
            historyItems[i]->setText(
                historyItems[i - 1]->text()
            );
        }


        // New history

        historyItems[0]->setText(
            value
        );
    }
};


// ============================================================
// MAIN
// ============================================================

int main(
    int argc,
    char *argv[]
)
{
    QApplication app(
        argc,
        argv
    );

    app.setWindowIcon(QIcon(":/icons/timer.png"));




    // ============================================================
    // BLACK + YELLOW UI
    // ============================================================

    app.setStyleSheet(R"(

        QWidget {
            background: #050505;
            color: #FFFFFF;
            font-family: "Sans";
        }


        #card {
            background: #0A0A0A;
            border: 1px solid #292929;
            border-radius: 24px;
        }


        #title {
            background: transparent;
            color: #FFD400;
            font-size: 15px;
            font-weight: 700;
            letter-spacing: 4px;
        }


        #display {
            background: transparent;
            color: #FFD400;
            font-size: 58px;
            font-weight: 700;
            letter-spacing: 2px;
        }


        #pickerLabel {
            background: transparent;
            color: #777777;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 2px;
        }


        QPushButton {
            min-height: 50px;

            padding-left: 25px;
            padding-right: 25px;

            border-radius: 14px;

            background: #111111;

            border: 1px solid #555555;

            color: #FFFFFF;

            font-size: 14px;

            font-weight: 700;
        }


        QPushButton:hover {
            background: #1B1B1B;

            border-color: #FFD400;

            color: #FFD400;
        }


        #primaryButton {
            background: #FFD400;

            border: 1px solid #FFD400;

            color: #000000;
        }


        #primaryButton:hover {
            background: #FFE45C;

            border-color: #FFE45C;

            color: #000000;
        }


        #resetButton {
            background: #0A0A0A;

            border: 1px solid #FFD400;

            color: #FFD400;
        }


        #resetButton:hover {
            background: #FFD400;

            color: #000000;
        }


        #historyTitle {
            background: transparent;

            color: #FFD400;

            font-size: 12px;

            font-weight: 700;

            letter-spacing: 3px;
        }


        #historyItem {
            min-height: 38px;

            background: #101010;

            border: 1px solid #252525;

            border-radius: 10px;

            color: #FFFFFF;

            font-size: 16px;

            font-weight: 600;
        }

    )");


    // ============================================================
    // WINDOW
    // ============================================================

    TimerWindow window;

    window.show();


    return app.exec();
}

#include "main.moc"