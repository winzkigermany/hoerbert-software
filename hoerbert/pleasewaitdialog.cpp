#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

#include "pleasewaitdialog.h"

PleaseWaitDialog::PleaseWaitDialog(QDialog *parent) : QDialog(parent)
{
    setWindowTitle(tr("Please wait"));
    resize(387, 146);
    m_allowClose = false;

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(25, 20, 25, 25);

    m_label = new QLabel(this);
    m_label->setGeometry(QRect(10, 10, 367, 101));
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setText( tr("This might take several minutes.") );
    m_layout->addWidget(m_label);


    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setValue(0);
    m_progressBar->setGeometry(QRect(32, 40, 323, 32));
    m_layout->addWidget(m_progressBar);
    m_progressBar->show();

    m_closeButton = new QPushButton(this);
    m_closeButton->setGeometry(QRect(250, 100, 112, 32));
    m_closeButton->setText(tr("OK"));
    m_closeButton->setDisabled(true);
    m_layout->addWidget(m_closeButton, 0, Qt::AlignRight);
    connect( m_closeButton, &QAbstractButton::clicked, this, &QWidget::close);
    QMetaObject::connectSlotsByName(this);

    QApplication::setOverrideCursor(Qt::WaitCursor);    // hint to background action
    qApp->processEvents();
}


void PleaseWaitDialog::setResultString( QString resultString )
{
    if( resultString.isEmpty() ){
        resultString = "Finished.";
    }

    QApplication::restoreOverrideCursor();
    setCursor(Qt::ArrowCursor);
    qApp->processEvents();

    m_label->setText( resultString );
    m_closeButton->setDisabled(false);
    m_progressBar->hide();

    m_allowClose = true;
}

void PleaseWaitDialog::reject()
{
    if( m_allowClose ){
        done(0);
    }
}


